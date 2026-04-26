#include "AfcControlLogic.h"

static float current_thrust = 0.0f;

// 模拟 Simulink 中的 Step 函数
void AfcControlLogic_Step(void) {
    // 简单的线性增加逻辑，模拟起飞爬升过程
    if (current_thrust < 80.0f) {
        current_thrust += 2.5f;
    }
}

float AfcControlLogic_GetOutput(void) {
    return current_thrust;
}