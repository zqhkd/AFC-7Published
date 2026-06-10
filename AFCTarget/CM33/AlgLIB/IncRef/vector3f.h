#ifndef __VECTOR3F_H
#define __VECTOR3F_H

#include "stm32h7xx_hal.h"
#include "AFCGlobalDef.h"

Vector3f Vector3f_Sub(Vector3f v1, const Vector3f v2); 
Vector3f Vector3f_Scale(Vector3f v1,const float v2);
Vector3f Vector3f_Add(Vector3f v1,const Vector3f v2);
void Vector3f_Zero(Vector3f *v);
float Vector3f_Length(const Vector3f v1);
float Vector3f_Multi(const Vector3f v1,const Vector3f v2);
Vector3f Vector3f_Cross(const Vector3f v1,const Vector3f v2);
unsigned char Vector3f_is_nan(Vector3f *v);

#endif


