#include "calibration.h"
#include "vector3f.h"
#include "ahrs.h"
#include "Sensor.h"
#include "some_work.h"
#include "FM25V01.h"

uint8_t one_face_cnt = 100;
uint8_t level_cal_count = 100;
Vector3f gyro_addup,accel_addup;
Vector3f gyro_addup2,accel_addup2;

void Start_Level_Cal(void)
{
	Vector3f_Zero(&gyro_addup);
	Vector3f_Zero(&gyro_addup2);
	level_cal_count = 0;
}

void Level_Cal_Update(void)
{
	if(level_cal_count < 50) {
		level_cal_count++;
		
		gyro_addup.x += gyro_raw[0][0];
		gyro_addup.y += gyro_raw[0][1];
		gyro_addup.z += gyro_raw[0][2];
		gyro_addup2.x += gyro_raw[1][0];
		gyro_addup2.y += gyro_raw[1][1];
		gyro_addup2.z += gyro_raw[1][2];

		if(level_cal_count == 50) {
			gyro_offset[0] = Vector3f_Scale(gyro_addup, 0.02f);
			gyro_offset[1] = Vector3f_Scale(gyro_addup2, 0.02f);
		}
	}
	
	if(one_face_cnt < 50) {
		one_face_cnt++;
		
		accel_addup.x += accel_raw[0][0];
		accel_addup.y += accel_raw[0][1];
		accel_addup.z += accel_raw[0][2];
		accel_addup2.x += accel_raw[1][0];
		accel_addup2.y += accel_raw[1][1];
		accel_addup2.z += accel_raw[1][2];

		if(one_face_cnt == 50) {
			accel_offset[0] = Vector3f_Scale(accel_addup,0.02f);
			accel_offset[0].z += 4096;
			accel_offset[1] = Vector3f_Scale(accel_addup2,0.02f);
			accel_offset[1].z += 4096;
			
			flash_store_value[0] = (((uint32_t)((uint16_t)(int16_t)accel_offset[0].x))<<16) | (uint32_t)((uint16_t)(int16_t)accel_offset[0].y);
			flash_store_value[1] = (((uint32_t)((uint16_t)(int16_t)accel_offset[0].z))<<16) | (uint32_t)((uint16_t)(int16_t)accel_offset[1].x);
			flash_store_value[2] = (((uint32_t)((uint16_t)(int16_t)accel_offset[1].y))<<16) | (uint32_t)((uint16_t)(int16_t)accel_offset[1].z);
			
			Save_Parameters();
		}
	}
}

void One_Face_Calibration(void)
{
	Vector3f_Zero(&accel_addup);
	Vector3f_Zero(&accel_addup2);
	one_face_cnt = 0;
}
