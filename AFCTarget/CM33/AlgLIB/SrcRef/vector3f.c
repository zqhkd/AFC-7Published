#include "vector3f.h"

Vector3f Vector3f_Sub(Vector3f v1,const Vector3f v2)
{
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	return v1;
}

Vector3f Vector3f_Scale(Vector3f v1,const float v2)
{
	v1.x *= v2;
	v1.y *= v2;
	v1.z *= v2;
	return v1;
}

Vector3f Vector3f_Add(Vector3f v1,const Vector3f v2)
{
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

void Vector3f_Zero(Vector3f *v)
{
	v->x = v->y = v->z = 0.0f;
}

float Vector3f_Length(const Vector3f v1)
{
	return sqrtf(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);
}

float Vector3f_Multi(const Vector3f v1,const Vector3f v2)
{
	return (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z);
}

Vector3f Vector3f_Cross(const Vector3f v1,const Vector3f v2)
{
	Vector3f temp;
	temp.x = v1.y*v2.z - v1.z*v2.y;
	temp.y = v1.z*v2.x - v1.x*v2.z;
	temp.z = v1.x*v2.y - v1.y*v2.x;
	return temp;
}

unsigned char Vector3f_is_nan(Vector3f *v)
{
    return isnan(v->x) || isnan(v->y) || isnan(v->z);
}

