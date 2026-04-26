#ifndef __AFCCONTROLLOGIC_H__
#define __AFCCONTROLLOGIC_H__

/* * Simulink 自动生成算法接口插槽
 * 模型名称：ZF02FlyDemo
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 算法单步执行函数 (Step Function)
 * 通常在定时任务或 while 循环中以固定频率调用
 */
void AfcControlLogic_Step(void);

/**
 * @brief 获取算法当前的控制输出
 * @return 当前计算出的推力指令
 */
float AfcControlLogic_GetOutput(void);

#ifdef __cplusplus
}
#endif

#endif /* __AFCCONTROLLOGIC_H__ */