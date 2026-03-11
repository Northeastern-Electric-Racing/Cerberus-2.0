#ifndef __U_TC_H
#define __U_TC_H

/**
 * @brief Initializes the TC module. Loads the tire curve from flash and checks
 * that it is valid.
 *
 * @return int Success or error code (U_SUCCESS or U_ERROR)
 */
int tc_init(void);

#endif /* u_tc.h */