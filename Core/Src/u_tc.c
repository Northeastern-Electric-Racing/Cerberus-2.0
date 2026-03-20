#include "u_tc.h"
#include "u_dti.h"
#include "u_tx_debug.h"
#include "u_peripherals.h"
#include <stdint.h>

// CONSTANTS ---------------------------------------------------------

// Math
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Unit conversions
#define INCHES_TO_MILES     63360.0f
#define SECONDS_TO_HOURS    1.0f / 3600.0f

// TC
#define TC_CURVE_MAGIC      0x004E4552 // " NER" in hex
#define TC_CURVE_VERSION    1
#define TC_CURVE_POINTS_MAX 200
#define TC_CALIB_SAMPLES    50
#define TC_MIN_VX           0.5f
#define VEL_ALPHA           0.98f 
#define TC_INTEGRAL_LIMIT   1.0f

// STRUCTS -----------------------------------------------------------

#pragma pack(push, 1)

typedef struct {
  float slip_ratio;
  float fx_norm;
} tire_curve_point_t;

typedef struct {
  uint32_t magic;
  uint8_t version;
  uint16_t num_points;
  char surface_id[16];
  float peak_lambda;
  tire_curve_point_t points[TC_CURVE_POINTS_MAX];
} tire_curve_t;

#pragma pack(pop)

extern const uint8_t _tire_curve_start[];
extern const uint8_t _tire_curve_size;

static tire_curve_t _tire_curve;

typedef struct {
  float    v_x;
  float    ax_bias;
  float    alpha;
  bool     init;
} vel_estimator_t;

typedef struct {
  float kp;
  float ki;
  float integral;
} tc_pi_t;

typedef struct {
  const tire_curve_t       *tire_curve;
  vel_estimator_t          vel_estimator;
  tc_pi_t                  pi;

  float omega_fl;
  float omega_fr;
  float omega_rl;
  float omega_rr;
  float torque_scale;
  float dt;
  uint32_t last_tick;
  bool tire_curve_loaded;
} tc_state_t;

static tc_state_t _tc_state = {
  .pi           = { .kp = 1.5f, .ki = 0.2f },
  .torque_scale = 1.0f,
};


// FUNCTION PROTOTYPES ------------------------------------------------

static void _load_tire_curve(tire_curve_t *curve, const uint8_t *data,
                             uint32_t size);
static float _calc_slip(float motor_rpm, float vx_car);
static float _rpm_to_rads(int16_t rpm);
static void _update_dt(void);
static float _lookup_fx(const tire_curve_t *curve, float slip);
static void _init_vel_estimator(vel_estimator_t *est, float avg_ax_stationary);
static float _estimate_velocity(vel_estimator_t *est, float avg_front_rads, float ax, float dt);
static float _update_pi(tc_pi_t *pi, const tire_curve_t *curve, float slip, float dt);

// PRIVATE FUNCTION DEFINIIONS ----------------------------------------

/**
 * @brief Loads the tire curve from the given data, and checks that it is valid.
 * Sets the _tc_curve_loaded flag to true if successful.
 *
 * @param curve The tire curve struct to load data into
 * @param data Pointer to the raw tire curve data (e.g. from flash)
 * @param size Size of the raw tire curve data in bytes
 */
static void _load_tire_curve(tire_curve_t *curve, const uint8_t *data,
                             uint32_t size) {
  if (size < sizeof(tire_curve_t)) {
    PRINTLN_ERROR("Tire curve data is too small to be valid (Size: %d bytes).",
                  size);
    return;
  }
  *curve = *(tire_curve_t *)data;
  if (curve->magic != TC_CURVE_MAGIC) {
    PRINTLN_ERROR("Invalid tire curve data (Magic: 0x%X).", curve->magic);
    return;
  }
  if (curve->version != TC_CURVE_VERSION) {
    PRINTLN_ERROR("Unsupported tire curve version (Version: %d).",
                  curve->version);
    return;
  }
  if (curve->num_points > TC_CURVE_POINTS_MAX) {
    PRINTLN_ERROR("Too many points in tire curve data (Num Points: %d).",
                  curve->num_points);
    return;
  }

  _tc_state.tire_curve_loaded = true;
}

/**
 * @brief Calculates the slip ratio based on the wheel speeds. Uses the average of the front left and right wheel speeds as the reference speed for slip calculation, since the TC algorithm is designed to prevent slip of the driven rear wheels relative to the front wheels.
 * 
 * @param motor_rpm The RPM of the motor (from DTI)
 * @param vx_car The longitudinal velocity of the car in miles per hour (estimated from wheel speeds and IMU)
 * @return float 
 */
static float _calc_slip(float motor_rpm, float vx_car) {
  float rear_rps = motor_rpm / GEAR_RATIO / 60.0f;
  float v_rear = rear_rps * (TIRE_DIAMETER * M_PI / 2.0f) * INCHES_TO_MILES * SECONDS_TO_HOURS;
  float denom = fmaxf(fabsf(vx_car), fabsf(v_rear)); 
  // Avoid division by zero and undefined slip at very low speeds
  if (denom < TC_MIN_VX) return 0.0f;
  return (v_rear - vx_car) / denom;
}

/**
 * @brief Converts RPM to radians per second.
 * 
 * @param rpm The speed in revolutions per minute
 * @return float
 */
static float _rpm_to_rads(int16_t rpm) {
  return (rpm * 2.0f * M_PI) / 60.0f;
}

/**
 * @brief Updates the time delta (dt) in seconds since the last call to this function, and stores the current tick count. Should be called at the beginning of each TC processing cycle to ensure accurate velocity estimation and slip calculation.
 * 
 */
static void _update_dt(void) {
  uint32_t current_tick = HAL_GetTick();
  _tc_state.dt = (current_tick - _tc_state.last_tick) / 1000.0f; // Convert ms to seconds
  _tc_state.last_tick = current_tick;
}

/**
 * @brief Linear interpolation into the tire curve using binary search.
 * O(log n)
 *
 * @param curve The tire curve struct to use for lookup
 * @param slip The slip ratio
 * @return float The longitudinal force
 */
static float _lookup_fx(const tire_curve_t *curve, float slip) {
  uint16_t n = curve->num_points;

  if (slip <= curve->points[0].slip_ratio) return curve->points[0].fx_norm;
  if (slip >= curve->points[n - 1].slip_ratio) return curve->points[n - 1].fx_norm;

  uint16_t lo = 0, hi = n - 1;
  while (hi - lo > 1) {
    uint16_t mid = (lo + hi) / 2;
    if (curve->points[mid].slip_ratio <= slip) {
      lo = mid;
    } else {
      hi = mid;
    }
  }

  // Linear interpolation between the two closest points
  float t = (slip - curve->points[lo].slip_ratio) /
            (curve->points[hi].slip_ratio - curve->points[lo].slip_ratio);
  return curve->points[lo].fx_norm + t * (curve->points[hi].fx_norm - curve->points[lo].fx_norm);
}

/**
 * @brief Initializes the velocity estimator with a stationary accelerometer
 * bias. Must be called once with the average ax reading while the car is
 * stationary so that ax_bias can be subtracted during integration.
 *
 * @param est Pointer to the velocity estimator struct
 * @param avg_ax_stationary Average x-axis accelerometer reading at rest (mg)
 */
static void _init_vel_estimator(vel_estimator_t *est, float avg_ax_stationary) {
  est->ax_bias   = avg_ax_stationary;
  est->v_x       = 0.0f;
  est->init      = true;
  est->alpha     = VEL_ALPHA;
  PRINTLN_INFO("Vel estimator calibrated (ax_bias=%.3f mg).", avg_ax_stationary);
}

/**
 * @brief Estimates the longitudinal velocity using the front wheel speeds and x-axis acceleration. Uses a simple complementary filter to combine the wheel speed-based velocity estimate with the accelerometer-based estimate, which helps mitigate wheel slip errors. Should be called periodically from the TC thread after calibration is complete.
 * 
 * @param est Pointer to the velocity estimator struct
 * @param avg_front_rads Average of the front left and right wheel speeds in radians per second
 * @param ax Accelerometer x-axis reading in mg (without bias already subtracted)
 * @param dt Time in seconds since the last velocity estimate (should be the TC thread period)
 * @return float Estimated longitudinal velocity in miles per hour
 */
static float _estimate_velocity(vel_estimator_t *est, float avg_front_rads, float ax, float dt) {
  // MPH
  float vx_wheel = avg_front_rads * TIRE_DIAMETER * INCHES_TO_MILES * SECONDS_TO_HOURS;
  float ax_corrected = ax - est->ax_bias;
  float vx_imu = est->v_x + (ax_corrected * dt * SECONDS_TO_HOURS * INCHES_TO_MILES);

  // Complementary filter to combine wheel and IMU estimates
  est->v_x = (est->alpha * vx_wheel) + ((1.0f - est->alpha) * vx_imu);

  // Velocity can't be negative (for TC purposes)
  if (est->v_x < 0.0f) est->v_x = 0.0f;

  return est->v_x;
}

/**
 * @brief Updates the PI controller and returns the resulting torque scale.
 * 
 * @param pi Pointer to the tc_pi_t struct containing the PI controller state and gains
 * @param curve Pointer to the tire curve struct for feedforward lookup
 * @param slip The current slip ratio
 * @param dt Time in seconds since the last update (should be the TC thread period)
 * @return float 
 */
static float _update_pi(tc_pi_t *pi, const tire_curve_t *curve, float slip, float dt) {
  // Target slip slightly below peak for stability
  float slip_ref = curve->peak_lambda * 0.98f; 
  float error = slip_ref - slip;

  pi->integral += error * dt;
  if (pi->integral > TC_INTEGRAL_LIMIT) pi->integral = TC_INTEGRAL_LIMIT;
  else if (pi->integral < -TC_INTEGRAL_LIMIT) pi->integral = -TC_INTEGRAL_LIMIT;

  // feedforward term from tire curve, plus PI control
  float fx_ff = _lookup_fx(curve, slip_ref);

  float feedback = pi->kp * error + pi->ki * pi->integral;
  float torque_scale = fx_ff + feedback;

  if (torque_scale < 0.0f) torque_scale = 0.0f;
  if (torque_scale > 1.0f) torque_scale = 1.0f;

  return torque_scale;
}

// PUBLIC FUNCTION DEFINITIONS -----------------------------------------

/**
 * @brief Loads the tire curve from flash into the _tc_curve global variable,
 * and checks that it is valid. Also inits TC module.
 *
 * @return int Success or error code (U_SUCCESS or U_ERROR)
 */
int tc_init(void) {
  int tire_model_bin_size = (int)&_tire_curve_size;
  printf("Tire model bin size: %d bytes\n", tire_model_bin_size);

  _load_tire_curve(&_tire_curve, _tire_curve_start, tire_model_bin_size);
  _tc_state.tire_curve = &_tire_curve;

  PRINTLN_INFO("Ran tc_init().");
  return U_SUCCESS;
}

/**
 * @brief Parses a front RPM CAN message and updates the stored front wheel
 * speeds. Expected format: bytes 0-1 = int16 FL RPM, bytes 2-3 = int16 FR RPM.
 *
 * @param msg The CAN message to parse
 */
void tc_record_front_rpm(can_msg_t msg) {
  int16_t fl_rpm = (int16_t)((msg.data[0] << 8) | msg.data[1]);
  int16_t fr_rpm = (int16_t)((msg.data[2] << 8) | msg.data[3]);
  _tc_state.omega_fl = _rpm_to_rads(fl_rpm);
  _tc_state.omega_fr = _rpm_to_rads(fr_rpm);
}

/**
 * @brief Parses a rear RPM CAN message and updates the stored rear wheel
 * speeds. Expected format: bytes 0-1 = int16 RL RPM, bytes 2-3 = int16 RR RPM.
 *
 * @param msg The CAN message to parse
 */
void tc_record_rear_rpm(can_msg_t msg) {
  int16_t rl_rpm = (int16_t)((msg.data[0] << 8) | msg.data[1]);
  int16_t rr_rpm = (int16_t)((msg.data[2] << 8) | msg.data[3]);
  _tc_state.omega_rl = _rpm_to_rads(rl_rpm);
  _tc_state.omega_rr = _rpm_to_rads(rr_rpm);
}

/**
 * @brief Returns the current TC torque scale factor.
 * A value of 1.0 means no reduction; 0.0 means full cutoff.
 *
 * @return float Torque scale factor in [0.0, 1.0]
 */
float tc_get_torque_scale(void) {
  return _tc_state.torque_scale;
}

/**
 * @brief Runs one iteration of the traction control algorithm.
 * Computes the current slip ratio and adjusts _torque_scale accordingly.
 * Should be called periodically from the TC thread.
 */
void tc_process(void) {
  if (!_tc_state.tire_curve_loaded) {
    _tc_state.torque_scale = 1.0f;
    return;
  }

  /* Calibration phase: accumulate TC_CALIB_SAMPLES stationary IMU readings
   * to establish the accelerometer x-axis bias before driving begins. */
  if (_tc_state.vel_estimator.init) {
    static float    _ax_sum       = 0.0f;
    static uint16_t _sample_count = 0;

    vector3_t accel;
    if (imu_getAcceleration(&accel) == U_SUCCESS) {
      _ax_sum += accel.x;
      _sample_count++;
    }

    if (_sample_count >= TC_CALIB_SAMPLES) {
      _init_vel_estimator(&_tc_state.vel_estimator, _ax_sum / _sample_count);
    }

    _tc_state.torque_scale = 1.0f;
    return;
  }

  _update_dt();

  vector3_t accel;
  if (imu_getAcceleration(&accel) != U_SUCCESS) {
    PRINTLN_ERROR("Failed to read accelerometer data during TC processing.");
    accel.x = 0.0f;
    // If we can't get IMU data, rely entirely on wheel speed for velocity estimation (disable complementary filter)
    _tc_state.vel_estimator.alpha = 0.0f; 
  }
  float f_rpms = (_tc_state.omega_fl + _tc_state.omega_fr) / 2.0f;
  float vx_car = _estimate_velocity(&_tc_state.vel_estimator, f_rpms, accel.x, _tc_state.dt);

  float slip = _calc_slip(_tc_state.omega_rl, vx_car);
  _tc_state.torque_scale = _update_pi(&_tc_state.pi, _tc_state.tire_curve, slip, _tc_state.dt);

  PRINTLN_INFO("TC: vx=%.3f scale=%.3f", vx_car, _tc_state.torque_scale);
}
