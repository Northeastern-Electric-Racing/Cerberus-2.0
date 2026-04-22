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
 * @brief  Indicates if shutdown is closed or not.
 * @return `true` if shutdown is closed, or `false` if otherwise. Shutdown has to be closed for us to drive.
 */
bool is_shutdown_closed(void);