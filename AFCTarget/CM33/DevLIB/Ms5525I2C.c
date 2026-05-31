/******************************************************/
/*                                                    */
/*            	2024,mibuwu,test file                  */
/*                                                    */
/******************************************************/
/******************************************************
//     @Copyright      :
//     @FileName       :  
//     @Author         :  mi bu wu
//     @Version        :  1.0.0
//     @CreateDate     :  2024
********************************************************/

#include "MS5525I2C.h" 
#define MS4525_SCL_Pin GPIO_PIN_8
#define MS4525_SCL_GPIO_Port GPIOB
#define MS4525_SDA_Pin GPIO_PIN_9
#define MS4525_SDA_GPIO_Port GPIOB

#define  MS5525_I2C_ADDRESS   					   0x76
#define  MS5525_CMD_ADC_READ               0x00 // ADC read command
#define  MS5525_CMD_PROM_START             0xA0 // Prom read command (first)

#define  MS5525_CMD_CONVERT_PRES 0x44
#define  MS5525_CMD_CONVERT_TEMP 0x54

void ms5525_delay_us(uint16_t nus)
{
	uint32_t Delay = nus * 168/4;
 do
 {
		__NOP();
 }
 while (Delay --); 
}
//将SDA引脚设为输入（IO口的设置）
void ms5525_iic_sda_in(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; 
  __HAL_RCC_GPIOC_CLK_ENABLE();   // 打开GPIOA时钟
  GPIO_InitStruct.Pin = MS4525_SDA_Pin;    // 设置GPIO引脚5
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;   //设置为输入模式 
  GPIO_InitStruct.Pull = GPIO_PULLUP;           // 不使用上下拉电阻
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // 设置GPIO速度为低速
  HAL_GPIO_Init(MS4525_SDA_GPIO_Port, &GPIO_InitStruct);      // 初始化GPIOA 
}
// 将SDA引脚设为输出（IO口的设置）
void ms5525_iic_sda_out(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();   // 打开GPIOA时钟
  GPIO_InitStruct.Pin = MS4525_SDA_Pin;    // 设置GPIO引脚5
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 设置为推挽输出模式  GPIO_MODE_OUTPUT_OD  GPIO_MODE_OUTPUT_PP
  GPIO_InitStruct.Pull = GPIO_PULLUP;           // 不使用上下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // 设置GPIO速度为低速
  HAL_GPIO_Init(MS4525_SDA_GPIO_Port, &GPIO_InitStruct);      // 初始化GPIOA 
}
//读取sda引脚的电平状态
uint8_t ms5525_read_sda(void)
{ 
	return HAL_GPIO_ReadPin(MS4525_SDA_GPIO_Port, MS4525_SDA_Pin);
}
//设置scl引脚的输出电平，0或1
void ms5525_iic_scl_bit_set(uint16_t bit)
{ 
	 HAL_GPIO_WritePin(  MS4525_SCL_GPIO_Port, MS4525_SCL_Pin, (GPIO_PinState)bit);
}
//设置sda引脚的输出电平，0或1
void ms5525_iic_sda_bit_set(uint16_t bit)
{ 
	 HAL_GPIO_WritePin(  MS4525_SDA_GPIO_Port, MS4525_SDA_Pin, (GPIO_PinState)bit);
}
//IIC的IO口初始化
void ms5525_iic_init(void) //******************
{
	GPIO_InitTypeDef GPIO_InitStruct = {0}; 
	
  __HAL_RCC_GPIOA_CLK_ENABLE();   //使能MS4525_SDA_GPIO_Port时钟 
	
  GPIO_InitStruct.Pin = MS4525_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(MS4525_SDA_GPIO_Port, &GPIO_InitStruct);
	
	__HAL_RCC_GPIOC_CLK_ENABLE();   //使能MS4525_SCL_GPIO_Port时钟 
	GPIO_InitStruct.Pin = MS4525_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(MS4525_SCL_GPIO_Port, &GPIO_InitStruct);
    
  ms5525_iic_sda_bit_set(1);
  ms5525_iic_scl_bit_set(1);  
}

//产生IIC起始信号
void ms5525_iic_start(void)
{
	ms5525_iic_sda_out();     //sda线输出
	ms5525_iic_sda_bit_set(1);	  	  
	ms5525_iic_scl_bit_set(1);
	ms5525_delay_us(2);
 	ms5525_iic_sda_bit_set(0);//START:when CLK is high,DATA change form high to low 
	ms5525_delay_us(2);
	ms5525_iic_scl_bit_set(0);//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void ms5525_iic_stop(void)
{
	ms5525_iic_sda_out();//sda线输出
	ms5525_iic_scl_bit_set(0);
	ms5525_iic_sda_bit_set(0);//STOP:when CLK is high DATA change form low to high
 	ms5525_delay_us(4);
	ms5525_iic_scl_bit_set(1); 
	ms5525_iic_sda_bit_set(1);//发送I2C总线结束信号
	ms5525_delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
unsigned char ms5525_iic_wait_ack(void)
{
	unsigned char ucErrTime=0;
	ms5525_iic_sda_in();      //SDA设置为输入  
	ms5525_iic_sda_bit_set(1);ms5525_delay_us(1);	   
	ms5525_iic_scl_bit_set(1);ms5525_delay_us(1);	 
	while(ms5525_read_sda())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			ms5525_iic_stop();
			return 1;
		}
	}
	ms5525_iic_scl_bit_set(0);//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void ms5525_iic_ack(void)
{
	ms5525_iic_scl_bit_set(0);
	ms5525_iic_sda_out();
	ms5525_iic_sda_bit_set(0);
	ms5525_delay_us(2);
	ms5525_iic_scl_bit_set(1);
	ms5525_delay_us(2);
	ms5525_iic_scl_bit_set(0);
}
//不产生ACK应答		    
void ms5525_iic_nack(void)
{
	ms5525_iic_scl_bit_set(0);
	ms5525_iic_sda_out();
	ms5525_iic_sda_bit_set(1);
	ms5525_delay_us(2);
	ms5525_iic_scl_bit_set(1);
	ms5525_delay_us(2);
	ms5525_iic_scl_bit_set(0);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void ms5525_iic_send_byte(unsigned char txd)
{                        
    unsigned char t;   
	  ms5525_iic_sda_out(); 	    
    ms5525_iic_scl_bit_set(0);//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {      
				ms5525_delay_us(1); 			
        ms5525_iic_sda_bit_set((txd&0x80)>>7);
        txd<<=1; 	  				  
				ms5525_iic_scl_bit_set(1);
				ms5525_delay_us(1); 
				ms5525_iic_scl_bit_set(0);	
			//	ms5525_delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
unsigned char ms5525_iic_read_byte(unsigned char ack)
{
	unsigned char i,receive=0;
	ms5525_iic_sda_in();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
		ms5525_iic_scl_bit_set(0); 
		ms5525_delay_us(2);
		ms5525_iic_scl_bit_set(1);
        receive<<=1;
        if(ms5525_read_sda())receive++;   
		ms5525_delay_us(1); 
    }					 
    if (!ack)
        ms5525_iic_nack();//发送nACK
    else
        ms5525_iic_ack(); //发送ACK   
    return receive;
}
//读取n个字节
void ms5525_iic_read_nbyte(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	ms5525_iic_start();
	ms5525_iic_send_byte(addr<<1);
	if(ms5525_iic_wait_ack())
	{
		ms5525_iic_stop();
		return;
	}
	ms5525_iic_send_byte(reg);
	ms5525_iic_wait_ack();

	ms5525_iic_start();
	ms5525_iic_send_byte(addr<<1 | 0x01);
	ms5525_iic_wait_ack();
	while(len)
	{
		if(len == 1)
		{
			*buf = ms5525_iic_read_byte(0);
		}
		else
		{
			*buf = ms5525_iic_read_byte(1);
		}
		buf++;
		len--;
	}
	ms5525_iic_stop();
}

//在移植过程中可以按实际情况修改延时函数
void ms5525_delay_ms(uint16_t t)
{
	  HAL_Delay(t);
}

//MS5525读取n个字节
void MS5525_read_nBYTE( uint8_t len, uint8_t *buf)//******************
{  
	ms5525_iic_start();
	ms5525_iic_send_byte(MS5525_I2C_ADDRESS<<1 | 0x01);
  if(ms5525_iic_wait_ack())
	{
//		my_printf("in MS5525_read_nBYTE process , after send addr , Wait_Ack failed \n");
		ms5525_iic_stop();
		return;
	}
	while(len)
	{
		if(len == 1)
		{
			*buf = ms5525_iic_read_byte(0);
		}
		else
		{
			*buf = ms5525_iic_read_byte(1);
		}
		buf++;
		len--;
	}
	ms5525_iic_stop();
}


//MS5525的单个字节写入函数
void MS5525_write_byte(uint8_t data )//******************
{
	ms5525_iic_start();
	ms5525_iic_send_byte( MS5525_I2C_ADDRESS << 1 );
  if(ms5525_iic_wait_ack())
	{
	//	my_printf("in MS5525_write_byte process , after send addr , Wait_Ack failed \n");
		ms5525_iic_stop();
		return;
	}
    ms5525_iic_send_byte(data);
  if(ms5525_iic_wait_ack())
	{
	//	my_printf("in MS5525_write_byte process , after send CMD , Wait_Ack failed \n");
		ms5525_iic_stop();
		return;
	}
}

//芯片手册在该部分主要用于CRC校验，来源于PX4
static unsigned int prom_crc4(uint16_t n_prom[])//******************
 { 
	 unsigned int n_rem = 0x00; 
	 unsigned int crc_read = n_prom[7]; // save read CRC
	 n_prom[7] = (0xFF00 & (n_prom[7])); // CRC byte is replaced by 0 
	 for (int cnt = 0; cnt < 16; cnt++) {
			 // choose LSB or MSB
			 if (cnt % 2 == 1) {
					 n_rem ^= (unsigned short)((n_prom[cnt >> 1]) & 0x00FF);

			 } else {
					 n_rem ^= (unsigned short)(n_prom[cnt >> 1] >> 8);
			 }

			 for (uint8_t n_bit = 8; n_bit > 0; n_bit--) {
					 if (n_rem & (0x8000)) {
							 n_rem = (n_rem << 1) ^ 0x3000;

					 } else {
							 n_rem = (n_rem << 1);
					 }
			 }
	 }
	 n_rem = (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
	 n_prom[7] = crc_read; // restore the crc_read to its original place
	 return (n_rem ^ 0x00);
 } 

uint16_t C1={0};
uint16_t C2={0};
uint16_t C3={0};
uint16_t C4={0};
uint16_t C5={0};
uint16_t C6={0};
// Qx Coefficients Matrix by Pressure Range
//  5525DSO-pp001DS (Pmin = -1, Pmax = 1)
static  uint8_t Q1 = 15; //15  17
static  uint8_t Q2 = 17; //17  19
static  uint8_t Q3 = 7;  //7   5
static  uint8_t Q4 = 5;  //5   3
static  uint8_t Q5 = 7;
static  uint8_t Q6 = 21; //21  22
int64_t Tref={0};
uint8_t  devtype =0;
// last readings for D1 (uncompensated pressure) and D2 (uncompensated temperature)
uint32_t D1={0};
uint32_t D2={0};
 
 //芯片手册中该芯片没有初始化的要求，可以直接读取数据。直接调用该函数
 //所读得的原始数据数据包含三种：出厂自带的压差和温度的校准数据、压差、温度。
 //原始数据需转换，压差需温度补偿，本段代码已对照芯片手册和PX4相关源码，互相验证过，代码正确。
 //返回温补后的压差（温度数据存于全局变量 ms5525_temperature）
//#define DELAY_USE_us
#ifdef DELAY_USE_us
void MS5525_get_diff_pressure_temp( float *dp_out ,  float *temp_out )//******************
{
	float diff_press = 0;
	uint16_t prom[8]={0};
	uint8_t  val1[2]={0};
	uint8_t  cmd = 0 ;
	static uint32_t dT = 0 ;
	uint8_t val[3] = {0};
	uint32_t adc_D1 =0 , adc_D2 = 0 ; 
	
	MS5525_write_byte(MS5525_CMD_RESET);
	
	ms5525_delay_ms(2);
	// Step 1 - read calibration coefficients from prom
	// prom layout
	// 0 factory data and the setup
	// 1-6 calibration coefficients
	// 7 serial code and CRC 
	//参考芯片手册内容这是第一步得到C1~C6 即prom[0~6] 即出厂时写入芯片的各种校准系数
	for (uint8_t i = 0; i < 8; i++) {
			cmd = MS5525_CMD_PROM_START + i * 2;
			MS5525_write_byte(cmd); ms5525_delay_us(100); 
			MS5525_read_nBYTE(2,&val1[0]);  ms5525_delay_us(100); 
		 // rt_kprintf("%x %x %x \n",val1[0],val1[1],val1[2]);
		 prom[i] = (val1[0] << 8) | val1[1]; //高八位，低八位，然后合为一个uint16_t数值的得到6个系数C1~C6
	} 
		 // Step 2 - check CRC 进行冗余校验，自己算出的冗余校验值和芯片输出的校验值要一样
	 uint8_t crc = prom_crc4(prom);
	 uint8_t onboard_crc = prom[7] & 0xF;//prom前6个是系数，后两个存放芯片输出的校验值
	 if (crc == onboard_crc) {
			 // store valid calibration coefficients
			 C1 = prom[1];
			 C2 = prom[2];
			 C3 = prom[3];
			 C4 = prom[4];
			 C5 = prom[5];
			 C6 = prom[6];

			 Tref = (int64_t)(C5) * (1UL << Q5); //1ul表示1是无符号长整型常量ulong(long)
	 }
//	 else { 
//			 my_printf("CRC mismatch \n");
//	 }	
	 
	// Step 3 - 参考芯片手册内容这是第二步得到D1,气压值 、D2，温度值。
	MS5525_write_byte(MS5525_CMD_CONVERT_PRES);ms5525_delay_ms(5); //D1,气压值
	// Step 4 - ADC 
	MS5525_write_byte(0x00); ms5525_delay_ms(1); 
	// Step 5 -  read 24 bits from the sensor 
	MS5525_read_nBYTE(3,&val[0]); ms5525_delay_ms(2); 
	// Step 6 -  get adc_D1
	adc_D1 = (val[0] << 16) | (val[1] << 8) | val[2];//D1,气压值
 
	// Step 3 - 参考芯片手册内容这是第二步得到D2，温度值。 
	MS5525_write_byte(MS5525_CMD_CONVERT_TEMP);ms5525_delay_ms(1); //D2，温度值。
	// Step 4 - ADC 
	MS5525_write_byte(0x00); ms5525_delay_ms(1); 
	// Step 5 -  read 24 bits from the sensor 
	MS5525_read_nBYTE(3,&val[0]); ms5525_delay_ms(1); 
	// Step 6 -  get adc_D2
	adc_D2 = (val[0] << 16) | (val[1] << 8) | val[2];//D2，温度值。
	 
	// Step 7 -  get dT
	dT = adc_D2 -  Tref ; //表示实际温度与参考温度之差
	
	// Step 7 -  计算温度  TEMP = 20°C + dT * TEMPSENS 
	const int64_t TEMP = 2000 + (dT * (int64_t)(C6)) / (1UL << Q6);
	// Step 8 -  计算温度补偿后的压差  TEMP = 20°C + dT * TEMPSENS 
		 // Offset at actual temperature
	 //  OFF = OFF_T1 + TCO * dT
	 const int64_t OFF = (int64_t)(C2) * (1UL << Q2) + ((int64_t)(C4) * dT) / (1UL << Q4);

	 // Sensitivity at actual temperature
	 //  SENS = SENS_T1 + TCS * dT
	 const int64_t SENS = (int64_t)(C1) * (1UL << Q1) + ((int64_t)(C3) * dT) / (1UL << Q3);

	 // Temperature Compensated Pressure (example 24996 = 2.4996 psi)
	 //  P = D1 * SENS - OFF
	 const int64_t P = (adc_D1 * SENS / (1UL << 21) - OFF) / (1UL << 15);

	 const float diff_press_PSI = P * 0.0001f;

	 // 1 PSI = 6894.76 Pascals
	 static  float PSI_to_Pa = 6894.757f;

//	 diff_press0 = diff_press_PSI * PSI_to_Pa;
//	 temp0 = TEMP * 0.01f;
	 
	 diff_press = diff_press_PSI * PSI_to_Pa;
   *dp_out = diff_press;

	 *temp_out = TEMP * 0.01f; 
}
#else
// 延时修改前版本
void MS5525_get_diff_pressure_temp( float *dp_out ,  float *temp_out )//******************
{
	float diff_press = 0;
	uint16_t prom[8]={0};
	uint8_t  val1[2]={0};
	uint8_t  cmd = 0 ;
	static uint32_t dT = 0 ;
	uint8_t val[3] = {0};
	uint32_t adc_D1 =0 , adc_D2 = 0 ; 
	
	MS5525_write_byte(MS5525_CMD_RESET);ms5525_delay_ms(3);
	// Step 1 - read calibration coefficients from prom
	// prom layout
	// 0 factory data and the setup
	// 1-6 calibration coefficients
	// 7 serial code and CRC 
	//参考芯片手册内容这是第一步得到C1~C6 即prom[0~6] 即出厂时写入芯片的各种校准系数
	for (uint8_t i = 0; i < 8; i++) {
			cmd = MS5525_CMD_PROM_START + i * 2;
			MS5525_write_byte(cmd); ms5525_delay_ms(1); 
			MS5525_read_nBYTE(2,&val1[0]);  ms5525_delay_ms(1); 
		 // rt_kprintf("%x %x %x \n",val1[0],val1[1],val1[2]);
		 prom[i] = (val1[0] << 8) | val1[1]; //高八位，低八位，然后合为一个uint16_t数值的得到6个系数C1~C6
	} 
		 // Step 2 - check CRC 进行冗余校验，自己算出的冗余校验值和芯片输出的校验值要一样
	 uint8_t crc = prom_crc4(prom);
	 uint8_t onboard_crc = prom[7] & 0xF;//prom前6个是系数，后两个存放芯片输出的校验值
	 if (crc == onboard_crc) {
			 // store valid calibration coefficients
			 C1 = prom[1];
			 C2 = prom[2];
			 C3 = prom[3];
			 C4 = prom[4];
			 C5 = prom[5];
			 C6 = prom[6];

			 Tref = (int64_t)(C5) * (1UL << Q5); //1ul表示1是无符号长整型常量ulong(long)
	 }
//	 else { 
//			 my_printf("CRC mismatch \n");
//	 }	
	 
	// Step 3 - 参考芯片手册内容这是第二步得到D1,气压值 、D2，温度值。
	MS5525_write_byte(MS5525_CMD_CONVERT_PRES);ms5525_delay_ms(2); //D1,气压值
	// Step 4 - ADC 
	MS5525_write_byte(0x00); ms5525_delay_ms(2); 
	// Step 5 -  read 24 bits from the sensor 
	MS5525_read_nBYTE(3,&val[0]); ms5525_delay_ms(2); 
	// Step 6 -  get adc_D1
	adc_D1 = (val[0] << 16) | (val[1] << 8) | val[2];//D1,气压值
 
	// Step 3 - 参考芯片手册内容这是第二步得到D2，温度值。 
	MS5525_write_byte(MS5525_CMD_CONVERT_TEMP);ms5525_delay_ms(2); //D2，温度值。
	// Step 4 - ADC 
	MS5525_write_byte(0x00); ms5525_delay_ms(2); 
	// Step 5 -  read 24 bits from the sensor 
	MS5525_read_nBYTE(3,&val[0]); ms5525_delay_ms(2); 
	// Step 6 -  get adc_D2
	adc_D2 = (val[0] << 16) | (val[1] << 8) | val[2];//D2，温度值。
	 
	// Step 7 -  get dT
	dT = adc_D2 -  Tref ; //表示实际温度与参考温度之差
	
	// Step 7 -  计算温度  TEMP = 20°C + dT * TEMPSENS 
	const int64_t TEMP = 2000 + (dT * (int64_t)(C6)) / (1UL << Q6);
	// Step 8 -  计算温度补偿后的压差  TEMP = 20°C + dT * TEMPSENS 
		 // Offset at actual temperature
	 //  OFF = OFF_T1 + TCO * dT
	 const int64_t OFF = (int64_t)(C2) * (1UL << Q2) + ((int64_t)(C4) * dT) / (1UL << Q4);

	 // Sensitivity at actual temperature
	 //  SENS = SENS_T1 + TCS * dT
	 const int64_t SENS = (int64_t)(C1) * (1UL << Q1) + ((int64_t)(C3) * dT) / (1UL << Q3);

	 // Temperature Compensated Pressure (example 24996 = 2.4996 psi)
	 //  P = D1 * SENS - OFF
	 const int64_t P = (adc_D1 * SENS / (1UL << 21) - OFF) / (1UL << 15);

	 const float diff_press_PSI = P * 0.0001f;

	 // 1 PSI = 6894.76 Pascals
	 static  float PSI_to_Pa = 6894.757f;

//	 diff_press0 = diff_press_PSI * PSI_to_Pa;
//	 temp0 = TEMP * 0.01f;
	 
	 diff_press = diff_press_PSI * PSI_to_Pa;
   *dp_out = diff_press;

	 *temp_out = TEMP * 0.01f; 
}
#endif
