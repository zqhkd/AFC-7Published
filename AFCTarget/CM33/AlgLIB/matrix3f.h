#ifndef __MATRIX3F_H
#define __MATRIX3F_H

#include "math.h"
#include "vector3f.h"
#include "AFCGlobalDef.h"

void Matrix3f_rotate(Matrix3f *m, Vector3f g);
void Matrix3f_rotateXYinv(Matrix3f *m, Vector3f g);
void Matrix3f_from_euler(Matrix3f *m, float roll, float pitch, float yaw);
void Matrix3f_to_euler(const Matrix3f m, float *roll, float *pitch, float *yaw);
unsigned char Matrix3f_is_nan(Matrix3f *m);

Vector3f Matrix3f_Muli_Vector3f(Matrix3f m, Vector3f v);
Vector3f Matrix3f_mul_transpose(Matrix3f m, Vector3f v);

#endif


