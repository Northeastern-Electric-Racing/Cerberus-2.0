#include "can_messages_rx.h"

void receive_front_msb_env(const can_msg_t *message, front_msb_env_t *front_msb_env) {
    
    uint32_t data_bigendian;
    memcpy(&data_bigendian, message->data, 4);
    uint32_t data = __builtin_bswap32(data_bigendian);
    uint64_t temp_mask = (1ULL << 16) - 1ULL;
    uint64_t temp_raw = (data >> 16) & temp_mask;
    front_msb_env->temp = (float)(temp_raw / 10);
    uint64_t humidity_mask = (1ULL << 16) - 1ULL;
    uint64_t humidity_raw = (data >> 0) & humidity_mask;
    front_msb_env->humidity = (float)(humidity_raw / 10);
}

void receive_front_msb_accel(const can_msg_t *message, front_msb_accel_t *front_msb_accel) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_force_mask = (1ULL << 16) - 1ULL;
    uint64_t x_force_bits = (data >> 48) & x_force_mask;
    int64_t x_force_raw = (x_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_force_bits | ~x_force_mask)
        : (int64_t)x_force_bits;
    front_msb_accel->x_force = (float)x_force_raw;
    uint64_t y_force_mask = (1ULL << 16) - 1ULL;
    uint64_t y_force_bits = (data >> 32) & y_force_mask;
    int64_t y_force_raw = (y_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_force_bits | ~y_force_mask)
        : (int64_t)y_force_bits;
    front_msb_accel->y_force = (float)y_force_raw;
    uint64_t z_force_mask = (1ULL << 16) - 1ULL;
    uint64_t z_force_bits = (data >> 16) & z_force_mask;
    int64_t z_force_raw = (z_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_force_bits | ~z_force_mask)
        : (int64_t)z_force_bits;
    front_msb_accel->z_force = (float)z_force_raw;
}

void receive_front_msb_gyro(const can_msg_t *message, front_msb_gyro_t *front_msb_gyro) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t x_deg_bits = (data >> 48) & x_deg_mask;
    int64_t x_deg_raw = (x_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_deg_bits | ~x_deg_mask)
        : (int64_t)x_deg_bits;
    front_msb_gyro->x_deg = (float)x_deg_raw;
    uint64_t y_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t y_deg_bits = (data >> 32) & y_deg_mask;
    int64_t y_deg_raw = (y_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_deg_bits | ~y_deg_mask)
        : (int64_t)y_deg_bits;
    front_msb_gyro->y_deg = (float)y_deg_raw;
    uint64_t z_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t z_deg_bits = (data >> 16) & z_deg_mask;
    int64_t z_deg_raw = (z_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_deg_bits | ~z_deg_mask)
        : (int64_t)z_deg_bits;
    front_msb_gyro->z_deg = (float)z_deg_raw;
}

void receive_front_msb_strain(const can_msg_t *message, front_msb_strain_t *front_msb_strain) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t strain1_mask = (1ULL << 32) - 1ULL;
    uint64_t strain1_raw = (data >> 32) & strain1_mask;
    front_msb_strain->strain1 = (uint32_t)strain1_raw;
    uint64_t strain2_mask = (1ULL << 32) - 1ULL;
    uint64_t strain2_raw = (data >> 0) & strain2_mask;
    front_msb_strain->strain2 = (uint32_t)strain2_raw;
}

void receive_front_shockpot(const can_msg_t *message, front_shockpot_t *front_shockpot) {
    
    struct __attribute__((__packed__)) {
        uint32_t shock1;
        uint16_t shock1_raw;
        
    } bitstream_data;

    memcpy(&bitstream_data, message->data, sizeof(bitstream_data));

    
    
    front_shockpot->shock1 = (float)bitstream_data.shock1;
    
    
    
    front_shockpot->shock1_raw = (uint16_t)bitstream_data.shock1_raw;
    
    
}

void receive_front_ride_height(const can_msg_t *message, front_ride_height_t *front_ride_height) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t rh_mask = (1ULL << 16) - 1ULL;
    uint64_t rh_bits = (data >> 0) & rh_mask;
    int64_t rh_raw = (rh_bits & (1ULL << (16 - 1)))
        ? (int64_t)(rh_bits | ~rh_mask)
        : (int64_t)rh_bits;
    front_ride_height->rh = (float)rh_raw;
}

void receive_front_wheel_temp(const can_msg_t *message, front_wheel_temp_t *front_wheel_temp) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t wheel_temp_mask = (1ULL << 16) - 1ULL;
    uint64_t wheel_temp_raw = (data >> 0) & wheel_temp_mask;
    front_wheel_temp->wheel_temp = (float)wheel_temp_raw;
}

void receive_front_msb_orientation(const can_msg_t *message, front_msb_orientation_t *front_msb_orientation) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t x_fdeg_bits = (data >> 48) & x_fdeg_mask;
    int64_t x_fdeg_raw = (x_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_fdeg_bits | ~x_fdeg_mask)
        : (int64_t)x_fdeg_bits;
    front_msb_orientation->x_fdeg = (float)x_fdeg_raw;
    uint64_t y_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t y_fdeg_bits = (data >> 32) & y_fdeg_mask;
    int64_t y_fdeg_raw = (y_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_fdeg_bits | ~y_fdeg_mask)
        : (int64_t)y_fdeg_bits;
    front_msb_orientation->y_fdeg = (float)y_fdeg_raw;
    uint64_t z_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t z_fdeg_bits = (data >> 16) & z_fdeg_mask;
    int64_t z_fdeg_raw = (z_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_fdeg_bits | ~z_fdeg_mask)
        : (int64_t)z_fdeg_bits;
    front_msb_orientation->z_fdeg = (float)z_fdeg_raw;
}

void receive_back_msb_env(const can_msg_t *message, back_msb_env_t *back_msb_env) {
    
    uint32_t data_bigendian;
    memcpy(&data_bigendian, message->data, 4);
    uint32_t data = __builtin_bswap32(data_bigendian);
    uint64_t temp_mask = (1ULL << 16) - 1ULL;
    uint64_t temp_raw = (data >> 16) & temp_mask;
    back_msb_env->temp = (float)(temp_raw / 10);
    uint64_t humidity_mask = (1ULL << 16) - 1ULL;
    uint64_t humidity_raw = (data >> 0) & humidity_mask;
    back_msb_env->humidity = (float)(humidity_raw / 10);
}

void receive_back_msb_accel(const can_msg_t *message, back_msb_accel_t *back_msb_accel) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_force_mask = (1ULL << 16) - 1ULL;
    uint64_t x_force_bits = (data >> 48) & x_force_mask;
    int64_t x_force_raw = (x_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_force_bits | ~x_force_mask)
        : (int64_t)x_force_bits;
    back_msb_accel->x_force = (float)x_force_raw;
    uint64_t y_force_mask = (1ULL << 16) - 1ULL;
    uint64_t y_force_bits = (data >> 32) & y_force_mask;
    int64_t y_force_raw = (y_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_force_bits | ~y_force_mask)
        : (int64_t)y_force_bits;
    back_msb_accel->y_force = (float)y_force_raw;
    uint64_t z_force_mask = (1ULL << 16) - 1ULL;
    uint64_t z_force_bits = (data >> 16) & z_force_mask;
    int64_t z_force_raw = (z_force_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_force_bits | ~z_force_mask)
        : (int64_t)z_force_bits;
    back_msb_accel->z_force = (float)z_force_raw;
}

void receive_back_msb_gyro(const can_msg_t *message, back_msb_gyro_t *back_msb_gyro) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t x_deg_bits = (data >> 48) & x_deg_mask;
    int64_t x_deg_raw = (x_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_deg_bits | ~x_deg_mask)
        : (int64_t)x_deg_bits;
    back_msb_gyro->x_deg = (float)x_deg_raw;
    uint64_t y_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t y_deg_bits = (data >> 32) & y_deg_mask;
    int64_t y_deg_raw = (y_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_deg_bits | ~y_deg_mask)
        : (int64_t)y_deg_bits;
    back_msb_gyro->y_deg = (float)y_deg_raw;
    uint64_t z_deg_mask = (1ULL << 16) - 1ULL;
    uint64_t z_deg_bits = (data >> 16) & z_deg_mask;
    int64_t z_deg_raw = (z_deg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_deg_bits | ~z_deg_mask)
        : (int64_t)z_deg_bits;
    back_msb_gyro->z_deg = (float)z_deg_raw;
}

void receive_back_msb_strain(const can_msg_t *message, back_msb_strain_t *back_msb_strain) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t strain1_mask = (1ULL << 32) - 1ULL;
    uint64_t strain1_raw = (data >> 32) & strain1_mask;
    back_msb_strain->strain1 = (uint32_t)strain1_raw;
    uint64_t strain2_mask = (1ULL << 32) - 1ULL;
    uint64_t strain2_raw = (data >> 0) & strain2_mask;
    back_msb_strain->strain2 = (uint32_t)strain2_raw;
}

void receive_back_shockpot(const can_msg_t *message, back_shockpot_t *back_shockpot) {
    
    struct __attribute__((__packed__)) {
        uint32_t shock1;
        uint16_t shock1_raw;
        
    } bitstream_data;

    memcpy(&bitstream_data, message->data, sizeof(bitstream_data));

    
    
    back_shockpot->shock1 = (float)bitstream_data.shock1;
    
    
    
    back_shockpot->shock1_raw = (uint16_t)bitstream_data.shock1_raw;
    
    
}

void receive_back_ride_height(const can_msg_t *message, back_ride_height_t *back_ride_height) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t rh_mask = (1ULL << 16) - 1ULL;
    uint64_t rh_bits = (data >> 0) & rh_mask;
    int64_t rh_raw = (rh_bits & (1ULL << (16 - 1)))
        ? (int64_t)(rh_bits | ~rh_mask)
        : (int64_t)rh_bits;
    back_ride_height->rh = (float)rh_raw;
}

void receive_back_wheel_temp(const can_msg_t *message, back_wheel_temp_t *back_wheel_temp) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t wheel_temp_mask = (1ULL << 16) - 1ULL;
    uint64_t wheel_temp_raw = (data >> 0) & wheel_temp_mask;
    back_wheel_temp->wheel_temp = (float)wheel_temp_raw;
}

void receive_back_msb_orientation(const can_msg_t *message, back_msb_orientation_t *back_msb_orientation) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t x_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t x_fdeg_bits = (data >> 48) & x_fdeg_mask;
    int64_t x_fdeg_raw = (x_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(x_fdeg_bits | ~x_fdeg_mask)
        : (int64_t)x_fdeg_bits;
    back_msb_orientation->x_fdeg = (float)x_fdeg_raw;
    uint64_t y_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t y_fdeg_bits = (data >> 32) & y_fdeg_mask;
    int64_t y_fdeg_raw = (y_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(y_fdeg_bits | ~y_fdeg_mask)
        : (int64_t)y_fdeg_bits;
    back_msb_orientation->y_fdeg = (float)y_fdeg_raw;
    uint64_t z_fdeg_mask = (1ULL << 16) - 1ULL;
    uint64_t z_fdeg_bits = (data >> 16) & z_fdeg_mask;
    int64_t z_fdeg_raw = (z_fdeg_bits & (1ULL << (16 - 1)))
        ? (int64_t)(z_fdeg_bits | ~z_fdeg_mask)
        : (int64_t)z_fdeg_bits;
    back_msb_orientation->z_fdeg = (float)z_fdeg_raw;
}

void receive_max_ac_current_command(const can_msg_t *message, max_ac_current_command_t *max_ac_current_command) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t max_current_ac_target_mask = (1ULL << 16) - 1ULL;
    uint64_t max_current_ac_target_bits = (data >> 48) & max_current_ac_target_mask;
    int64_t max_current_ac_target_raw = (max_current_ac_target_bits & (1ULL << (16 - 1)))
        ? (int64_t)(max_current_ac_target_bits | ~max_current_ac_target_mask)
        : (int64_t)max_current_ac_target_bits;
    max_ac_current_command->max_current_ac_target = (float)(max_current_ac_target_raw / 10);
}

void receive_max_ac_brake_current_command(const can_msg_t *message, max_ac_brake_current_command_t *max_ac_brake_current_command) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t max_ac_brake_current_target_mask = (1ULL << 16) - 1ULL;
    uint64_t max_ac_brake_current_target_bits = (data >> 48) & max_ac_brake_current_target_mask;
    int64_t max_ac_brake_current_target_raw = (max_ac_brake_current_target_bits & (1ULL << (16 - 1)))
        ? (int64_t)(max_ac_brake_current_target_bits | ~max_ac_brake_current_target_mask)
        : (int64_t)max_ac_brake_current_target_bits;
    max_ac_brake_current_command->max_ac_brake_current_target = (float)(max_ac_brake_current_target_raw / 10);
}

void receive_max_dc_current_command(const can_msg_t *message, max_dc_current_command_t *max_dc_current_command) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t max_dc_current_target_mask = (1ULL << 16) - 1ULL;
    uint64_t max_dc_current_target_bits = (data >> 0) & max_dc_current_target_mask;
    int64_t max_dc_current_target_raw = (max_dc_current_target_bits & (1ULL << (16 - 1)))
        ? (int64_t)(max_dc_current_target_bits | ~max_dc_current_target_mask)
        : (int64_t)max_dc_current_target_bits;
    max_dc_current_command->max_dc_current_target = (float)(max_dc_current_target_raw / 10);
}

void receive_max_dc_brake_current_command(const can_msg_t *message, max_dc_brake_current_command_t *max_dc_brake_current_command) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t max_dc_brake_current_target_mask = (1ULL << 16) - 1ULL;
    uint64_t max_dc_brake_current_target_bits = (data >> 0) & max_dc_brake_current_target_mask;
    int64_t max_dc_brake_current_target_raw = (max_dc_brake_current_target_bits & (1ULL << (16 - 1)))
        ? (int64_t)(max_dc_brake_current_target_bits | ~max_dc_brake_current_target_mask)
        : (int64_t)max_dc_brake_current_target_bits;
    max_dc_brake_current_command->max_dc_brake_current_target = (float)(max_dc_brake_current_target_raw / 10);
}

void receive_dashboard_efuse_state(const can_msg_t *message, dashboard_efuse_state_t *dashboard_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    dashboard_efuse_state->state = (uint8_t)state_raw;
}

void receive_brake_efuse_state(const can_msg_t *message, brake_efuse_state_t *brake_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    brake_efuse_state->state = (uint8_t)state_raw;
}

void receive_shutdown_efuse_state(const can_msg_t *message, shutdown_efuse_state_t *shutdown_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    shutdown_efuse_state->state = (uint8_t)state_raw;
}

void receive_lv_efuse_state(const can_msg_t *message, lv_efuse_state_t *lv_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    lv_efuse_state->state = (uint8_t)state_raw;
}

void receive_radfan_efuse_state(const can_msg_t *message, radfan_efuse_state_t *radfan_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    radfan_efuse_state->state = (uint8_t)state_raw;
}

void receive_fanbatt_efuse_state(const can_msg_t *message, fanbatt_efuse_state_t *fanbatt_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    fanbatt_efuse_state->state = (uint8_t)state_raw;
}

void receive_pumpone_efuse_state(const can_msg_t *message, pumpone_efuse_state_t *pumpone_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    pumpone_efuse_state->state = (uint8_t)state_raw;
}

void receive_pumptwo_efuse_state(const can_msg_t *message, pumptwo_efuse_state_t *pumptwo_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    pumptwo_efuse_state->state = (uint8_t)state_raw;
}

void receive_battbox_efuse_state(const can_msg_t *message, battbox_efuse_state_t *battbox_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    battbox_efuse_state->state = (uint8_t)state_raw;
}

void receive_mc_efuse_state(const can_msg_t *message, mc_efuse_state_t *mc_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    mc_efuse_state->state = (uint8_t)state_raw;
}

void receive_spare_efuse_state(const can_msg_t *message, spare_efuse_state_t *spare_efuse_state) {
    
    uint8_t data = message->data[0];
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 0) & state_mask;
    spare_efuse_state->state = (uint8_t)state_raw;
}

void receive_rtds_command_message(const can_msg_t *message, rtds_command_message_t *rtds_command_message) {
    
    uint8_t data = message->data[0];
    uint64_t command_mask = (1ULL << 8) - 1ULL;
    uint64_t command_raw = (data >> 0) & command_mask;
    rtds_command_message->command = (uint8_t)command_raw;
}

void receive_lightning_board_imu_acceleration_data(const can_msg_t *message, lightning_board_imu_acceleration_data_t *lightning_board_imu_acceleration_data) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t accel_x_mask = (1ULL << 16) - 1ULL;
    uint64_t accel_x_bits = (data >> 48) & accel_x_mask;
    int64_t accel_x_raw = (accel_x_bits & (1ULL << (16 - 1)))
        ? (int64_t)(accel_x_bits | ~accel_x_mask)
        : (int64_t)accel_x_bits;
    lightning_board_imu_acceleration_data->accel_x = (float)(accel_x_raw / 1000);
    uint64_t accel_y_mask = (1ULL << 16) - 1ULL;
    uint64_t accel_y_bits = (data >> 32) & accel_y_mask;
    int64_t accel_y_raw = (accel_y_bits & (1ULL << (16 - 1)))
        ? (int64_t)(accel_y_bits | ~accel_y_mask)
        : (int64_t)accel_y_bits;
    lightning_board_imu_acceleration_data->accel_y = (float)(accel_y_raw / 1000);
    uint64_t accel_z_mask = (1ULL << 16) - 1ULL;
    uint64_t accel_z_bits = (data >> 16) & accel_z_mask;
    int64_t accel_z_raw = (accel_z_bits & (1ULL << (16 - 1)))
        ? (int64_t)(accel_z_bits | ~accel_z_mask)
        : (int64_t)accel_z_bits;
    lightning_board_imu_acceleration_data->accel_z = (float)(accel_z_raw / 1000);
}

void receive_lightning_board_imu_gyro_data(const can_msg_t *message, lightning_board_imu_gyro_data_t *lightning_board_imu_gyro_data) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t gyro_x_mask = (1ULL << 16) - 1ULL;
    uint64_t gyro_x_bits = (data >> 48) & gyro_x_mask;
    int64_t gyro_x_raw = (gyro_x_bits & (1ULL << (16 - 1)))
        ? (int64_t)(gyro_x_bits | ~gyro_x_mask)
        : (int64_t)gyro_x_bits;
    lightning_board_imu_gyro_data->gyro_x = (float)(gyro_x_raw / 1000);
    uint64_t gyro_y_mask = (1ULL << 16) - 1ULL;
    uint64_t gyro_y_bits = (data >> 32) & gyro_y_mask;
    int64_t gyro_y_raw = (gyro_y_bits & (1ULL << (16 - 1)))
        ? (int64_t)(gyro_y_bits | ~gyro_y_mask)
        : (int64_t)gyro_y_bits;
    lightning_board_imu_gyro_data->gyro_y = (float)(gyro_y_raw / 1000);
    uint64_t gyro_z_mask = (1ULL << 16) - 1ULL;
    uint64_t gyro_z_bits = (data >> 16) & gyro_z_mask;
    int64_t gyro_z_raw = (gyro_z_bits & (1ULL << (16 - 1)))
        ? (int64_t)(gyro_z_bits | ~gyro_z_mask)
        : (int64_t)gyro_z_bits;
    lightning_board_imu_gyro_data->gyro_z = (float)(gyro_z_raw / 1000);
}

void receive_lightning_board_lightning_sensor_information(const can_msg_t *message, lightning_board_lightning_sensor_information_t *lightning_board_lightning_sensor_information) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t interrupt_mask = (1ULL << 8) - 1ULL;
    uint64_t interrupt_raw = (data >> 56) & interrupt_mask;
    lightning_board_lightning_sensor_information->interrupt = (uint8_t)interrupt_raw;
    uint64_t distance_mask = (1ULL << 8) - 1ULL;
    uint64_t distance_raw = (data >> 48) & distance_mask;
    lightning_board_lightning_sensor_information->distance = (uint8_t)distance_raw;
    uint64_t energy_mask = (1ULL << 32) - 1ULL;
    uint64_t energy_raw = (data >> 16) & energy_mask;
    lightning_board_lightning_sensor_information->energy = (uint32_t)energy_raw;
}

void receive_lightning_board_magnometer_sensor_information(const can_msg_t *message, lightning_board_magnometer_sensor_information_t *lightning_board_magnometer_sensor_information) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t mag_x_mask = (1ULL << 16) - 1ULL;
    uint64_t mag_x_bits = (data >> 48) & mag_x_mask;
    int64_t mag_x_raw = (mag_x_bits & (1ULL << (16 - 1)))
        ? (int64_t)(mag_x_bits | ~mag_x_mask)
        : (int64_t)mag_x_bits;
    lightning_board_magnometer_sensor_information->mag_x = (float)(mag_x_raw / 1000);
    uint64_t mag_y_mask = (1ULL << 16) - 1ULL;
    uint64_t mag_y_bits = (data >> 32) & mag_y_mask;
    int64_t mag_y_raw = (mag_y_bits & (1ULL << (16 - 1)))
        ? (int64_t)(mag_y_bits | ~mag_y_mask)
        : (int64_t)mag_y_bits;
    lightning_board_magnometer_sensor_information->mag_y = (float)(mag_y_raw / 1000);
    uint64_t mag_z_mask = (1ULL << 16) - 1ULL;
    uint64_t mag_z_bits = (data >> 16) & mag_z_mask;
    int64_t mag_z_raw = (mag_z_bits & (1ULL << (16 - 1)))
        ? (int64_t)(mag_z_bits | ~mag_z_mask)
        : (int64_t)mag_z_bits;
    lightning_board_magnometer_sensor_information->mag_z = (float)(mag_z_raw / 1000);
}

void receive_bms_charge_message_send(const can_msg_t *message, bms_charge_message_send_t *bms_charge_message_send) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t charge_volts_mask = (1ULL << 16) - 1ULL;
    uint64_t charge_volts_raw = (data >> 48) & charge_volts_mask;
    bms_charge_message_send->charge_volts = (float)(charge_volts_raw / 10);
    uint64_t charge_current_mask = (1ULL << 16) - 1ULL;
    uint64_t charge_current_raw = (data >> 32) & charge_current_mask;
    bms_charge_message_send->charge_current = (float)(charge_current_raw / 10);
    uint64_t enable_charging_mask = (1ULL << 8) - 1ULL;
    uint64_t enable_charging_raw = (data >> 24) & enable_charging_mask;
    bms_charge_message_send->enable_charging = (uint8_t)enable_charging_raw;
}

void receive_pack_status(const can_msg_t *message, pack_status_t *pack_status) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t voltage_mask = (1ULL << 16) - 1ULL;
    uint64_t voltage_raw = (data >> 48) & voltage_mask;
    pack_status->voltage = (float)(voltage_raw / 10);
    uint64_t current_mask = (1ULL << 16) - 1ULL;
    uint64_t current_bits = (data >> 32) & current_mask;
    int64_t current_raw = (current_bits & (1ULL << (16 - 1)))
        ? (int64_t)(current_bits | ~current_mask)
        : (int64_t)current_bits;
    pack_status->current = (float)(current_raw / 10);
    uint64_t amp_hours_mask = (1ULL << 16) - 1ULL;
    uint64_t amp_hours_raw = (data >> 16) & amp_hours_mask;
    pack_status->amp_hours = (float)amp_hours_raw;
    uint64_t soc_mask = (1ULL << 8) - 1ULL;
    uint64_t soc_raw = (data >> 8) & soc_mask;
    pack_status->soc = (float)soc_raw;
    uint64_t health_mask = (1ULL << 8) - 1ULL;
    uint64_t health_raw = (data >> 0) & health_mask;
    pack_status->health = (float)health_raw;
}

void receive_bms_status(const can_msg_t *message, bms_status_t *bms_status) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 8) & state_mask;
    bms_status->state = (uint8_t)state_raw;
    uint64_t temp_average_mask = (1ULL << 8) - 1ULL;
    uint64_t temp_average_raw = (data >> 0) & temp_average_mask;
    bms_status->temp_average = (float)temp_average_raw;
}

void receive_cell_voltage(const can_msg_t *message, cell_voltage_t *cell_voltage) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t high_val_mask = (1ULL << 16) - 1ULL;
    uint64_t high_val_raw = (data >> 48) & high_val_mask;
    cell_voltage->high_val = (float)(high_val_raw / 10000);
    uint64_t high_chip_mask = (1ULL << 4) - 1ULL;
    uint64_t high_chip_raw = (data >> 44) & high_chip_mask;
    cell_voltage->high_chip = (uint8_t)high_chip_raw;
    uint64_t high_cell_mask = (1ULL << 4) - 1ULL;
    uint64_t high_cell_raw = (data >> 40) & high_cell_mask;
    cell_voltage->high_cell = (uint8_t)high_cell_raw;
    uint64_t low_val_mask = (1ULL << 16) - 1ULL;
    uint64_t low_val_raw = (data >> 24) & low_val_mask;
    cell_voltage->low_val = (float)(low_val_raw / 10000);
    uint64_t low_chip_mask = (1ULL << 4) - 1ULL;
    uint64_t low_chip_raw = (data >> 20) & low_chip_mask;
    cell_voltage->low_chip = (uint8_t)low_chip_raw;
    uint64_t low_cell_mask = (1ULL << 4) - 1ULL;
    uint64_t low_cell_raw = (data >> 16) & low_cell_mask;
    cell_voltage->low_cell = (uint8_t)low_cell_raw;
    uint64_t avg_val_mask = (1ULL << 16) - 1ULL;
    uint64_t avg_val_raw = (data >> 0) & avg_val_mask;
    cell_voltage->avg_val = (float)(avg_val_raw / 10000);
}

void receive_cell_temperatures(const can_msg_t *message, cell_temperatures_t *cell_temperatures) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t high_val_mask = (1ULL << 16) - 1ULL;
    uint64_t high_val_raw = (data >> 48) & high_val_mask;
    cell_temperatures->high_val = (float)(high_val_raw / 100);
    uint64_t high_chip_mask = (1ULL << 4) - 1ULL;
    uint64_t high_chip_raw = (data >> 44) & high_chip_mask;
    cell_temperatures->high_chip = (uint8_t)high_chip_raw;
    uint64_t high_cell_mask = (1ULL << 4) - 1ULL;
    uint64_t high_cell_raw = (data >> 40) & high_cell_mask;
    cell_temperatures->high_cell = (uint8_t)high_cell_raw;
    uint64_t low_val_mask = (1ULL << 16) - 1ULL;
    uint64_t low_val_raw = (data >> 24) & low_val_mask;
    cell_temperatures->low_val = (float)(low_val_raw / 100);
    uint64_t low_chip_mask = (1ULL << 4) - 1ULL;
    uint64_t low_chip_raw = (data >> 20) & low_chip_mask;
    cell_temperatures->low_chip = (uint8_t)low_chip_raw;
    uint64_t low_cell_mask = (1ULL << 4) - 1ULL;
    uint64_t low_cell_raw = (data >> 16) & low_cell_mask;
    cell_temperatures->low_cell = (uint8_t)low_cell_raw;
    uint64_t avg_val_mask = (1ULL << 16) - 1ULL;
    uint64_t avg_val_raw = (data >> 0) & avg_val_mask;
    cell_temperatures->avg_val = (float)(avg_val_raw / 100);
}

void receive_segment_temperatures(const can_msg_t *message, segment_temperatures_t *segment_temperatures) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t seg1_mask = (1ULL << 8) - 1ULL;
    uint64_t seg1_bits = (data >> 56) & seg1_mask;
    int64_t seg1_raw = (seg1_bits & (1ULL << (8 - 1)))
        ? (int64_t)(seg1_bits | ~seg1_mask)
        : (int64_t)seg1_bits;
    segment_temperatures->seg1 = (float)seg1_raw;
    uint64_t seg2_mask = (1ULL << 8) - 1ULL;
    uint64_t seg2_bits = (data >> 48) & seg2_mask;
    int64_t seg2_raw = (seg2_bits & (1ULL << (8 - 1)))
        ? (int64_t)(seg2_bits | ~seg2_mask)
        : (int64_t)seg2_bits;
    segment_temperatures->seg2 = (float)seg2_raw;
    uint64_t seg3_mask = (1ULL << 8) - 1ULL;
    uint64_t seg3_bits = (data >> 40) & seg3_mask;
    int64_t seg3_raw = (seg3_bits & (1ULL << (8 - 1)))
        ? (int64_t)(seg3_bits | ~seg3_mask)
        : (int64_t)seg3_bits;
    segment_temperatures->seg3 = (float)seg3_raw;
    uint64_t seg4_mask = (1ULL << 8) - 1ULL;
    uint64_t seg4_bits = (data >> 32) & seg4_mask;
    int64_t seg4_raw = (seg4_bits & (1ULL << (8 - 1)))
        ? (int64_t)(seg4_bits | ~seg4_mask)
        : (int64_t)seg4_bits;
    segment_temperatures->seg4 = (float)seg4_raw;
    uint64_t seg5_mask = (1ULL << 8) - 1ULL;
    uint64_t seg5_bits = (data >> 24) & seg5_mask;
    int64_t seg5_raw = (seg5_bits & (1ULL << (8 - 1)))
        ? (int64_t)(seg5_bits | ~seg5_mask)
        : (int64_t)seg5_bits;
    segment_temperatures->seg5 = (float)seg5_raw;
}

void receive_segment_isospi_communication_status(const can_msg_t *message, segment_isospi_communication_status_t *segment_isospi_communication_status) {
    
    uint32_t data_bigendian;
    memcpy(&data_bigendian, message->data, 4);
    uint32_t data = __builtin_bswap32(data_bigendian);
    uint64_t state_mask = (1ULL << 8) - 1ULL;
    uint64_t state_raw = (data >> 24) & state_mask;
    segment_isospi_communication_status->state = (uint8_t)state_raw;
    uint64_t break_location_mask = (1ULL << 8) - 1ULL;
    uint64_t break_location_raw = (data >> 16) & break_location_mask;
    segment_isospi_communication_status->break_location = (uint8_t)break_location_raw;
    uint64_t verification_attempts_mask = (1ULL << 8) - 1ULL;
    uint64_t verification_attempts_raw = (data >> 8) & verification_attempts_mask;
    segment_isospi_communication_status->verification_attempts = (uint8_t)verification_attempts_raw;
    uint64_t recovery_successful_mask = (1ULL << 1) - 1ULL;
    uint64_t recovery_successful_raw = (data >> 7) & recovery_successful_mask;
    segment_isospi_communication_status->recovery_successful = (uint8_t)recovery_successful_raw;
}

void receive_fault_status(const can_msg_t *message, fault_status_t *fault_status) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t dcl_enforce_mask = (1ULL << 1) - 1ULL;
    uint64_t dcl_enforce_raw = (data >> 15) & dcl_enforce_mask;
    fault_status->dcl_enforce = (bool)dcl_enforce_raw;
    uint64_t ccl_enforce_mask = (1ULL << 1) - 1ULL;
    uint64_t ccl_enforce_raw = (data >> 14) & ccl_enforce_mask;
    fault_status->ccl_enforce = (bool)ccl_enforce_raw;
    uint64_t low_cell_volt_mask = (1ULL << 1) - 1ULL;
    uint64_t low_cell_volt_raw = (data >> 13) & low_cell_volt_mask;
    fault_status->low_cell_volt = (bool)low_cell_volt_raw;
    uint64_t high_cell_volt_mask = (1ULL << 1) - 1ULL;
    uint64_t high_cell_volt_raw = (data >> 12) & high_cell_volt_mask;
    fault_status->high_cell_volt = (bool)high_cell_volt_raw;
    uint64_t high_charge_volt_mask = (1ULL << 1) - 1ULL;
    uint64_t high_charge_volt_raw = (data >> 11) & high_charge_volt_mask;
    fault_status->high_charge_volt = (bool)high_charge_volt_raw;
    uint64_t pack_hot_mask = (1ULL << 1) - 1ULL;
    uint64_t pack_hot_raw = (data >> 10) & pack_hot_mask;
    fault_status->pack_hot = (bool)pack_hot_raw;
    uint64_t die_temp_max_mask = (1ULL << 1) - 1ULL;
    uint64_t die_temp_max_raw = (data >> 9) & die_temp_max_mask;
    fault_status->die_temp_max = (bool)die_temp_max_raw;
    uint64_t segment_comms_mask = (1ULL << 1) - 1ULL;
    uint64_t segment_comms_raw = (data >> 8) & segment_comms_mask;
    fault_status->segment_comms = (bool)segment_comms_raw;
    uint64_t hv_plate_comms_mask = (1ULL << 1) - 1ULL;
    uint64_t hv_plate_comms_raw = (data >> 7) & hv_plate_comms_mask;
    fault_status->hv_plate_comms = (bool)hv_plate_comms_raw;
}

void receive_segment_average_voltages(const can_msg_t *message, segment_average_voltages_t *segment_average_voltages) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t seg1_mask = (1ULL << 12) - 1ULL;
    uint64_t seg1_raw = (data >> 52) & seg1_mask;
    segment_average_voltages->seg1 = (float)(seg1_raw / 1000);
    uint64_t seg2_mask = (1ULL << 12) - 1ULL;
    uint64_t seg2_raw = (data >> 40) & seg2_mask;
    segment_average_voltages->seg2 = (float)(seg2_raw / 1000);
    uint64_t seg3_mask = (1ULL << 12) - 1ULL;
    uint64_t seg3_raw = (data >> 28) & seg3_mask;
    segment_average_voltages->seg3 = (float)(seg3_raw / 1000);
    uint64_t seg4_mask = (1ULL << 12) - 1ULL;
    uint64_t seg4_raw = (data >> 16) & seg4_mask;
    segment_average_voltages->seg4 = (float)(seg4_raw / 1000);
    uint64_t seg5_mask = (1ULL << 12) - 1ULL;
    uint64_t seg5_raw = (data >> 4) & seg5_mask;
    segment_average_voltages->seg5 = (float)(seg5_raw / 1000);
}

void receive_segment_total_voltages(const can_msg_t *message, segment_total_voltages_t *segment_total_voltages) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t seg1_mask = (1ULL << 12) - 1ULL;
    uint64_t seg1_raw = (data >> 52) & seg1_mask;
    segment_total_voltages->seg1 = (float)(seg1_raw / 39);
    uint64_t seg2_mask = (1ULL << 12) - 1ULL;
    uint64_t seg2_raw = (data >> 40) & seg2_mask;
    segment_total_voltages->seg2 = (float)(seg2_raw / 39);
    uint64_t seg3_mask = (1ULL << 12) - 1ULL;
    uint64_t seg3_raw = (data >> 28) & seg3_mask;
    segment_total_voltages->seg3 = (float)(seg3_raw / 39);
    uint64_t seg4_mask = (1ULL << 12) - 1ULL;
    uint64_t seg4_raw = (data >> 16) & seg4_mask;
    segment_total_voltages->seg4 = (float)(seg4_raw / 39);
    uint64_t seg5_mask = (1ULL << 12) - 1ULL;
    uint64_t seg5_raw = (data >> 4) & seg5_mask;
    segment_total_voltages->seg5 = (float)(seg5_raw / 39);
}

void receive_segment_delta_voltages(const can_msg_t *message, segment_delta_voltages_t *segment_delta_voltages) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t seg1_mask = (1ULL << 12) - 1ULL;
    uint64_t seg1_raw = (data >> 52) & seg1_mask;
    segment_delta_voltages->seg1 = (float)(seg1_raw / 1000);
    uint64_t seg2_mask = (1ULL << 12) - 1ULL;
    uint64_t seg2_raw = (data >> 40) & seg2_mask;
    segment_delta_voltages->seg2 = (float)(seg2_raw / 1000);
    uint64_t seg3_mask = (1ULL << 12) - 1ULL;
    uint64_t seg3_raw = (data >> 28) & seg3_mask;
    segment_delta_voltages->seg3 = (float)(seg3_raw / 1000);
    uint64_t seg4_mask = (1ULL << 12) - 1ULL;
    uint64_t seg4_raw = (data >> 16) & seg4_mask;
    segment_delta_voltages->seg4 = (float)(seg4_raw / 1000);
    uint64_t seg5_mask = (1ULL << 12) - 1ULL;
    uint64_t seg5_raw = (data >> 4) & seg5_mask;
    segment_delta_voltages->seg5 = (float)(seg5_raw / 1000);
}

void receive_bms_debug(const can_msg_t *message, bms_debug_t *bms_debug) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t spare0_mask = (1ULL << 8) - 1ULL;
    uint64_t spare0_raw = (data >> 56) & spare0_mask;
    bms_debug->spare0 = (uint8_t)spare0_raw;
    uint64_t spare1_mask = (1ULL << 8) - 1ULL;
    uint64_t spare1_raw = (data >> 48) & spare1_mask;
    bms_debug->spare1 = (uint8_t)spare1_raw;
    uint64_t spare2_mask = (1ULL << 16) - 1ULL;
    uint64_t spare2_raw = (data >> 32) & spare2_mask;
    bms_debug->spare2 = (uint16_t)spare2_raw;
    uint64_t spare3_mask = (1ULL << 32) - 1ULL;
    uint64_t spare3_raw = (data >> 0) & spare3_mask;
    bms_debug->spare3 = (uint32_t)spare3_raw;
}

void receive_bms_fault_timers(const can_msg_t *message, bms_fault_timers_t *bms_fault_timers) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t start_stop_mask = (1ULL << 8) - 1ULL;
    uint64_t start_stop_raw = (data >> 56) & start_stop_mask;
    bms_fault_timers->start_stop = (uint8_t)start_stop_raw;
    uint64_t code_mask = (1ULL << 8) - 1ULL;
    uint64_t code_raw = (data >> 48) & code_mask;
    bms_fault_timers->code = (uint8_t)code_raw;
    uint64_t value_mask = (1ULL << 32) - 1ULL;
    uint64_t value_raw = (data >> 16) & value_mask;
    bms_fault_timers->value = (float)value_raw;
}

void receive_shepherd_version_tag(const can_msg_t *message, shepherd_version_tag_t *shepherd_version_tag) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t major_mask = (1ULL << 8) - 1ULL;
    uint64_t major_raw = (data >> 56) & major_mask;
    shepherd_version_tag->major = (uint8_t)major_raw;
    uint64_t minor_mask = (1ULL << 8) - 1ULL;
    uint64_t minor_raw = (data >> 48) & minor_mask;
    shepherd_version_tag->minor = (uint8_t)minor_raw;
    uint64_t patch_mask = (1ULL << 8) - 1ULL;
    uint64_t patch_raw = (data >> 40) & patch_mask;
    shepherd_version_tag->patch = (uint8_t)patch_raw;
    uint64_t dirty_mask = (1ULL << 8) - 1ULL;
    uint64_t dirty_raw = (data >> 32) & dirty_mask;
    shepherd_version_tag->dirty = (bool)dirty_raw;
    uint64_t local_commit_mask = (1ULL << 8) - 1ULL;
    uint64_t local_commit_raw = (data >> 24) & local_commit_mask;
    shepherd_version_tag->local_commit = (bool)local_commit_raw;
}

void receive_shepherd_version_hash(const can_msg_t *message, shepherd_version_hash_t *shepherd_version_hash) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t short_hash_mask = (1ULL << 32) - 1ULL;
    uint64_t short_hash_raw = (data >> 32) & short_hash_mask;
    shepherd_version_hash->short_hash = (uint32_t)short_hash_raw;
    uint64_t author_hash_mask = (1ULL << 32) - 1ULL;
    uint64_t author_hash_raw = (data >> 0) & author_hash_mask;
    shepherd_version_hash->author_hash = (uint32_t)author_hash_raw;
}

void receive_overflow_notification_for_percell(const can_msg_t *message, overflow_notification_for_percell_t *overflow_notification_for_percell) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t overflow_can_id_mask = (1ULL << 32) - 1ULL;
    uint64_t overflow_can_id_raw = (data >> 32) & overflow_can_id_mask;
    overflow_notification_for_percell->overflow_can_id = (uint32_t)overflow_can_id_raw;
    uint64_t overflow_cnt_mask = (1ULL << 16) - 1ULL;
    uint64_t overflow_cnt_raw = (data >> 16) & overflow_cnt_mask;
    overflow_notification_for_percell->overflow_cnt = (uint16_t)overflow_cnt_raw;
}

void receive_alpha_cell_data_debug(const can_msg_t *message, alpha_cell_data_debug_t *alpha_cell_data_debug) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t therm_mask = (1ULL << 10) - 1ULL;
    uint64_t therm_raw = (data >> 54) & therm_mask;
    alpha_cell_data_debug->therm = (float)(therm_raw / 10);
    uint64_t voltage_a_mask = (1ULL << 13) - 1ULL;
    uint64_t voltage_a_raw = (data >> 41) & voltage_a_mask;
    alpha_cell_data_debug->voltage_a = (float)(voltage_a_raw / 1000);
    uint64_t voltage_b_mask = (1ULL << 13) - 1ULL;
    uint64_t voltage_b_raw = (data >> 28) & voltage_b_mask;
    alpha_cell_data_debug->voltage_b = (float)(voltage_b_raw / 1000);
    uint64_t chip_id_mask = (1ULL << 4) - 1ULL;
    uint64_t chip_id_raw = (data >> 24) & chip_id_mask;
    alpha_cell_data_debug->chip_id = (uint8_t)chip_id_raw;
    uint64_t cell_a_mask = (1ULL << 4) - 1ULL;
    uint64_t cell_a_raw = (data >> 20) & cell_a_mask;
    alpha_cell_data_debug->cell_a = (uint8_t)cell_a_raw;
    uint64_t cell_b_mask = (1ULL << 4) - 1ULL;
    uint64_t cell_b_raw = (data >> 16) & cell_b_mask;
    alpha_cell_data_debug->cell_b = (uint8_t)cell_b_raw;
    uint64_t discharging_a_mask = (1ULL << 1) - 1ULL;
    uint64_t discharging_a_raw = (data >> 15) & discharging_a_mask;
    alpha_cell_data_debug->discharging_a = (bool)discharging_a_raw;
    uint64_t discharging_b_mask = (1ULL << 1) - 1ULL;
    uint64_t discharging_b_raw = (data >> 14) & discharging_b_mask;
    alpha_cell_data_debug->discharging_b = (bool)discharging_b_raw;
    uint64_t cvs_a_mask = (1ULL << 1) - 1ULL;
    uint64_t cvs_a_raw = (data >> 13) & cvs_a_mask;
    alpha_cell_data_debug->cvs_a = (bool)cvs_a_raw;
    uint64_t cvs_b_mask = (1ULL << 1) - 1ULL;
    uint64_t cvs_b_raw = (data >> 12) & cvs_b_mask;
    alpha_cell_data_debug->cvs_b = (bool)cvs_b_raw;
}

void receive_beta_cell_data_debug(const can_msg_t *message, beta_cell_data_debug_t *beta_cell_data_debug) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t therm_mask = (1ULL << 10) - 1ULL;
    uint64_t therm_raw = (data >> 54) & therm_mask;
    beta_cell_data_debug->therm = (float)(therm_raw / 10);
    uint64_t voltage_a_mask = (1ULL << 13) - 1ULL;
    uint64_t voltage_a_raw = (data >> 41) & voltage_a_mask;
    beta_cell_data_debug->voltage_a = (float)(voltage_a_raw / 1000);
    uint64_t voltage_b_mask = (1ULL << 13) - 1ULL;
    uint64_t voltage_b_raw = (data >> 28) & voltage_b_mask;
    beta_cell_data_debug->voltage_b = (float)(voltage_b_raw / 1000);
    uint64_t chip_id_mask = (1ULL << 4) - 1ULL;
    uint64_t chip_id_raw = (data >> 24) & chip_id_mask;
    beta_cell_data_debug->chip_id = (uint8_t)chip_id_raw;
    uint64_t cell_a_mask = (1ULL << 4) - 1ULL;
    uint64_t cell_a_raw = (data >> 20) & cell_a_mask;
    beta_cell_data_debug->cell_a = (uint8_t)cell_a_raw;
    uint64_t cell_b_mask = (1ULL << 4) - 1ULL;
    uint64_t cell_b_raw = (data >> 16) & cell_b_mask;
    beta_cell_data_debug->cell_b = (uint8_t)cell_b_raw;
    uint64_t discharging_a_mask = (1ULL << 1) - 1ULL;
    uint64_t discharging_a_raw = (data >> 15) & discharging_a_mask;
    beta_cell_data_debug->discharging_a = (bool)discharging_a_raw;
    uint64_t discharging_b_mask = (1ULL << 1) - 1ULL;
    uint64_t discharging_b_raw = (data >> 14) & discharging_b_mask;
    beta_cell_data_debug->discharging_b = (bool)discharging_b_raw;
    uint64_t cvs_a_mask = (1ULL << 1) - 1ULL;
    uint64_t cvs_a_raw = (data >> 13) & cvs_a_mask;
    beta_cell_data_debug->cvs_a = (bool)cvs_a_raw;
    uint64_t cvs_b_mask = (1ULL << 1) - 1ULL;
    uint64_t cvs_b_raw = (data >> 12) & cvs_b_mask;
    beta_cell_data_debug->cvs_b = (bool)cvs_b_raw;
}

void receive_chip_a_debug(const can_msg_t *message, chip_a_debug_t *chip_a_debug) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t chip_id_mask = (1ULL << 4) - 1ULL;
    uint64_t chip_id_raw = (data >> 60) & chip_id_mask;
    chip_a_debug->chip_id = (uint8_t)chip_id_raw;
    uint64_t die_temp_mask = (1ULL << 13) - 1ULL;
    uint64_t die_temp_raw = (data >> 47) & die_temp_mask;
    chip_a_debug->die_temp = (float)(die_temp_raw / 100);
    uint64_t vpv_mask = (1ULL << 13) - 1ULL;
    uint64_t vpv_raw = (data >> 34) & vpv_mask;
    chip_a_debug->vpv = (float)(vpv_raw / 100);
    uint64_t vmv_mask = (1ULL << 13) - 1ULL;
    uint64_t vmv_raw = (data >> 21) & vmv_mask;
    chip_a_debug->vmv = (float)(vmv_raw / 1000);
    uint64_t va_ov_mask = (1ULL << 1) - 1ULL;
    uint64_t va_ov_raw = (data >> 20) & va_ov_mask;
    chip_a_debug->va_ov = (bool)va_ov_raw;
    uint64_t va_uv_mask = (1ULL << 1) - 1ULL;
    uint64_t va_uv_raw = (data >> 19) & va_uv_mask;
    chip_a_debug->va_uv = (bool)va_uv_raw;
    uint64_t vd_ov_mask = (1ULL << 1) - 1ULL;
    uint64_t vd_ov_raw = (data >> 18) & vd_ov_mask;
    chip_a_debug->vd_ov = (bool)vd_ov_raw;
    uint64_t vd_uv_mask = (1ULL << 1) - 1ULL;
    uint64_t vd_uv_raw = (data >> 17) & vd_uv_mask;
    chip_a_debug->vd_uv = (bool)vd_uv_raw;
    uint64_t vde_mask = (1ULL << 1) - 1ULL;
    uint64_t vde_raw = (data >> 16) & vde_mask;
    chip_a_debug->vde = (bool)vde_raw;
    uint64_t vdel_mask = (1ULL << 1) - 1ULL;
    uint64_t vdel_raw = (data >> 15) & vdel_mask;
    chip_a_debug->vdel = (bool)vdel_raw;
    uint64_t spiflt_mask = (1ULL << 1) - 1ULL;
    uint64_t spiflt_raw = (data >> 14) & spiflt_mask;
    chip_a_debug->spiflt = (bool)spiflt_raw;
    uint64_t sleep_mask = (1ULL << 1) - 1ULL;
    uint64_t sleep_raw = (data >> 13) & sleep_mask;
    chip_a_debug->sleep = (bool)sleep_raw;
    uint64_t thsd_mask = (1ULL << 1) - 1ULL;
    uint64_t thsd_raw = (data >> 12) & thsd_mask;
    chip_a_debug->thsd = (bool)thsd_raw;
    uint64_t tmodchk_mask = (1ULL << 1) - 1ULL;
    uint64_t tmodchk_raw = (data >> 11) & tmodchk_mask;
    chip_a_debug->tmodchk = (bool)tmodchk_raw;
    uint64_t oscchk_mask = (1ULL << 1) - 1ULL;
    uint64_t oscchk_raw = (data >> 10) & oscchk_mask;
    chip_a_debug->oscchk = (bool)oscchk_raw;
}

void receive_chip_b_debug(const can_msg_t *message, chip_b_debug_t *chip_b_debug) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t vres_mask = (1ULL << 13) - 1ULL;
    uint64_t vres_raw = (data >> 51) & vres_mask;
    chip_b_debug->vres = (float)(vres_raw / 1000);
    uint64_t chip_id_mask = (1ULL << 4) - 1ULL;
    uint64_t chip_id_raw = (data >> 47) & chip_id_mask;
    chip_b_debug->chip_id = (uint8_t)chip_id_raw;
    uint64_t vref2_mask = (1ULL << 13) - 1ULL;
    uint64_t vref2_raw = (data >> 34) & vref2_mask;
    chip_b_debug->vref2 = (float)(vref2_raw / 1000);
    uint64_t v_analog_mask = (1ULL << 13) - 1ULL;
    uint64_t v_analog_raw = (data >> 21) & v_analog_mask;
    chip_b_debug->v_analog = (float)(v_analog_raw / 1000);
    uint64_t v_digital_mask = (1ULL << 13) - 1ULL;
    uint64_t v_digital_raw = (data >> 8) & v_digital_mask;
    chip_b_debug->v_digital = (float)(v_digital_raw / 1000);
    uint64_t otp1_med_mask = (1ULL << 1) - 1ULL;
    uint64_t otp1_med_raw = (data >> 7) & otp1_med_mask;
    chip_b_debug->otp1_med = (bool)otp1_med_raw;
    uint64_t opt2_med_mask = (1ULL << 1) - 1ULL;
    uint64_t opt2_med_raw = (data >> 6) & opt2_med_mask;
    chip_b_debug->opt2_med = (bool)opt2_med_raw;
}

void receive_fan_duty_cycle_percentage(const can_msg_t *message, fan_duty_cycle_percentage_t *fan_duty_cycle_percentage) {
    
    uint8_t data = message->data[0];
    uint64_t fan_duty_cycle_mask = (1ULL << 8) - 1ULL;
    uint64_t fan_duty_cycle_raw = (data >> 0) & fan_duty_cycle_mask;
    fan_duty_cycle_percentage->fan_duty_cycle = (uint8_t)fan_duty_cycle_raw;
}

void receive_onboard_therm_temperatures(const can_msg_t *message, onboard_therm_temperatures_t *onboard_therm_temperatures) {
    
    uint8_t data = message->data[0];
    uint64_t chip_id_mask = (1ULL << 1) - 1ULL;
    uint64_t chip_id_raw = (data >> 7) & chip_id_mask;
    onboard_therm_temperatures->chip_id = (uint8_t)chip_id_raw;
    uint64_t therm_temp_1_mask = (1ULL << 2) - 1ULL;
    uint64_t therm_temp_1_raw = (data >> 5) & therm_temp_1_mask;
    onboard_therm_temperatures->therm_temp_1 = (float)therm_temp_1_raw;
    uint64_t therm_temp_2_mask = (1ULL << 2) - 1ULL;
    uint64_t therm_temp_2_raw = (data >> 3) & therm_temp_2_mask;
    onboard_therm_temperatures->therm_temp_2 = (float)therm_temp_2_raw;
    uint64_t therm_temp_3_mask = (1ULL << 2) - 1ULL;
    uint64_t therm_temp_3_raw = (data >> 1) & therm_temp_3_mask;
    onboard_therm_temperatures->therm_temp_3 = (float)therm_temp_3_raw;
}

void receive_precharge_status(const can_msg_t *message, precharge_status_t *precharge_status) {
    
    uint8_t data = message->data[0];
    uint64_t precharge_status_mask = (1ULL << 1) - 1ULL;
    uint64_t precharge_status_raw = (data >> 7) & precharge_status_mask;
    precharge_status->precharge_status = (bool)precharge_status_raw;
}

void receive_hv_plate_data(const can_msg_t *message, hv_plate_data_t *hv_plate_data) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t batt_voltage_mask = (1ULL << 16) - 1ULL;
    uint64_t batt_voltage_bits = (data >> 48) & batt_voltage_mask;
    int64_t batt_voltage_raw = (batt_voltage_bits & (1ULL << (16 - 1)))
        ? (int64_t)(batt_voltage_bits | ~batt_voltage_mask)
        : (int64_t)batt_voltage_bits;
    hv_plate_data->batt_voltage = (float)(batt_voltage_raw / 100);
    uint64_t ts_voltage_mask = (1ULL << 16) - 1ULL;
    uint64_t ts_voltage_bits = (data >> 32) & ts_voltage_mask;
    int64_t ts_voltage_raw = (ts_voltage_bits & (1ULL << (16 - 1)))
        ? (int64_t)(ts_voltage_bits | ~ts_voltage_mask)
        : (int64_t)ts_voltage_bits;
    hv_plate_data->ts_voltage = (float)(ts_voltage_raw / 100);
    uint64_t shunt_temp_mask = (1ULL << 16) - 1ULL;
    uint64_t shunt_temp_bits = (data >> 16) & shunt_temp_mask;
    int64_t shunt_temp_raw = (shunt_temp_bits & (1ULL << (16 - 1)))
        ? (int64_t)(shunt_temp_bits | ~shunt_temp_mask)
        : (int64_t)shunt_temp_bits;
    hv_plate_data->shunt_temp = (float)(shunt_temp_raw / 100);
    uint64_t pack_current_mask = (1ULL << 16) - 1ULL;
    uint64_t pack_current_bits = (data >> 0) & pack_current_mask;
    int64_t pack_current_raw = (pack_current_bits & (1ULL << (16 - 1)))
        ? (int64_t)(pack_current_bits | ~pack_current_mask)
        : (int64_t)pack_current_bits;
    hv_plate_data->pack_current = (float)(pack_current_raw / 100);
}

void receive_segment_pec_errors(const can_msg_t *message, segment_pec_errors_t *segment_pec_errors) {
    
    uint32_t data_bigendian;
    memcpy(&data_bigendian, message->data, 4);
    uint32_t data = __builtin_bswap32(data_bigendian);
    uint64_t chip_id_mask = (1ULL << 8) - 1ULL;
    uint64_t chip_id_raw = (data >> 24) & chip_id_mask;
    segment_pec_errors->chip_id = (uint8_t)chip_id_raw;
    uint64_t pec_errors_mask = (1ULL << 16) - 1ULL;
    uint64_t pec_errors_raw = (data >> 8) & pec_errors_mask;
    segment_pec_errors->pec_errors = (uint16_t)pec_errors_raw;
}

void receive_hv_plate_pec_errors(const can_msg_t *message, hv_plate_pec_errors_t *hv_plate_pec_errors) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t pec_errors_mask = (1ULL << 16) - 1ULL;
    uint64_t pec_errors_raw = (data >> 0) & pec_errors_mask;
    hv_plate_pec_errors->pec_errors = (uint16_t)pec_errors_raw;
}

void receive_hv_plate_diagnostics(const can_msg_t *message, hv_plate_diagnostics_t *hv_plate_diagnostics) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t flags_mask = (1ULL << 12) - 1ULL;
    uint64_t flags_raw = (data >> 52) & flags_mask;
    hv_plate_diagnostics->flags = (uint16_t)flags_raw;
    uint64_t vreg_mask = (1ULL << 12) - 1ULL;
    uint64_t vreg_raw = (data >> 40) & vreg_mask;
    hv_plate_diagnostics->vreg = (float)(vreg_raw / 100);
    uint64_t tmp1_mask = (1ULL << 12) - 1ULL;
    uint64_t tmp1_raw = (data >> 28) & tmp1_mask;
    hv_plate_diagnostics->tmp1 = (float)(tmp1_raw / 100);
    uint64_t vref1p25_mask = (1ULL << 12) - 1ULL;
    uint64_t vref1p25_raw = (data >> 16) & vref1p25_mask;
    hv_plate_diagnostics->vref1p25 = (float)(vref1p25_raw / 100);
    uint64_t osccnt_mask = (1ULL << 16) - 1ULL;
    uint64_t osccnt_raw = (data >> 0) & osccnt_mask;
    hv_plate_diagnostics->osccnt = (uint16_t)osccnt_raw;
}

void receive_hv_plate_diagnostics_second(const can_msg_t *message, hv_plate_diagnostics_second_t *hv_plate_diagnostics_second) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t epad_mask = (1ULL << 12) - 1ULL;
    uint64_t epad_raw = (data >> 52) & epad_mask;
    hv_plate_diagnostics_second->epad = (float)(epad_raw / 100);
    uint64_t vdig_mask = (1ULL << 12) - 1ULL;
    uint64_t vdig_raw = (data >> 40) & vdig_mask;
    hv_plate_diagnostics_second->vdig = (float)(vdig_raw / 100);
    uint64_t vdd_mask = (1ULL << 12) - 1ULL;
    uint64_t vdd_raw = (data >> 28) & vdd_mask;
    hv_plate_diagnostics_second->vdd = (float)(vdd_raw / 100);
    uint64_t tmp2_mask = (1ULL << 12) - 1ULL;
    uint64_t tmp2_raw = (data >> 16) & tmp2_mask;
    hv_plate_diagnostics_second->tmp2 = (float)(tmp2_raw / 100);
    uint64_t vdiv_mask = (1ULL << 12) - 1ULL;
    uint64_t vdiv_raw = (data >> 4) & vdiv_mask;
    hv_plate_diagnostics_second->vdiv = (float)(vdiv_raw / 100);
}

void receive_bms_onboard_temperature(const can_msg_t *message, bms_onboard_temperature_t *bms_onboard_temperature) {
    
    uint16_t data_bigendian;
    memcpy(&data_bigendian, message->data, 2);
    uint16_t data = __builtin_bswap16(data_bigendian);
    uint64_t internal_temp_mask = (1ULL << 16) - 1ULL;
    uint64_t internal_temp_raw = (data >> 0) & internal_temp_mask;
    bms_onboard_temperature->internal_temp = (float)(internal_temp_raw / 100);
}

void receive_bms_imu_accelerometer(const can_msg_t *message, bms_imu_accelerometer_t *bms_imu_accelerometer) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t imu_accelerometer_x_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_accelerometer_x_bits = (data >> 48) & imu_accelerometer_x_mask;
    int64_t imu_accelerometer_x_raw = (imu_accelerometer_x_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_accelerometer_x_bits | ~imu_accelerometer_x_mask)
        : (int64_t)imu_accelerometer_x_bits;
    bms_imu_accelerometer->imu_accelerometer_x = (float)(imu_accelerometer_x_raw / 4);
    uint64_t imu_accelerometer_y_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_accelerometer_y_bits = (data >> 32) & imu_accelerometer_y_mask;
    int64_t imu_accelerometer_y_raw = (imu_accelerometer_y_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_accelerometer_y_bits | ~imu_accelerometer_y_mask)
        : (int64_t)imu_accelerometer_y_bits;
    bms_imu_accelerometer->imu_accelerometer_y = (float)(imu_accelerometer_y_raw / 4);
    uint64_t imu_accelerometer_z_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_accelerometer_z_bits = (data >> 16) & imu_accelerometer_z_mask;
    int64_t imu_accelerometer_z_raw = (imu_accelerometer_z_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_accelerometer_z_bits | ~imu_accelerometer_z_mask)
        : (int64_t)imu_accelerometer_z_bits;
    bms_imu_accelerometer->imu_accelerometer_z = (float)(imu_accelerometer_z_raw / 4);
}

void receive_bms_imu_gyro(const can_msg_t *message, bms_imu_gyro_t *bms_imu_gyro) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t imu_gyro_x_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_gyro_x_bits = (data >> 48) & imu_gyro_x_mask;
    int64_t imu_gyro_x_raw = (imu_gyro_x_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_gyro_x_bits | ~imu_gyro_x_mask)
        : (int64_t)imu_gyro_x_bits;
    bms_imu_gyro->imu_gyro_x = (float)(imu_gyro_x_raw / 100);
    uint64_t imu_gyro_y_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_gyro_y_bits = (data >> 32) & imu_gyro_y_mask;
    int64_t imu_gyro_y_raw = (imu_gyro_y_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_gyro_y_bits | ~imu_gyro_y_mask)
        : (int64_t)imu_gyro_y_bits;
    bms_imu_gyro->imu_gyro_y = (float)(imu_gyro_y_raw / 100);
    uint64_t imu_gyro_z_mask = (1ULL << 16) - 1ULL;
    uint64_t imu_gyro_z_bits = (data >> 16) & imu_gyro_z_mask;
    int64_t imu_gyro_z_raw = (imu_gyro_z_bits & (1ULL << (16 - 1)))
        ? (int64_t)(imu_gyro_z_bits | ~imu_gyro_z_mask)
        : (int64_t)imu_gyro_z_bits;
    bms_imu_gyro->imu_gyro_z = (float)(imu_gyro_z_raw / 100);
}

void receive_bms_test_message_one(const can_msg_t *message, bms_test_message_one_t *bms_test_message_one) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t one_mask = (1ULL << 32) - 1ULL;
    uint64_t one_bits = (data >> 32) & one_mask;
    int64_t one_raw = (one_bits & (1ULL << (32 - 1)))
        ? (int64_t)(one_bits | ~one_mask)
        : (int64_t)one_bits;
    bms_test_message_one->one = (float)(one_raw / 1000);
    uint64_t two_mask = (1ULL << 16) - 1ULL;
    uint64_t two_bits = (data >> 16) & two_mask;
    int64_t two_raw = (two_bits & (1ULL << (16 - 1)))
        ? (int64_t)(two_bits | ~two_mask)
        : (int64_t)two_bits;
    bms_test_message_one->two = (int16_t)two_raw;
    uint64_t three_mask = (1ULL << 8) - 1ULL;
    uint64_t three_raw = (data >> 8) & three_mask;
    bms_test_message_one->three = (uint8_t)three_raw;
}

void receive_bms_test_message_two(const can_msg_t *message, bms_test_message_two_t *bms_test_message_two) {
    
    uint64_t data_bigendian;
    memcpy(&data_bigendian, message->data, 8);
    uint64_t data = __builtin_bswap64(data_bigendian);
    uint64_t one_mask = (1ULL << 2) - 1ULL;
    uint64_t one_raw = (data >> 62) & one_mask;
    bms_test_message_two->one = (uint8_t)one_raw;
    uint64_t two_mask = (1ULL << 1) - 1ULL;
    uint64_t two_raw = (data >> 61) & two_mask;
    bms_test_message_two->two = (bool)two_raw;
    uint64_t three_mask = (1ULL << 3) - 1ULL;
    uint64_t three_raw = (data >> 58) & three_mask;
    bms_test_message_two->three = (uint8_t)three_raw;
    uint64_t four_mask = (1ULL << 6) - 1ULL;
    uint64_t four_raw = (data >> 52) & four_mask;
    bms_test_message_two->four = (uint8_t)four_raw;
    uint64_t five_mask = (1ULL << 1) - 1ULL;
    uint64_t five_raw = (data >> 51) & five_mask;
    bms_test_message_two->five = (bool)five_raw;
    uint64_t six_mask = (1ULL << 1) - 1ULL;
    uint64_t six_raw = (data >> 50) & six_mask;
    bms_test_message_two->six = (bool)six_raw;
    uint64_t seven_mask = (1ULL << 1) - 1ULL;
    uint64_t seven_raw = (data >> 49) & seven_mask;
    bms_test_message_two->seven = (bool)seven_raw;
    uint64_t eight_mask = (1ULL << 1) - 1ULL;
    uint64_t eight_raw = (data >> 48) & eight_mask;
    bms_test_message_two->eight = (bool)eight_raw;
    uint64_t nine_mask = (1ULL << 23) - 1ULL;
    uint64_t nine_raw = (data >> 25) & nine_mask;
    bms_test_message_two->nine = (uint32_t)nine_raw;
    uint64_t ten_mask = (1ULL << 9) - 1ULL;
    uint64_t ten_raw = (data >> 16) & ten_mask;
    bms_test_message_two->ten = (uint16_t)ten_raw;
}

