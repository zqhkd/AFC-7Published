#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "stdint.h"
#include "stdbool.h"
#include "vector3f.h"

#define GRAVITY_MSS 9.80665f

#define GYRO_SCALE_2000  (0.0174532f / 16.4f)   // V5.02.231010版陀螺仪标度系数，转变为rad/s
#define GYRO_SCALE_1000  (0.0174532f / 32.8f)
#define GYRO_SCALE_500  (0.0174532f / 65.5f)
#define GYRO_SCALE_250  (0.0174532f / 131.0f)

#define ACCEL_SCALE_16G    (GRAVITY_MSS / 2048.0f)
#define ACCEL_SCALE_8G    (GRAVITY_MSS / 4096.0f)    // V5.02.231010版加计标度系数
#define ACCEL_SCALE_4G    (GRAVITY_MSS / 8192.0f)
#define ACCEL_SCALE_2G    (GRAVITY_MSS / 16384.0f)

extern float accel_raw[2][3],gyro_raw[2][3],temp_raw[2];
extern Vector3f accel_lpf[2],gyro_lpf[2];
extern Vector3f accel_scale[2],accel_offset[2],gyro_offset[2];

void Sensor_Accel_Raw_Update(uint8_t id,float x,float y,float z);
void Sensor_Gyro_Raw_Update(uint8_t id,float x,float y,float z,float temp);
void Sensor_LPF(uint8_t id, bool bCacFlg);

#endif



