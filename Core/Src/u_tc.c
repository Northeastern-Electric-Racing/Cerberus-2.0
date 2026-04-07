#include <stdint.h>
#include <math.h>
#include "u_tc.h"
#include "u_dti.h"
#include "u_tx_debug.h"
#include "u_peripherals.h"

// CONSTANTS ---------------------------------------------------------

// Math
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Unit conversions
#define INCHES_TO_MILES     63360.0f   // inches per mile
#define SECONDS_TO_HOURS    3600.0f    // seconds per hour
#define G_IN_PER_S2         386.09f    // standard gravity in in/s^2

// TC
#define TC_CURVE_MAGIC            0x004E4552 // " NER" in hex
#define TC_CURVE_VERSION          1
#define TC_CURVE_POINTS_MAX       200
#define TC_CALIB_SAMPLES          50
#define TC_MIN_VX                 0.5f
#define VEL_ALPHA                 0.98f
#define TC_INTEGRAL_LIMIT         1.0f
#define TC_SLIP_THRESHOLD_FACTOR  1.1f   // engage when slip > peak * this
#define TC_SLIP_RECOVERY_FACTOR   0.7f   // release when slip < peak * this
#define TC_FF_SLIP_FACTOR         0.8f   // FF ceiling blend starts at peak * this
#define TC_SLIP_RAW_WEIGHT        0.3f   // IIR filter: weight on raw slip signal
#define TC_INTEGRAL_DECAY_TAU     0.1f   // s, exponential decay when TC inactive
#define TC_RECOVERY_TIME_FAST     0.05f  // s, ramp rate when slip margin is large
#define TC_RECOVERY_TIME_SLOW     0.20f  // s, ramp rate when slip just cleared recovery

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

  _Atomic float torque_scale;
  _Atomic bool tc_enabled;
  float omega_fl;
  float omega_fr;
  float dt;
  uint32_t last_tick;
  bool tire_curve_loaded;

  bool  tc_active;          // hysteresis: true while TC is engaged
  float prev_slip;          // IIR filter memory
  float prev_torque_scale;  // rate limiter reference
} tc_state_t;

static tc_state_t _tc_state = {
  .pi                = { .kp = 1.5f, .ki = 0.2f },
  .torque_scale      = 1.0f,
  .prev_torque_scale = 1.0f,
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
static void _reset_tc_state(tc_state_t *s);
static float _tc_core_process(tc_state_t *s, float slip_raw, float dt);

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
 * @param motor_rpm Motor RPM from the DTI
 * @param vx_car The longitudinal velocity of the car in miles per hour (estimated from wheel speeds and IMU)
 * @return float
 */
static float _calc_slip(float motor_rpm, float vx_car) {
  float wheel_rps = motor_rpm / GEAR_RATIO / 60.0f;
  float v_rear = wheel_rps * M_PI * TIRE_DIAMETER * SECONDS_TO_HOURS / INCHES_TO_MILES;
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
  // Convert ms to seconds
  _tc_state.dt = (current_tick - _tc_state.last_tick) / 1000.0f;
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
  // MPH: v = ω * r * SECONDS_TO_HOURS / INCHES_TO_MILES
  //      = ω [rad/s] * r [in] * (3600 s/hr) / (63360 in/mi)
  float vx_wheel = avg_front_rads * (TIRE_DIAMETER / 2.0f) * SECONDS_TO_HOURS / INCHES_TO_MILES;
  float ax_corrected = ax - est->ax_bias;
  // IMU integration: a [mg] * (G_IN_PER_S2/1000) [in/s² per mg] * dt [s] → dv [in/s]
  //                  then convert dv to mph and add to current estimate
  float vx_imu = est->v_x + ax_corrected * (G_IN_PER_S2 / 1000.0f) * dt * SECONDS_TO_HOURS / INCHES_TO_MILES;

  // Complementary filter to combine wheel and IMU estimates
  est->v_x = (est->alpha * vx_wheel) + ((1.0f - est->alpha) * vx_imu);

  // Velocity can't be negative (for TC purposes)
  if (est->v_x < 0.0f) est->v_x = 0.0f;

  return est->v_x;
}

/**
 * @brief Resets TC hysteresis, integrator, and filter state. Called when TC
 * is suppressed (low speed, disabled, or no tire data).
 *
 * @param s Pointer to the TC state struct
 */
static void _reset_tc_state(tc_state_t *s) {
  s->tc_active         = false;
  s->pi.integral       = 0.0f;
  s->prev_slip         = 0.0f;
  s->prev_torque_scale = 1.0f;
}

/**
 * @brief Core TC algorithm: feedforward ceiling + PI feedback with hysteresis,
 * IIR slip filter, integrator decay, and rate-limited recovery.
 *
 * @param s        Pointer to the TC state struct (modified in place)
 * @param slip_raw Raw slip ratio from _calc_slip()
 * @param dt       Time in seconds since the last call
 * @return float   Torque scale in [0.0, 1.0]
 */
static float _tc_core_process(tc_state_t *s, float slip_raw, float dt) {
  const tire_curve_t *curve = s->tire_curve;
  float peak_lambda = curve->peak_lambda;

  // 1. IIR low-pass filter on slip
  float slip = TC_SLIP_RAW_WEIGHT * slip_raw
               + (1.0f - TC_SLIP_RAW_WEIGHT) * s->prev_slip;
  s->prev_slip = slip;

  // 2. Hysteresis thresholds
  float slip_threshold = peak_lambda * TC_SLIP_THRESHOLD_FACTOR;
  float slip_recovery  = peak_lambda * TC_SLIP_RECOVERY_FACTOR;

  // 3. Engagement / release
  if (!s->tc_active && slip > slip_threshold) s->tc_active = true;
  if ( s->tc_active && slip < slip_recovery)  s->tc_active = false;

  // 4. Integral decay when TC is inactive — prevents wind-up between events
  if (!s->tc_active) {
    s->pi.integral *= expf(-dt / TC_INTEGRAL_DECAY_TAU);
  }

  // 5. Feedforward ceiling blend
  //    Ramps continuously from open (1.0) to the tire curve peak as slip
  //    rises from ff_slip_factor*peak to slip_threshold*peak, avoiding a
  //    sudden torque step that would bypass the rate limiter.
  float ff_denom = peak_lambda * (TC_SLIP_THRESHOLD_FACTOR - TC_FF_SLIP_FACTOR);
  float ff_blend;
  if (ff_denom > 0.0f) {
    ff_blend = (slip - peak_lambda * TC_FF_SLIP_FACTOR) / ff_denom;
    if (ff_blend < 0.0f) ff_blend = 0.0f;
    if (ff_blend > 1.0f) ff_blend = 1.0f;
  } else {
    ff_blend = 0.0f;
  }
  float ff_scale_limit = _lookup_fx(curve, peak_lambda);
  float ff_scale = 1.0f + ff_blend * (ff_scale_limit - 1.0f);
  float torque_scale = (ff_scale < 1.0f) ? ff_scale : 1.0f;

  // 6. PI correction — only applied when TC is active
  if (s->tc_active) {
    float slip_error = slip - peak_lambda;   // positive = too much slip

    s->pi.integral += slip_error * dt;
    if (s->pi.integral >  TC_INTEGRAL_LIMIT) s->pi.integral =  TC_INTEGRAL_LIMIT;
    if (s->pi.integral < -TC_INTEGRAL_LIMIT) s->pi.integral = -TC_INTEGRAL_LIMIT;

    float pi_correction = s->pi.kp * slip_error + s->pi.ki * s->pi.integral;
    float trim = (pi_correction > 0.0f) ? pi_correction : 0.0f;
    torque_scale -= trim;
  }

  // 7. Rate limiter — only on recovery from a TC-reduced state.
  //    Instant snap-down is always allowed; ramp-up is capped only when
  //    prev_torque_scale < 1.0 (i.e. TC was active last cycle).
  //    Rate scales with slip margin: faster when well below recovery,
  //    slower when slip just cleared the recovery threshold.
  if (torque_scale > s->prev_torque_scale && s->prev_torque_scale < 1.0f) {
    float slip_margin = (slip_recovery - slip) / fmaxf(slip_recovery, 0.01f);
    if (slip_margin < 0.0f) slip_margin = 0.0f;
    if (slip_margin > 1.0f) slip_margin = 1.0f;

    float rate_fast  = 1.0f / TC_RECOVERY_TIME_FAST;
    float rate_slow  = 1.0f / TC_RECOVERY_TIME_SLOW;
    float rate_limit = rate_slow + slip_margin * (rate_fast - rate_slow);

    float delta = torque_scale - s->prev_torque_scale;
    float max_delta = rate_limit * dt;
    if (delta > max_delta) delta = max_delta;
    torque_scale = s->prev_torque_scale + delta;
  }

  // 8. Final clamp
  if (torque_scale < 0.0f) torque_scale = 0.0f;
  if (torque_scale > 1.0f) torque_scale = 1.0f;

  s->prev_torque_scale = torque_scale;
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
 * @brief Enables traction control.
 * 
 * @return int 
 */
int enable_tc(void) {
  _tc_state.tc_enabled = true;
  return U_SUCCESS;
}

/**
 * @brief Disables traction control.
 * 
 * @return int 
 */
int disable_tc(void) {
  _tc_state.tc_enabled = false;
  return U_SUCCESS;
}

/**
 * @brief Toggles traction control.
 * 
 * @return N/A 
 */
void toggle_tc(void) {
  _tc_state.tc_enabled = !_tc_state.tc_enabled;
}

/**
 * @brief Gets traction control enabled state.
 * 
 * @return 'true' if tc is enabled, 'false' if tc is not enabled. 
 */
bool tc_is_enabled(void) {
  return _tc_state.tc_enabled;
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
  if (!_tc_state.tc_enabled) {
    _reset_tc_state(&_tc_state);
    _tc_state.torque_scale = 1.0f;
    return;
  }

  if (!_tc_state.tire_curve_loaded) {
    _reset_tc_state(&_tc_state);
    _tc_state.torque_scale = 1.0f;
    return;
  }

  /* Calibration phase: accumulate TC_CALIB_SAMPLES stationary IMU readings
   * to establish the accelerometer x-axis bias before driving begins. */
  if (!_tc_state.vel_estimator.init) {
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

  float motor_rpm = (float)dti_get_rpm();

  // Low-speed guard: suppress TC and reset state below TC_MIN_VX
  float wheel_rps = motor_rpm / GEAR_RATIO / 60.0f;
  float v_rear = wheel_rps * M_PI * TIRE_DIAMETER * SECONDS_TO_HOURS / INCHES_TO_MILES;
  float denom = fmaxf(fabsf(vx_car), fabsf(v_rear));
  if (denom < TC_MIN_VX) {
    _reset_tc_state(&_tc_state);
    _tc_state.torque_scale = 1.0f;
    return;
  }

  float slip_raw = _calc_slip(motor_rpm, vx_car);
  _tc_state.torque_scale = _tc_core_process(&_tc_state, slip_raw, _tc_state.dt);
}
