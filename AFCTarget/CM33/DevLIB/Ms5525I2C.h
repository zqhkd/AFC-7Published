#ifndef __MS5525_I2C_H__
#define __MS5525_I2C_H__
#include "MS5525I2C.h" 
#include "stm32h7xx_hal.h"
#include <stdarg.h>
#include <string.h>
#include "stdio.h"
#define  MS5525_CMD_RESET       0x48  // ADC reset command  48  1E
void ms5525_iic_init(void);                //初始化IIC的IO口				 
void MS5525_write_byte(uint8_t data );
void MS5525_get_diff_pressure_temp( float *dp_out ,  float *temp_out );

#endif
