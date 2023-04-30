#ifndef IMU_H
#define IMU_H

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} imu_data_S;

void imu_init();
void imu_read(imu_data_S *data);

#endif // IMU_H
