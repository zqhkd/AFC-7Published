#include "AFCGlobalVar.h"
#include "ahrs.h"
#include "ICM42688.h"
#include "matrix3f.h"
#include "Sensor.h"
#include "fc_math.h"
#include "IST8310.h"

#define _kp      0.14f
#define _kp_yaw  0.16f
#define _ki      0.0087f
#define _ki_yaw  0.001f

float dcm_roll = 0.0f;
float dcm_pitch = 0.0f;
float dcm_yaw = 0.0f;

//float cos_roll, cos_pitch, cos_yaw;
//float sin_roll, sin_pitch, sin_yaw;

Vector3f dcm_accel_ef;

Vector3f _omega_P;         //加速度计P比例校正
Vector3f _omega_yaw_P;      //偏航 P比例校正
Vector3f _omega_I;       //积分校正
Vector3f _omega;         //校准陀螺仪向量
Matrix3f _dcm_matrix;    //旋转矩阵
Vector3f _trim;

void matrix_update(Vector3f Wxyz);
void reset(uint8_t recover_eulers);
uint8_t renorm(Vector3f a, Vector3f *result);
void normalize(void);
void check_matrix(void);
void drift_correction(Vector3f Axyz,float fHeadAng, bool bDcmFastGroundGains);
Vector3f euler_angles(void);

// 姿态角解算采用老程序，算法所用机体坐标系估计是后X、右Y、上Z
void ahrs_update(double fWxyz[3], double fAxyz[3],double fHeadAng, bool bDcmFastGroundGains, double fDcmAng[3], double fDcmWxyz[3])
{
	Vector3f Wxyz,Axyz, dcmAng, dcmWxyz;
	Wxyz.x = fWxyz[0];		Wxyz.y = fWxyz[1];	Wxyz.z = fWxyz[2];
	Axyz.x = fAxyz[0];		Axyz.y = fAxyz[1];	Axyz.z = fAxyz[2];      // V5.05.240817: 经与宵宇、陈凯沟通，认为MEMS惯组测得的加速度应该是除重力以外的力。
	                                                                  //          以Az为例, 无人机静止桌面,它受到向下的重力和桌面向上的支撑力，两个力大小相等方向相反，
	                                                                  //          按前右下规则，Az正向向下，因此此时Az为负g。 
	
////	V5.03.240622：按前、右、下规则，ahrs_update解算时输入参数对Ax、Ay、Az取反，其它不用变 
//	Wxyz.x =  fWxyz[0];		Wxyz.y =  fWxyz[1];	Wxyz.z =  fWxyz[2];
//	Axyz.x =  -fAxyz[0];		Axyz.y =  -fAxyz[1];	Axyz.z =  -fAxyz[2];  // V5.05取消乘以-1

	matrix_update(Wxyz);
	normalize();
	drift_correction(Axyz, fHeadAng,bDcmFastGroundGains);
	check_matrix();
	dcmAng = euler_angles();
	dcmWxyz = _omega;
	
	fDcmAng[0]  =  dcmAng.x;   fDcmAng[1]  = dcmAng.y;    fDcmAng[2]  = dcmAng.z;    
	fDcmWxyz[0] = dcmWxyz.x;   fDcmWxyz[1] = dcmWxyz.y;   fDcmWxyz[2] = dcmWxyz.z;   
	
//	// V5.04.240622: 按照前X、左Y、上Z规则，对输出结果的pitch和head、及输出的wy、Wz进行了取反操作。
//	// 以下取反,再在simulink外部再取反时，程序不能正常飞行。V5.04.240623反复测试均不能解决
//	fDcmAng[0]  =  dcmAng.x;   fDcmAng[1]  = -dcmAng.y;    fDcmAng[2]  = -dcmAng.z;    
//	fDcmWxyz[0] = dcmWxyz.x;   fDcmWxyz[1] = -dcmWxyz.y;   fDcmWxyz[2] = -dcmWxyz.z;    
}

void matrix_update(Vector3f Wxyz)
{
	Vector3f r;

	Vector3f_Zero(&_omega);

	_omega = Wxyz;
	_omega = Vector3f_Add(_omega, _omega_I);
	r = Vector3f_Add(_omega, _omega_P);
	r = Vector3f_Add(r, _omega_yaw_P);
	r = Vector3f_Scale(r, g_iSimulinkAlgorithmStep/1000.f);

	Matrix3f_rotate(&_dcm_matrix, r);
}

void reset(uint8_t recover_eulers)
{
	Vector3f_Zero(&_omega_I);
	Vector3f_Zero(&_omega_P);
	Vector3f_Zero(&_omega_yaw_P);
	Vector3f_Zero(&_omega);

	if(recover_eulers && !isnan(dcm_roll) && !isnan(dcm_pitch) && !isnan(dcm_yaw)) {
		Matrix3f_from_euler(&_dcm_matrix, dcm_roll, dcm_pitch, dcm_yaw);
  } else {
		Matrix3f_from_euler(&_dcm_matrix, 0,0,0);
  }
}

uint8_t renorm(Vector3f a, Vector3f *result)
{
	float renorm_val,a_length;

	a_length = Vector3f_Length(a);
	if(a_length==0.0f) {
        return 0;
	}
	renorm_val = 1.0f / a_length;

	if(!(renorm_val < 2.0f && renorm_val > 0.5f)) {
     if(!(renorm_val < 1.0e6f && renorm_val > 1.0e-6f)) {
        return 0;
     }
    }

	*result = Vector3f_Scale(a, renorm_val);
	return 1;
}

void normalize(void)
{
	float error;
  Vector3f t0, t1, t2;

	error = Vector3f_Multi(_dcm_matrix.a, _dcm_matrix.b);               // eq.18

	t0 = Vector3f_Sub(_dcm_matrix.a, Vector3f_Scale(_dcm_matrix.b, 0.5f * error));
	t1 = Vector3f_Sub(_dcm_matrix.b, Vector3f_Scale(_dcm_matrix.a, 0.5f * error));
	t2 = Vector3f_Cross(t0, t1);

	if(!renorm(t0, &_dcm_matrix.a) ||
     !renorm(t1, &_dcm_matrix.b) ||
     !renorm(t2, &_dcm_matrix.c)) {
		 reset(1);
    }
}

void check_matrix(void)
{
	if(Matrix3f_is_nan(&_dcm_matrix)) {
		reset(1);
		return;
	}

	if(!(_dcm_matrix.c.x < 1.0f && _dcm_matrix.c.x > -1.0f)) {
        normalize();

		if(fabsf(_dcm_matrix.c.x) > 10) reset(1);
    }
}

float yaw_error_compass(float fHeadAng,float dcmYaw)
{
	float yaw_error;
	yaw_error = ToRad(fHeadAng) - dcmYaw;
	yaw_error = sinf(wrap_PI(yaw_error));
	
	return yaw_error;
}

float _P_gain(float spin_rate)
{
	if (spin_rate < ToRad(50)) return 1.0f;
	if (spin_rate > ToRad(500)) return 10.0f;
	return spin_rate/ToRad(50);
}

float _yaw_gain(void)
{
	float VdotEFmag = pythagorous2(dcm_accel_ef.x, dcm_accel_ef.y);
	if (VdotEFmag <= 4.0f) {
			return 0.2f*(4.5f - VdotEFmag);
	}
	return 0.1f;
}

Vector3f _ra_delay_buffer;
Vector3f ra_delayed(Vector3f ra)
{
	Vector3f ret = _ra_delay_buffer;
	_ra_delay_buffer = ra;
	if(ret.x == 0 && ret.y == 0&& ret.z == 0) {
		return ra;
	}
	return ret;
}

void drift_correction(Vector3f Axyz,float fHeadAng, bool bDcmFastGroundGains)
{
//	static uint8_t error_ki_count = 0;
	static Vector3f _ra_sum={0,0,0};
	static Vector3f _omega_I_sum={0,0,0};
	static float _omega_I_sum_time=0;

	static bool have_yaw_init = false;
	static uint8_t yaw_correction_count = 0;
	
	float deltat = g_iSimulinkAlgorithmStep/1000.f;

	Vector3f error;
	float drift_limit;
	float error_z, spin_rate;

	Vector3f GA_e = {0.0f, 0.0f, -1.0f};
	float ra_scale;
  Vector3f GA_b;

	if(yaw_correction_count<100) {
		yaw_correction_count++;
		_omega_yaw_P = Vector3f_Scale(_omega_yaw_P, 0.97f);
		return;
	}

	if(!have_yaw_init) {
		Matrix3f_from_euler(&_dcm_matrix, dcm_roll, dcm_pitch, fHeadAng);
		have_yaw_init = true;
	} else {
		error_z = _dcm_matrix.c.z * yaw_error_compass(fHeadAng,dcm_yaw);
		spin_rate = Vector3f_Length(_omega);
		_omega_yaw_P.z = error_z * _P_gain(spin_rate) * _kp_yaw * _yaw_gain();

		_omega_I.z += error_z*_ki_yaw*deltat;
	}

	dcm_accel_ef = Matrix3f_Muli_Vector3f(_dcm_matrix, Axyz);
	_ra_sum = Vector3f_Add(_ra_sum, Vector3f_Scale(dcm_accel_ef, deltat));

	ra_scale = 1.0f/(deltat*GRAVITY_MSS);

	_ra_sum = Vector3f_Scale(_ra_sum, ra_scale);

	GA_b = _ra_sum;

	if(!(GA_b.x == 0.0f && GA_b.y == 0.0f && GA_b.z == 0.0f)) {
		GA_b = Vector3f_Scale(GA_b, 1/Vector3f_Length(GA_b));
		error = Vector3f_Cross(GA_b, GA_e);
	}

	error.z *= sinf(fabsf(dcm_roll));

	error = Matrix3f_mul_transpose(_dcm_matrix, error);

	spin_rate = Vector3f_Length(_omega);

	_omega_P = Vector3f_Scale(error, _P_gain(spin_rate)*_kp);
	if(bDcmFastGroundGains) {
		_omega_P = Vector3f_Scale(_omega_P, 8);
	}

	if(spin_rate < ToRad(20)) {
		_omega_I_sum = Vector3f_Add(_omega_I_sum, Vector3f_Scale(error, _ki*deltat));
		_omega_I_sum_time += deltat;
	}

	if(_omega_I_sum_time >= 0.5f) {
		drift_limit = ToRad(0.5/60) * _omega_I_sum_time;
		_omega_I_sum.x = constrain_float(_omega_I_sum.x, -drift_limit, drift_limit);
		_omega_I_sum.y = constrain_float(_omega_I_sum.y, -drift_limit, drift_limit);
		_omega_I_sum.z = constrain_float(_omega_I_sum.z, -drift_limit, drift_limit);
		_omega_I = Vector3f_Add(_omega_I, _omega_I_sum);

		Vector3f_Zero(&_omega_I_sum);
		_omega_I_sum_time = 0;
	}

	_ra_sum.x = _ra_sum.y = _ra_sum.z = 0;
}

Vector3f euler_angles(void)
{
	Vector3f dcmAng;
	Matrix3f_to_euler(_dcm_matrix, &dcm_roll, &dcm_pitch, &dcm_yaw);
	dcmAng.x = dcm_roll;  dcmAng.y = dcm_pitch;  dcmAng.z = dcm_yaw;
	return dcmAng;
}

// attiAngDegX100疑似后续应更改为Vector3f，没必要用int32_t类型
void update_cd_values(Vector3f dcmAng,Vector3f dcmWxyz,Vector3i *attiAngDegX100, Vector3f *attiRateDegX100)
{
	attiAngDegX100->x= dcmAng.x*RAD_TO_DEG_X100;
	attiAngDegX100->y = dcmAng.y*RAD_TO_DEG_X100;
	attiAngDegX100->z = dcmAng.z*RAD_TO_DEG_X100;
	if(attiAngDegX100->z < 0)		attiAngDegX100->z += 36000;

	attiRateDegX100->x = dcmWxyz.x*RAD_TO_DEG_X100;
	attiRateDegX100->y = dcmWxyz.y*RAD_TO_DEG_X100;
	attiRateDegX100->z = dcmWxyz.z*RAD_TO_DEG_X100;
}

double fGetDcmAccelEf(uint8_t iCh)
{
	float val;
	switch(iCh){
		case 0:
			val = dcm_accel_ef.x; break;
		case 1:
			val = dcm_accel_ef.y; break;
		case 2:
			val = dcm_accel_ef.z; break;
	}
	return val;
}


Vector3i getAttiAngRad2DegX100(double fDcmAng[3])
{
	Vector3i attiAngDegX100;    // attiAngDegX100疑似后续应更改为Vector3f，没必要用int32_t类型
	attiAngDegX100.x= fDcmAng[0]*RAD_TO_DEG_X100;
	attiAngDegX100.y = fDcmAng[1]*RAD_TO_DEG_X100;
	attiAngDegX100.z = fDcmAng[2]*RAD_TO_DEG_X100;
	if(attiAngDegX100.z < 0)		attiAngDegX100.z += 36000;
	return attiAngDegX100;
}

Vector3f getAttiRateRad2DegX100(double fDcmWxyz[3])
{
	Vector3f attiRateDegX100;
	attiRateDegX100.x = fDcmWxyz[0]*RAD_TO_DEG_X100;
	attiRateDegX100.y = fDcmWxyz[1]*RAD_TO_DEG_X100;
	attiRateDegX100.z = fDcmWxyz[2]*RAD_TO_DEG_X100;
	
	return attiRateDegX100;
}

/*
void update_trig(void)
{
	float cos_roll, cos_pitch, cos_yaw;
	float sin_roll, sin_pitch, sin_yaw;
	
	float yaw_vector_x, yaw_vector_y;
	float length;

	yaw_vector_x = _dcm_matrix.a.x;
  yaw_vector_y = _dcm_matrix.b.x;
	length = sqrt(yaw_vector_x*yaw_vector_x+yaw_vector_y*yaw_vector_y);

	yaw_vector_x /= length;
	yaw_vector_y /= length;

	sin_yaw = constrain_float(yaw_vector_y, -1.0, 1.0);
  cos_yaw = constrain_float(yaw_vector_x, -1.0, 1.0);

	cos_pitch = sqrt(1 - (_dcm_matrix.c.x * _dcm_matrix.c.x));
  cos_roll = _dcm_matrix.c.z / cos_pitch;
  cos_pitch = constrain_float(cos_pitch, 0, 1.0);
  cos_roll = constrain_float(cos_roll, -1.0, 1.0);

  sin_pitch = -_dcm_matrix.c.x;
  sin_roll = _dcm_matrix.c.y / cos_pitch;
}
*/
