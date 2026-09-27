#ifndef __U_DRIVE_MODES_H
#define __U_DRIVE_MODES_H

typedef enum
{
    UNSELECTED,
    CRUISE_CONTROL,
    MAX_TEST_MODES
} test_mode_t;

void drive_process(float mph, float percentage_accel);

#endif /* u_drive_modes.h */