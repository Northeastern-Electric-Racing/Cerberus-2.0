#pragma once

#include <stdbool.h>

/**
 * @brief  Updates the BMS shutdown state. Should only be called upon receiving the BMS shutdown state message.
 * @param new_state The new state, as reported by the incoming message from BMS.
 */
void update_bms_shutdown(bool new_state);

/**
 * @brief  Processes shutdown telemetry and lightning fault. Meant to be called by the shutdown thread.
 */
void shutdown_process(void);

/**
 * @brief  Indicates if shutdown is active or not.
 * @return The current shutdown state. `false` means that shutdown is NOT active, indicating normal operation. `true` means that shutdown IS active, which is bad.
 */
bool is_shutdown_active(void);