#ifndef __ZFA35LIB_H__
#define __ZFA35LIB_H__

/* * AFC-7 A35 底层驱动库接口 (V2.1)
 * 归属：核心团队 (Private Source, Public Header)
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 A35 执行器硬件（PWM 频率、GPIO 模式等）
 * @param frequency PWM 频率 (Hz)
 * @return 0 成功, -1 失败
 */
int ZfA35_Actuator_Init(int frequency);

/**
 * @brief 输出执行器控制指令
 * @param thrust_percent 推力百分比 (0.0 - 100.0)
 */
void ZfA35_Actuator_Output(float thrust_percent);

/* 未来可扩展：传感器读取、CAN通讯等 */
// int ZfA35_Imu_Read(float* gyro, float* acc);

#ifdef __cplusplus
}
#endif

#endif /* __ZFLIBA35_H__ */