#ifndef __AHRS_H
#define __AHRS_H

#include "stdbool.h"
#include "stm32h7xx_hal.h"
#include "vector3f.h"

void ahrs_update(double fWxyz[3], double fAxyz[3],double fHeadAng, bool bDcmFastGroundGains, double fDcmAng[3], double fDcmWxyz[3]);

void update_cd_values(Vector3f dcmAng,Vector3f dcmWxyz,Vector3i *attiAngDegX100, Vector3f *attiRateDegX100);

Vector3i getAttiAngRad2DegX100(double fDcmAng[3]);
Vector3f getAttiRateRad2DegX100(double fDcmWxyz[3]);

//extern Vector3f dcm_accel_ef;
double fGetDcmAccelEf(uint8_t iCh);
#endif


