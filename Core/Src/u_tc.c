#include "u_tc.h"
#include "u_dti.h"
#include "u_tx_debug.h"
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_SLIP_ERROR .06f
#define INCHES_TO_MILES 63360.0f
#define SECONDS_TO_HOURS 1.0f / 3600.0f
#define TC_CURVE_POINTS_MAX 200

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
static bool _tire_curve_loaded = false;

/* Wheel speeds in rad/s, updated from CAN messages. */
static float _omega_fl = 0.0f;
static float _omega_fr = 0.0f;
static float _omega_rl = 0.0f;
static float _omega_rr = 0.0f;

/* Torque scale factor output by TC, range [0.0, 1.0]. */
static float _torque_scale = 1.0f;

static float _rpm_to_rads(int16_t rpm);

static void _load_tire_curve(tire_curve_t *curve, const uint8_t *data,
                             uint32_t size);
static float _calc_slip(float omega_rl, float omega_rr, float omega_fl,
                        float omega_fr);
static float _calc_vehicle_speed(float omega_fl, float omega_fr);
static float _calc_rear_wheel_speed(float omega_rl, float omega_rr);
static float _rads_to_mph(float omega, float r);
static float _clamp_slip(float slip);

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
  PRINTLN_INFO("Ran tc_init().");
  return U_SUCCESS;
}

/**
 * @brief Loads the tire curve from the given data, and checks that it is valid.
 * Sets the _tc_curve_loaded flag to true if successful.
 *
 * @param curve
 * @param data
 * @param size
 */
static void _load_tire_curve(tire_curve_t *curve, const uint8_t *data,
                             uint32_t size) {
  if (size < sizeof(tire_curve_t)) {
    PRINTLN_ERROR("Tire curve data is too small to be valid (Size: %d bytes).",
                  size);
    return;
  }
  *curve = *(tire_curve_t *)data;
  if (curve->magic != 0x004E4552) { // " NER" in hex
    PRINTLN_ERROR("Invalid tire curve data (Magic: 0x%X).", curve->magic);
    return;
  }
  if (curve->version != 1) {
    PRINTLN_ERROR("Unsupported tire curve version (Version: %d).",
                  curve->version);
    return;
  }
  if (curve->num_points > TC_CURVE_POINTS_MAX) {
    PRINTLN_ERROR("Too many points in tire curve data (Num Points: %d).",
                  curve->num_points);
    return;
  }

  _tire_curve_loaded = true;
}

/**
 * @brief Calculates the slip error based on the wheel speeds and vehicle
 * speed.
 *
 * @param omega_rl Rear left wheel speed in radians per second
 * @param omega_rr Rear right wheel speed in radians per second
 * @param omega_fl Front left wheel speed in radians per second
 * @param omega_fr Front right wheel speed in radians per second
 * @return float
 */
static float _calc_slip(float omega_rl, float omega_rr, float omega_fl,
                        float omega_fr) {
  // Use min of left/right since the slower wheel will always have less slip
  float omega_rear = MIN(omega_rl, omega_rr);
  float vehicle_speed = _calc_vehicle_speed(omega_fl, omega_fr);
  float rear_wheel_ground_speed = _calc_rear_wheel_speed(omega_rl, omega_rr);
  // Vehicle speed can be 0, so add small number to denominator to prevent
  // divide by 0 (though TC should not even be active at 0 mph)
  float slip =
      (rear_wheel_ground_speed - vehicle_speed) / MAX(vehicle_speed, 1e-6f);
  return _clamp_slip(slip);
}

/**
 * @brief Clamps the slip value from [0, 1]
 *
 * @param slip The slip ratio
 * @return float The slip ratio clamped to [0, 1]
 */
static float _clamp_slip(float slip) {
  if (slip < 0) {
    return 0;
  } else if (slip > 1) {
    return 1;
  } else {
    return slip;
  }
}

/**
 * @brief Gets a better esitmate of vehicle speed by averaging the speed
 * calculated from the front wheels with the speed calculated from the rear
 * wheels.
 *
 * @param omega_fl Front left wheel speed in radians per second
 * @param omega_fr Front right wheel speed in radians per second
 * @return float Vehicle speed in miles per hour
 */
static float _calc_vehicle_speed(float omega_fl, float omega_fr) {
  // Use min of left/right since the slower wheel will always have less slip
  float omega_front = MIN(omega_fl, omega_fr);
  return (_rads_to_mph(omega_front, TIRE_DIAMETER / 2) + dti_get_mph()) / 2;
}

/**
 * @brief Calculates the rear wheel ground speed.
 *
 * @param omega_rl Rear left wheel speed in radians per second
 * @param omega_rr Rear right wheel speed in radians per second
 * @return float The rear wheel ground speed in miles per hour
 */
static float _calc_rear_wheel_speed(float omega_rl, float omega_rr) {
  // Use min of left/right since the slower wheel will always have less slip
  float omega_rear = MIN(omega_rl, omega_rr);
  return _rads_to_mph(omega_rear, TIRE_DIAMETER / 2);
}

/**
 * @brief Converts rad/s and inches to MPH ground speed.
 *
 * @param omega Angular velocity in radians per second
 * @param r Radius in inches
 * @return float The ground speed in MPH
 */
static float _rads_to_mph(float omega, float r) {
  // Convert radians per second to miles per hour
  float wheel_speed_ips = omega * r; // Inches per second
  float wheel_speed_mph =
      wheel_speed_ips * SECONDS_TO_HOURS * INCHES_TO_MILES; // Miles per hour
  return wheel_speed_mph;
}

/**
 * @brief Converts RPM to radians per second.
 *
 * @param rpm Wheel speed in RPM
 * @return float Wheel speed in rad/s
 */
static float _rpm_to_rads(int16_t rpm) {
  return (float)rpm * 2.0f * M_PI / 60.0f;
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
  _omega_fl = _rpm_to_rads(fl_rpm);
  _omega_fr = _rpm_to_rads(fr_rpm);
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
  _omega_rl = _rpm_to_rads(rl_rpm);
  _omega_rr = _rpm_to_rads(rr_rpm);
}

/**
 * @brief Returns the current TC torque scale factor.
 * A value of 1.0 means no reduction; 0.0 means full cutoff.
 *
 * @return float Torque scale factor in [0.0, 1.0]
 */
float tc_get_torque_scale(void) {
  return _torque_scale;
}

/**
 * @brief Runs one iteration of the traction control algorithm.
 * Computes the current slip ratio and adjusts _torque_scale accordingly.
 * Should be called periodically from the TC thread.
 */
void tc_process(void) {
  if (!_tire_curve_loaded) {
    _torque_scale = 1.0f;
    return;
  }

  float slip = _calc_slip(_omega_rl, _omega_rr, _omega_fl, _omega_fr);
  float peak = _tire_curve.peak_lambda;

  if (slip <= peak + MAX_SLIP_ERROR) {
    /* Slip is within acceptable bounds — no reduction needed. */
    _torque_scale = 1.0f;
  } else {
    /* Proportionally reduce torque based on how far past the slip limit we are. */
    float excess = slip - peak;
    _torque_scale = MAX(0.0f, 1.0f - (excess / MAX_SLIP_ERROR));
  }

  PRINTLN_INFO("TC: slip=%.3f peak=%.3f scale=%.3f", slip, peak, _torque_scale);
}
