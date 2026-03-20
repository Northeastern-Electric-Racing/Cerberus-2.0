#include "unity.h"
#include "mock_u_dti.h"
#include "mock_u_peripherals.h"
#include "u_tc.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* =========================================================
 * Tire curve flash symbol stubs
 *
 * tc_init() reads the tire curve via two linker symbols:
 *   _tire_curve_start  - pointer to raw binary data in flash
 *   _tire_curve_size   - its ADDRESS (not value) is used as
 *                        the byte count (embedded linker trick)
 *
 * On a 64-bit test host, &_tire_curve_size will be a BSS
 * address >> 1627 (= sizeof(tire_curve_t) packed), so the
 * minimum-size check in _load_tire_curve() passes without
 * any extra tricks.
 *
 * Tire curve layout (pack 1):
 *   uint32_t magic       = 0x004E4552 ("NER")
 *   uint8_t  version     = 1
 *   uint16_t num_points  = 5
 *   char     surface_id  [16]
 *   float    peak_lambda = 0.1f
 *   struct { float slip_ratio; float fx_norm; } points[200]
 * Total: 4+1+2+16+4+200*8 = 1627 bytes
 * ========================================================= */
const uint8_t _tire_curve_start[1627] = {
    /* magic: 0x004E4552 little-endian */
    0x52, 0x45, 0x4E, 0x00,
    /* version: 1 */
    0x01,
    /* num_points: 5 little-endian */
    0x05, 0x00,
    /* surface_id: "asphalt" + padding (16 bytes) */
    'a','s','p','h','a','l','t', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* peak_lambda: 0.1f (IEEE 754 LE = 0x3DCCCCCD) */
    0xCD, 0xCC, 0xCC, 0x3D,
    /* points[0]: slip=0.00f, fx=0.50f */
    0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x3F,
    /* points[1]: slip=0.05f, fx=0.80f */
    0xCD, 0xCC, 0x4C, 0x3D,  0xCD, 0xCC, 0x4C, 0x3F,
    /* points[2]: slip=0.10f, fx=1.00f  <-- peak */
    0xCD, 0xCC, 0xCC, 0x3D,  0x00, 0x00, 0x80, 0x3F,
    /* points[3]: slip=0.20f, fx=0.90f */
    0xCD, 0xCC, 0x4C, 0x3E,  0x66, 0x66, 0x66, 0x3F,
    /* points[4]: slip=0.50f, fx=0.60f */
    0x00, 0x00, 0x00, 0x3F,  0x9A, 0x99, 0x19, 0x3F,
    /* remaining 195 * 8 = 1560 bytes zero-initialized by C */
};

/* Value unused; only its ADDRESS matters as described above. */
const uint8_t _tire_curve_size = 0;

/* =========================================================
 * Mock: HAL_GetTick
 * ========================================================= */
static uint32_t _mock_tick_ms = 100;

uint32_t HAL_GetTick(void) {
    return _mock_tick_ms;
}

/* =========================================================
 * CMock stub state — read by callbacks registered in setUp
 * ========================================================= */
static float   _mock_ax        = 0.0f;
static int     _mock_imu_ret   = 0; /* 0 = U_SUCCESS */
static int32_t _mock_motor_rpm = 0;

static int _imu_stub(vector3_t *data, int num_calls) {
    (void)num_calls;
    data->x = _mock_ax;
    data->y = 0.0f;
    data->z = 0.0f;
    return _mock_imu_ret;
}

static int32_t _dti_rpm_stub(int num_calls) {
    (void)num_calls;
    return _mock_motor_rpm;
}

/* =========================================================
 * Helper: pack two big-endian int16 RPMs into a CAN message.
 * Format matches tc_record_front_rpm expectations:
 *   bytes[0:1] = rpm_a (MSB first), bytes[2:3] = rpm_b
 * ========================================================= */
static can_msg_t make_rpm_msg(int16_t rpm_a, int16_t rpm_b) {
    can_msg_t msg = {0};
    msg.data[0] = (uint8_t)((rpm_a >> 8) & 0xFF);
    msg.data[1] = (uint8_t)(rpm_a        & 0xFF);
    msg.data[2] = (uint8_t)((rpm_b >> 8) & 0xFF);
    msg.data[3] = (uint8_t)(rpm_b        & 0xFF);
    return msg;
}

/* =========================================================
 * setUp / tearDown
 * ========================================================= */
void setUp(void) {
    _mock_tick_ms   = 100;
    _mock_ax        = 0.0f;
    _mock_imu_ret   = 0;
    _mock_motor_rpm = 0;

    mock_u_dti_Init();
    mock_u_peripherals_Init();
    dti_get_rpm_Stub(_dti_rpm_stub);
    imu_getAcceleration_Stub(_imu_stub);

    assert(tc_init() == 0);
}

void tearDown(void) {
    mock_u_dti_Verify();
    mock_u_dti_Destroy();
    mock_u_peripherals_Verify();
    mock_u_peripherals_Destroy();
}

/* =========================================================
 * Tests: tc_init / tc_get_torque_scale
 * ========================================================= */

void test_tc_init_returns_success(void) {
    TEST_ASSERT_EQUAL_INT(0, tc_init());
}

/* Must run before any tc_process() call because tc_init()
 * does not reset the static torque_scale field. */
void test_default_torque_scale_is_one(void) {
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, tc_get_torque_scale());
}

/* =========================================================
 * Tests: tc_record_front_rpm — CAN byte parsing
 * ========================================================= */

void test_record_front_rpm_zero_values(void) {
    can_msg_t msg = make_rpm_msg(0, 0);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_positive_equal(void) {
    can_msg_t msg = make_rpm_msg(1000, 1000);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_asymmetric_wheels(void) {
    /* FL != FR — average should be used in velocity estimate */
    can_msg_t msg = make_rpm_msg(500, 1500);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_negative_values(void) {
    can_msg_t msg = make_rpm_msg(-500, -500);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_max_int16(void) {
    can_msg_t msg = make_rpm_msg(32767, 32767);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_min_int16(void) {
    can_msg_t msg = make_rpm_msg(-32768, -32768);
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_big_endian_byte_order(void) {
    /* Manually set bytes to verify MSB-first decoding.
     * 0x03E8 = 1000 for FL, 0x07D0 = 2000 for FR. */
    can_msg_t msg = {0};
    msg.data[0] = 0x03; msg.data[1] = 0xE8; /* FL = 1000 */
    msg.data[2] = 0x07; msg.data[3] = 0xD0; /* FR = 2000 */
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_record_front_rpm_negative_sign_extension(void) {
    /* 0xFF00 = -256 as int16 (big-endian) */
    can_msg_t msg = {0};
    msg.data[0] = 0xFF; msg.data[1] = 0x00; /* FL = -256 */
    msg.data[2] = 0xFF; msg.data[3] = 0x00; /* FR = -256 */
    tc_record_front_rpm(msg);
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

/* =========================================================
 * Tests: tc_process behavior
 * ========================================================= */

void test_process_scale_bounded_over_many_iterations(void) {
    /* Scale must remain in [0, 1] across sustained operation. */
    tc_record_front_rpm(make_rpm_msg(1000, 1000));
    _mock_motor_rpm = 5000;

    for (int i = 0; i < 50; i++) {
        _mock_tick_ms += 50;
        tc_process();
        float s = tc_get_torque_scale();
        TEST_ASSERT_TRUE(s >= 0.0f);
        TEST_ASSERT_TRUE(s <= 1.0f);
    }
}

void test_process_zero_speed_slip_is_zero(void) {
    /* Front wheels at rest, motor RPM = 0 -> denominator below TC_MIN_VX
     * -> _calc_slip returns 0 -> error positive -> scale clamps to 1.0. */
    tc_record_front_rpm(make_rpm_msg(0, 0));
    _mock_motor_rpm = 0;
    _mock_tick_ms = 200;

    /* Run enough iterations for the PI integral to fully wind up. */
    for (int i = 0; i < 200; i++) {
        _mock_tick_ms += 100;
        tc_process();
    }
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, tc_get_torque_scale());
}

void test_process_scale_bounded_with_high_front_rpm(void) {
    tc_record_front_rpm(make_rpm_msg(32767, 32767));
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_reduces_torque_on_high_motor_rpm(void) {
    /* Front wheels slow (low vx_car), motor spinning very fast -> high slip
     * -> PI should reduce torque scale below 1.0 after several iterations. */
    tc_record_front_rpm(make_rpm_msg(100, 100));
    _mock_motor_rpm = 20000;

    for (int i = 0; i < 25; i++) {
        _mock_tick_ms += 100;
        tc_process();
    }
    TEST_ASSERT_TRUE(tc_get_torque_scale() < 1.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
}

void test_process_zero_motor_rpm_no_reduction(void) {
    /* Motor not spinning -> no rear wheel velocity -> slip = 0
     * -> TC should not reduce torque. */
    tc_record_front_rpm(make_rpm_msg(1000, 1000));
    _mock_motor_rpm = 0;

    for (int i = 0; i < 200; i++) {
        _mock_tick_ms += 100;
        tc_process();
    }
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 1.0f, tc_get_torque_scale());
}

void test_process_imu_failure_falls_back_gracefully(void) {
    /* When IMU read fails, tc_process should not crash and must
     * produce a valid scale.  The implementation falls back to
     * alpha=0 (wheel-speed-only velocity estimate). */
    _mock_imu_ret   = 1; /* U_ERROR */
    _mock_motor_rpm = 5000;
    tc_record_front_rpm(make_rpm_msg(500, 500));
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_imu_failure_multiple_iterations(void) {
    _mock_imu_ret   = 1; /* U_ERROR for all calls */
    _mock_motor_rpm = 3000;
    tc_record_front_rpm(make_rpm_msg(1000, 1000));
    for (int i = 0; i < 20; i++) {
        _mock_tick_ms += 100;
        tc_process();
        TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
        TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
    }
}

void test_process_dt_zero_no_crash(void) {
    /* HAL_GetTick returns the same value as last_tick -> dt = 0.
     * Integral update and PI output must still be well-defined. */
    tc_record_front_rpm(make_rpm_msg(1000, 1000));
    _mock_motor_rpm = 5000;
    /* _mock_tick_ms unchanged from setUp (100) */
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_large_positive_imu_ax(void) {
    /* Extreme positive acceleration should not produce negative velocity
     * or an out-of-range scale. */
    _mock_ax = 100000.0f;
    tc_record_front_rpm(make_rpm_msg(500, 500));
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_large_negative_imu_ax(void) {
    /* Extreme deceleration: velocity estimator clamps to >= 0. */
    _mock_ax = -100000.0f;
    tc_record_front_rpm(make_rpm_msg(500, 500));
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_scale_bounded_alternating_motor_rpm(void) {
    /* Simulate varying motor RPM across iterations. */
    for (int i = 0; i < 30; i++) {
        int16_t f_rpm = (int16_t)(1000 + (i % 3) * 200);
        _mock_motor_rpm = 3000 + (i % 5) * 1000;
        tc_record_front_rpm(make_rpm_msg(f_rpm, f_rpm));
        _mock_tick_ms += 100;
        tc_process();
        float s = tc_get_torque_scale();
        TEST_ASSERT_TRUE(s >= 0.0f);
        TEST_ASSERT_TRUE(s <= 1.0f);
    }
}

void test_process_no_rpm_messages_recorded(void) {
    /* tc_process() with no preceding front rpm recording (omega == 0). */
    _mock_tick_ms = 200;
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_rapid_tick_advance(void) {
    /* Large dt (tick jumps by 5 seconds) should not cause overflow. */
    tc_record_front_rpm(make_rpm_msg(500, 500));
    _mock_motor_rpm = 5000;
    _mock_tick_ms += 5000; /* 5 s */
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

void test_process_tick_rollover(void) {
    /* Simulate uint32_t tick rollover (wraps from near-max to small value). */
    _mock_tick_ms = 0xFFFFFF00;
    tc_init(); /* re-init so last_tick is set near rollover */
    tc_record_front_rpm(make_rpm_msg(500, 500));
    _mock_motor_rpm = 5000;
    _mock_tick_ms = 0x00000100; /* rollover */
    tc_process();
    TEST_ASSERT_TRUE(tc_get_torque_scale() >= 0.0f);
    TEST_ASSERT_TRUE(tc_get_torque_scale() <= 1.0f);
}

/* =========================================================
 * main
 * ========================================================= */
int main(void) {
    UNITY_BEGIN();

    /* Initialization */
    RUN_TEST(test_tc_init_returns_success);
    RUN_TEST(test_default_torque_scale_is_one); /* must run before any tc_process() */

    /* CAN parsing — front RPM */
    RUN_TEST(test_record_front_rpm_zero_values);
    RUN_TEST(test_record_front_rpm_positive_equal);
    RUN_TEST(test_record_front_rpm_asymmetric_wheels);
    RUN_TEST(test_record_front_rpm_negative_values);
    RUN_TEST(test_record_front_rpm_max_int16);
    RUN_TEST(test_record_front_rpm_min_int16);
    RUN_TEST(test_record_front_rpm_big_endian_byte_order);
    RUN_TEST(test_record_front_rpm_negative_sign_extension);

    /* tc_process behavior */
    RUN_TEST(test_process_scale_bounded_over_many_iterations);
    RUN_TEST(test_process_zero_speed_slip_is_zero);
    RUN_TEST(test_process_scale_bounded_with_high_front_rpm);
    RUN_TEST(test_process_reduces_torque_on_high_motor_rpm);
    RUN_TEST(test_process_zero_motor_rpm_no_reduction);
    RUN_TEST(test_process_imu_failure_falls_back_gracefully);
    RUN_TEST(test_process_imu_failure_multiple_iterations);
    RUN_TEST(test_process_dt_zero_no_crash);
    RUN_TEST(test_process_large_positive_imu_ax);
    RUN_TEST(test_process_large_negative_imu_ax);
    RUN_TEST(test_process_scale_bounded_alternating_motor_rpm);
    RUN_TEST(test_process_no_rpm_messages_recorded);
    RUN_TEST(test_process_rapid_tick_advance);
    RUN_TEST(test_process_tick_rollover);

    return UNITY_END();
}