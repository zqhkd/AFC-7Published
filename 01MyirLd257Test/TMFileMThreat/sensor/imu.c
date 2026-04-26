/******************** (C) COPYRIGHT 2019 ACE Tech Co.*************************
 * 作  者 ： 曾庆华
 * 文件名 ： imu.c
 * 版  本 ： 
 *          V1.01.260320 
 * 描  述 ：AFC-7问候世界的多文件测试模块
 * 官  网 ：www.acecreator.com
 * 淘  宝 ：acecreator.taobao.com
 * 公众号 ：无人飞行控制
*****************************************************************************/
#include "imu.h"
#include <stdlib.h>

float get_accel_z() {
    // 模拟产生一个 9.8 附近的随机重力加速度
    return 9.8f + ((rand() % 100) / 1000.0f);
}