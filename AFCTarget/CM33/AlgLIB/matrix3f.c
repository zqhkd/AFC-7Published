#include "matrix3f.h"
#include "fc_math.h"

void Matrix3f_rotate(Matrix3f *m, Vector3f g)
{
	Matrix3f temp_matrix;
  temp_matrix.a.x = m->a.y * g.z - m->a.z * g.y;
  temp_matrix.a.y = m->a.z * g.x - m->a.x * g.z;
  temp_matrix.a.z = m->a.x * g.y - m->a.y * g.x;
  temp_matrix.b.x = m->b.y * g.z - m->b.z * g.y;
  temp_matrix.b.y = m->b.z * g.x - m->b.x * g.z;
  temp_matrix.b.z = m->b.x * g.y - m->b.y * g.x;
  temp_matrix.c.x = m->c.y * g.z - m->c.z * g.y;
  temp_matrix.c.y = m->c.z * g.x - m->c.x * g.z;
  temp_matrix.c.z = m->c.x * g.y - m->c.y * g.x;
	
	m->a = Vector3f_Add(m->a, temp_matrix.a);
	m->b = Vector3f_Add(m->b, temp_matrix.b);
	m->c = Vector3f_Add(m->c, temp_matrix.c);
}

void Matrix3f_rotateXYinv(Matrix3f *m, Vector3f g)
{
  Matrix3f temp_matrix;
  temp_matrix.a.x = - m->a.z * g.y;
  temp_matrix.a.y = m->a.z * g.x;
  temp_matrix.a.z = m->a.x * g.y - m->a.y * g.x;
  temp_matrix.b.x = - m->b.z * g.y;
  temp_matrix.b.y = m->b.z * g.x;
  temp_matrix.b.z = m->b.x * g.y - m->b.y * g.x;
  temp_matrix.c.x = - m->c.z * g.y;
  temp_matrix.c.y = m->c.z * g.x;
  temp_matrix.c.z = m->c.x * g.y - m->c.y * g.x;
	
  m->a = Vector3f_Add(m->a, temp_matrix.a);
	m->b = Vector3f_Add(m->b, temp_matrix.b);
	m->c = Vector3f_Add(m->c, temp_matrix.c);
}

void Matrix3f_from_euler(Matrix3f *m, float roll, float pitch, float yaw)
{
    float cp = cosf(pitch);
    float sp = sinf(pitch);
    float sr = sinf(roll);
    float cr = cosf(roll);
    float sy = sinf(yaw);
    float cy = cosf(yaw);

    m->a.x = cp * cy;
    m->a.y = (sr * sp * cy) - (cr * sy);
    m->a.z = (cr * sp * cy) + (sr * sy);
    m->b.x = cp * sy;
    m->b.y = (sr * sp * sy) + (cr * cy);
    m->b.z = (cr * sp * sy) - (sr * cy);
    m->c.x = -sp;
    m->c.y = sr * cp;
    m->c.z = cr * cp;
}

void Matrix3f_to_euler(const Matrix3f m, float *roll, float *pitch, float *yaw)
{
    if (pitch != 0) {
        *pitch = -safe_asin(m.c.x);
    }
    if (roll != 0) {
        *roll = atan2f(m.c.y, m.c.z);
    }
    if (yaw != 0) {
        *yaw = atan2f(m.b.x, m.a.x);
    }
}

unsigned char Matrix3f_is_nan(Matrix3f *m)
{
	return (Vector3f_is_nan(&m->a) || Vector3f_is_nan(&m->b) || Vector3f_is_nan(&m->c));
}


Vector3f Matrix3f_Muli_Vector3f(Matrix3f m, Vector3f v)
{
	Vector3f result;
	result.x = m.a.x*v.x + m.a.y*v.y + m.a.z*v.z;
	result.y = m.b.x*v.x + m.b.y*v.y + m.b.z*v.z;
	result.z = m.c.x*v.x + m.c.y*v.y + m.c.z*v.z;
	return result;
}

Vector3f Matrix3f_mul_transpose(Matrix3f m, Vector3f v)
{
	Vector3f result;
	result.x = m.a.x*v.x + m.b.x*v.y + m.c.x*v.z;
	result.y = m.a.y*v.x + m.b.y*v.y + m.c.y*v.z;
	result.z = m.a.z*v.x + m.b.z*v.y + m.c.z*v.z;
	return result;
}
