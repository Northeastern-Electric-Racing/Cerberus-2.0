#ifndef __U_PEDALS_H
#define __U_PEDALS_H

/* Pedal sensors. This enum is ordered based on each sensor's ADC rank, which corresponds to the index of each sensor's data in the ADC buffer.  */
// u_TODO - once pedal ADC stuff is set up in CubeMX, make sure this order is accurate.
typedef enum {
    ACCEL_PEDAL_1, /* Sensor 1 for the Acceleration Pedal. */
    ACCEL_PEDAL_2, /* Sensor 2 for the Acceleration Pedal. */
    BRAKE_PEDAL_1, /* Sensor 1 for the Brake Pedal. */
    BRAKE_PEDAL_2, /* Sensor 2 for the Brake Pedal. */
} pedal_sensor_t;

/* API */
uint16_t pedals_getRaw(pedal_sensor_t pedal_sensor);
float pedals_getSensorVoltage(pedal_sensor_t pedal_sensor);
float pedal_percent_pressed(float voltage, float offset, float max);

#endif /* u_pedals.h */