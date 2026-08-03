#include "unity.h"
#include "test_pedals.h"
#include "mock_u_dti.h"
#include "mock_u_tc.h"
#include "mock_u_statemachine.h"
#include "u_pedals.h"
#include <stdbool.h>
#include <stdlib.h>

#include "u_queues.h"   /* for queue_t and the `faults` declaration */

/* `faults` is an extern global declared in u_queues.h. Since queue_send()
 * is mocked (via u_queues.h in mocked-files), this queue is never actually
 * used — it just needs to exist to satisfy the linker. */
queue_t faults;

uint32_t HAL_GetTick(void) {
    return 0;
}

/* ============================================================
 * Helpers — mirror the real formulas in u_pedals.c so expected
 * values recompute automatically if ACCELERATION_THRESHOLD or
 * REGEN_THRESHOLD change.
 * ============================================================ */

/* Mirrors _accel_pedal_regen_torque()'s math. */
static uint16_t expected_accel_torque(float percentage_accel, float tc_scale, float torque_limit_percentage) {
    float coeff = tc_scale * (percentage_accel - ACCELERATION_THRESHOLD) / (1.0f - ACCELERATION_THRESHOLD);
    uint16_t torque = (uint16_t)(coeff * torque_limit_percentage * MAX_TORQUE);
    if (torque > MAX_TORQUE * torque_limit_percentage) {
        torque = (uint16_t)(MAX_TORQUE * torque_limit_percentage);
    }
    return torque;
}

/* Mirrors _accel_pedal_regen_braking()'s math. */
static uint16_t expected_regen_current_x10(float percentage_accel, uint16_t regen_limit) {
    float regen_current = (regen_limit / REGEN_THRESHOLD) * (REGEN_THRESHOLD - percentage_accel);
    if (regen_current > regen_limit) {
        regen_current = (float)regen_limit;
    }
    return (uint16_t)(regen_current * 10);
}

void setUp(void) {
    mock_u_dti_Init();
    mock_u_tc_Init();
    mock_u_statemachine_Init();

    pedals_setTorqueLimitPercentage(1.0f);
}

void tearDown(void) {
    mock_u_dti_Verify();
    mock_u_dti_Destroy();
    mock_u_tc_Verify();
    mock_u_tc_Destroy();
    mock_u_statemachine_Verify();
    mock_u_statemachine_Destroy();
}

/* ============================================================
 * Acceleration branch (percentage_accel >= ACCELERATION_THRESHOLD)
 * Test points expressed as offsets from ACCELERATION_THRESHOLD,
 * so they stay meaningful (and stay inside the valid 0-1 range)
 * if the threshold value changes.
 * ============================================================ */

void test_accel_quarter_into_range(void) {
    float percentage_accel = ACCELERATION_THRESHOLD + 0.25f * (1.0f - ACCELERATION_THRESHOLD);
    float tc_scale = 1.0f;

    tc_get_torque_scale_ExpectAndReturn(tc_scale);
    dti_set_torque_Expect(expected_accel_torque(percentage_accel, tc_scale, 1.0f));

    _handle_endurance(20.0f, percentage_accel);
}

void test_accel_halfway_into_range(void) {
    float percentage_accel = ACCELERATION_THRESHOLD + 0.5f * (1.0f - ACCELERATION_THRESHOLD);
    float tc_scale = 1.0f;

    tc_get_torque_scale_ExpectAndReturn(tc_scale);
    dti_set_torque_Expect(expected_accel_torque(percentage_accel, tc_scale, 1.0f));

    _handle_endurance(20.0f, percentage_accel);
}

void test_accel_full_pedal(void) {
    float percentage_accel = 1.0f;
    float tc_scale = 1.0f;

    tc_get_torque_scale_ExpectAndReturn(tc_scale);
    dti_set_torque_Expect(expected_accel_torque(percentage_accel, tc_scale, 1.0f));

    _handle_endurance(20.0f, percentage_accel);
}

void test_accel_reduced_by_tc_scale(void) {
    /* Confirms tc_get_torque_scale()'s return value actually scales torque down. */
    float percentage_accel = ACCELERATION_THRESHOLD + 0.5f * (1.0f - ACCELERATION_THRESHOLD);
    float tc_scale = 0.6f;

    tc_get_torque_scale_ExpectAndReturn(tc_scale);
    dti_set_torque_Expect(expected_accel_torque(percentage_accel, tc_scale, 1.0f));

    _handle_endurance(20.0f, percentage_accel);
}

/* ============================================================
 * Regen braking branch (mph*MPH_TO_KMH > 5 && percentage_accel <= REGEN_THRESHOLD)
 * Test points expressed as offsets below REGEN_THRESHOLD.
 * ============================================================ */

void test_regen_quarter_into_range(void) {
    /* Light regen: pedal only slightly below REGEN_THRESHOLD. */
    float percentage_accel = REGEN_THRESHOLD * 0.75f;

    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    dti_set_regen_Expect(expected_regen_current_x10(percentage_accel, regen_limits[1]));

    _handle_endurance(10.0f, percentage_accel);
}

void test_regen_halfway_into_range(void) {
    float percentage_accel = REGEN_THRESHOLD * 0.5f;

    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    dti_set_regen_Expect(expected_regen_current_x10(percentage_accel, regen_limits[1]));

    _handle_endurance(10.0f, percentage_accel);
}

void test_regen_pedal_fully_released(void) {
    /* Maximum regen: pedal all the way up (0% travel). */
    float percentage_accel = 0.0f;

    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    get_func_state_ExpectAndReturn(F_EFFICIENCY);
    dti_set_regen_Expect(expected_regen_current_x10(percentage_accel, regen_limits[1]));

    _handle_endurance(10.0f, percentage_accel);
}

void test_no_regen_braking_when_too_slow(void) {
    /* mph=3.0 -> 3*1.609=4.827 kmh, below the 5 kmh cutoff, even though
     * percentage_accel qualifies for regen. Falls through to dead zone. */
    float percentage_accel = REGEN_THRESHOLD * 0.5f;

    dti_set_torque_Expect(0);

    _handle_endurance(3.0f, percentage_accel);
}

/* ============================================================
 * Dead zone (strictly between REGEN_THRESHOLD and ACCELERATION_THRESHOLD)
 * ============================================================ */

void test_dead_zone_between_thresholds_sets_zero_torque(void) {
    float percentage_accel = (REGEN_THRESHOLD + ACCELERATION_THRESHOLD) / 2.0f;

    dti_set_torque_Expect(0);

    _handle_endurance(20.0f, percentage_accel);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_accel_quarter_into_range);
    RUN_TEST(test_accel_halfway_into_range);
    RUN_TEST(test_accel_full_pedal);
    RUN_TEST(test_accel_reduced_by_tc_scale);
    RUN_TEST(test_regen_quarter_into_range);
    RUN_TEST(test_regen_halfway_into_range);
    RUN_TEST(test_regen_pedal_fully_released);
    RUN_TEST(test_no_regen_braking_when_too_slow);
    RUN_TEST(test_dead_zone_between_thresholds_sets_zero_torque);
    return UNITY_END();
}