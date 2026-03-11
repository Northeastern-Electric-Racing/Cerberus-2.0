#include "u_tc.h"
#include "u_dti.h"
#include "u_tx_debug.h"
#include <stdint.h>

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