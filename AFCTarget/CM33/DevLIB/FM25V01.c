#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FM25V01.h"
#include "spiAPI.h"
#include "Sensor.h"
#include "ADSample.h"

#include "ACGCommonAPI.h"
#include "AFCGlobalVar.h"
//#include <stddef.h>

#define SN_ID_KEY  92671

#define BANK0   0x00000
#define BANK1   0x20000
#define BANK2   0x30000
#define BANK01	0x01000
#define BANK02	0x00200
#define BANK03	0x00030

#define		WREN			0x06						// Set Write Enable Latch
#define		WRDI			0x04						// Write Disable
#define		RDSR			0x05						// Read Status Register
#define		WRSR			0x01						// Write Status Register 
#define		READ			0x03						// Read Memory Data.
#define 	FSTRD			0x0B						// Fast Read Memory Data.
#define		WRITE			0x02 						// Write Memory Data
#define		SLEEP			0xB9						// Enter Sleep Mode.
#define		RDID			0x9F						// Read Device ID.
#define 	SNR				0xC3						// Read S/N.

#define __PRODUCT_RELEASE_VERSION__

bool g_bUsedOfFM25V01 = false;

// 获取STM32H743芯片具有一个独特的96位或128位芯片ID，可以通过读取特定的寄存器来获取。
#include "stm32h743xx.h"
#define SysChipCrcLen  22    // 芯片ID.12 + 产品ID.4 + 授权码.4 + CRC校验码.2

// 从芯片ID和产品ID中随意抽取相关字节
uint8_t iGetChidIdIndx(uint8_t iAuthCode, uint8_t *chipId)
{
	 return chipId[iAuthCode];
}

uint8_t iGetChidIdStr(uint32_t iUavId,uint16_t iAuthCode,uint8_t *chipId)
{
 	  uint8_t i;
    // 读取96位芯片ID
    for (i = 0; i < 12; i++) {
        chipId[i] = *(uint8_t *)(0x1FF1E800 + i);
    }
	  
		// 产品授权码(P码)采用小端模式存储，低位在前高位在后
		chipId[i++] = (uint8_t) (iUavId);  chipId[i++] = (uint8_t) (iUavId >> 8); 
		chipId[i++] = (uint8_t) (iUavId >> 16);   chipId[i++] = (uint8_t) (iUavId>> 24); 
		
		// 用户授权码(U码), 2字节，每半字节对应芯片ID位置 + 4位iUavId组成的16字节数组的索引。
		chipId[i++] = iGetChidIdIndx((uint8_t)(iAuthCode & 0x0f),chipId);
		chipId[i++] = iGetChidIdIndx((uint8_t)((iAuthCode>>4) & 0x0f),chipId);
		chipId[i++] = iGetChidIdIndx((uint8_t)((iAuthCode>>8) & 0x0f),chipId);
		chipId[i++] = iGetChidIdIndx((uint8_t)((iAuthCode>>12) & 0x0f),chipId);
		
		return i;
}

// iUavId: 产品授权码（P码）；iAuthCode：用户授权码（U码）；iCrcCode：产品密钥（Key码）。Modified by zqh 20251115
bool bChkChipCrc(uint32_t iUavId,uint16_t iAuthCode,uint16_t iCrcCode)
{
#ifdef __PRODUCT_RELEASE_VERSION__
	  uint8_t i;
		uint8_t chip_id[SysChipCrcLen]; // 96-bit or 128-bit chip ID

	  i = iGetChidIdStr(iUavId,iAuthCode,chip_id);
		chip_id[i++] = (uint8_t) (iCrcCode);  chip_id[i++] = (uint8_t) (iCrcCode >> 8);
    return bChkCRC16(chip_id, i);	
#else
    return true;
#endif	
}

// 检查产品授权信息
bool bChkProductAuth(TProductConfig cfg)
{
	 return bChkChipCrc(AFC_SYSTEM_SN_ID  - SN_ID_KEY ,cfg.UserAuthCode,cfg.KeyCode);
}

void DisableUSART3(void) {
    // 禁用USART3的传输
      USART3->CR1 &= ~(USART_CR1_TE | USART_CR1_RE);
}

uint16_t iGetAFCCrc(uint32_t iUavId,uint16_t iAuthCode)
{
	  uint8_t i;
		uint8_t chip_id[SysChipCrcLen]; // 96-bit or 128-bit chip ID
	  i = iGetChidIdStr(iUavId,iAuthCode,chip_id);
		
	  return CalCRC16(chip_id,i);
}

SSaveParamFlash loadDefaultPara(void)
{
		SSaveParamFlash pSUavDefaultPara;
	
	  pSUavDefaultPara.FirstInitFlg = 0xA5DA;
	
	// UAV总体配置参数
		pSUavDefaultPara.UavPara.UavId = 0;
		pSUavDefaultPara.UavPara.UavFrame = HS620_UAV;
		pSUavDefaultPara.UavPara.FcsBoard = AFC5A_BOARD;
		pSUavDefaultPara.UavPara.XSetup = true;			pSUavDefaultPara.UavPara.YSetup = true;			pSUavDefaultPara.UavPara.ZSetup = false;
	  pSUavDefaultPara.UavPara.ResSetup = 0x0;
//		pSUavDefaultPara.UavPara.Remoter = JUMPER_T20;
		pSUavDefaultPara.UavPara.Remoter = WFLY_ETS6S;
		pSUavDefaultPara.UavPara.FlyMode = ALTITUDE_HOLD_MODE;
	
	  pSUavDefaultPara.UavPara.AuthorizationCode = 0x2CAD;
		#ifdef __PRODUCT_RELEASE_VERSION__
        pSUavDefaultPara.UavPara.CRCCode = 0xA5DA;
		#endif
	
	// UAV飞控板相关传感器标校参数
	  pSUavDefaultPara.Icm42688.AccScale.x  = pSUavDefaultPara.Icm42688.AccScale.y  = pSUavDefaultPara.Icm42688.AccScale.z  = 1;
	  pSUavDefaultPara.Icm42688.AccOffset.x = pSUavDefaultPara.Icm42688.AccOffset.y = pSUavDefaultPara.Icm42688.AccOffset.z = 0;
	  pSUavDefaultPara.Icm42688.GyroScale.x  = pSUavDefaultPara.Icm42688.GyroScale.y  = pSUavDefaultPara.Icm42688.GyroScale.z  = 1;
	  pSUavDefaultPara.Icm42688.GyroOffset.x = pSUavDefaultPara.Icm42688.GyroOffset.y = pSUavDefaultPara.Icm42688.GyroOffset.z = 0;

	  pSUavDefaultPara.Icm20602.AccScale.x  = pSUavDefaultPara.Icm20602.AccScale.y  = pSUavDefaultPara.Icm20602.AccScale.z  = 1;
	  pSUavDefaultPara.Icm20602.AccOffset.x = pSUavDefaultPara.Icm20602.AccOffset.y = pSUavDefaultPara.Icm20602.AccOffset.z = 0;
	  pSUavDefaultPara.Icm20602.GyroScale.x  = pSUavDefaultPara.Icm20602.GyroScale.y  = pSUavDefaultPara.Icm20602.GyroScale.z  = 1;
	  pSUavDefaultPara.Icm20602.GyroOffset.x = pSUavDefaultPara.Icm20602.GyroOffset.y = pSUavDefaultPara.Icm20602.GyroOffset.z = 0;

	  pSUavDefaultPara.Dps310Offset = 0;
	  
		pSUavDefaultPara.Ist8310Bias.x = pSUavDefaultPara.Ist8310Bias.y = pSUavDefaultPara.Ist8310Bias.z = 0;
		pSUavDefaultPara.Ist8310Radius.x = pSUavDefaultPara.Ist8310Radius.y = pSUavDefaultPara.Ist8310Radius.z = 1;
		pSUavDefaultPara.Ist8310Neg = 1;
		
 	  return pSUavDefaultPara;
}

void initRandValue(void)
{
	  unsigned int seeds;
	
    // 这里仅作为示例，您需要根据实际情况实现这个函数
		ADSample_ReadData();
    seeds = (unsigned int)READ_REG(SysTick->VAL ^ ad_vol[0] ^ ad_vol[1]); // SysTick是一个例子
	
    // 使用硬件生成的种子值来初始化随机数生成器
    srand(seeds);
}

// 获取一个(0,1)区间的随机数
float fGetRandVal(void)
{
	  return (float)rand() / (float)RAND_MAX;
}

// V5.05.240811：根据芯片ID等设置其加密操作
void initSysChipIdValid(void)
{
	// 需要根据遥控器Remoter的选择重新初始化usart3, 此操作的原因是MX_USART3_UART_Init启动执行时g_iCurRemoter还未赋值
		 MX_USART3_UART_Init();
	// 允许PWM输出
		 HAL_GPIO_WritePin(PWM_OE_GPIO_Port,PWM_OE_Pin,GPIO_PIN_SET);
	
	   initRandValue();
	// 获取一个(-0.5,0.5)区间的随机数
	   g_SysRndXyz.x = fGetRandVal()-0.5f;  g_SysRndXyz.y = fGetRandVal()-0.5f; g_SysRndXyz.z = fGetRandVal()-0.5f;
	
//		#ifndef __PRODUCT_RELEASE_VERSION__
//				g_UavFcsParam.UavPara.CRCCode = iGetAFCCrc(AFC_SYSTEM_SN_ID  - SN_ID_KEY ,g_UavFcsParam.UavPara.AuthorizationCode);
//     #endif

		 TProductConfig cfgRead;
//		 if(g_bUsedOfFM25V01){
//			  cfgRead.ProductId = g_UavFcsParam.UavPara.UavId;
//			  cfgRead.UserAuthCode = g_UavFcsParam.UavPara.AuthorizationCode;
//			  cfgRead.BoardType = g_UavFcsParam.UavPara.FcsBoard;
//			  cfgRead.KeyCode   = g_UavFcsParam.UavPara.CRCCode;
//		 }
//		 else{
//			 FlashRead(&cfgRead);
//		 }

		 FlashRead(&cfgRead);
	   g_UavFcsParam.UavPara.bAuthorizedFlg = bChkProductAuth(cfgRead);
		 
// 非授权用户的功能严重受限禁止：禁用遥控器操作usart3、禁止PWM输出，只能用于测试，无法控制无人机
//			   DisableUSART3();
//			// 禁止PWM输出
//   			 HAL_GPIO_WritePin(PWM_OE_GPIO_Port,PWM_OE_Pin,GPIO_PIN_RESET);
}
		 
void Flash_Page_Read(uint32_t addr,uint8_t *buf,uint32_t len)
{
	FM25V01_SPI_Read(addr,READ,buf,len);
}

void Flash_Page_Write(uint32_t addr,uint8_t *buf,uint32_t len)
{
	FM25V01_SPI_Write(addr,WRITE,buf,len);
}

extern bool bSPI1IsUsing;
// 暂时用BAK1来做存储
uint8_t fm25v01_id[9] = {0};
// 读取参数正确，则返回true,否则false
bool LoadFromFM25V01(SSaveParamFlash *pBuff)
{
	bSPI1IsUsing = true;   // 防止SPI1上其它设备访问总线，造成数据混乱
	size_t structSize = sizeof(SSaveParamFlash);
	
	SPI1_Read_Long(1,RDID,fm25v01_id,9);  // 读取FM25V01的设备ID信息，应该为：0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xC2, 0x24, 0x00
	const uint8_t FM25_COMMON_MFG_ID[7] = {0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0xC2};	
	g_bUsedOfFM25V01 = (memcmp(fm25v01_id, FM25_COMMON_MFG_ID, 7) == 0);
	
	Flash_Page_Read(BANK1,(uint8_t *)pBuff,structSize);
	
	uint16_t iCrc = CalCRC16((uint8_t *)pBuff,offsetof(SSaveParamFlash, ParaSaveCRC));
	bSPI1IsUsing = false;

	return (pBuff->ParaSaveCRC == iCrc);
}

void SaveToFM25V01(SSaveParamFlash *pBuff)
{
	size_t structSize = sizeof(SSaveParamFlash);
	
	bSPI1IsUsing = true;
	pBuff->ParaSaveCRC = CalCRC16((uint8_t *)pBuff,offsetof(SSaveParamFlash, ParaSaveCRC));
	
	Flash_Page_Write(BANK1,(uint8_t *)pBuff,structSize);
	bSPI1IsUsing = false;
}


uint8_t fm_buf[200];
uint8_t fm_step = 0;
void FM25V01_Test(void)
{
	uint8_t i,buf[200];
	
	if(fm_step==0) {
		SPI1_Read_Long(1,RDID,fm25v01_id,9);
		if(fm25v01_id[7]==0x21&&fm25v01_id[8]==0x08) fm_step++;
	} else if(fm_step==1) {
		fm_step++;
		for(i=0;i<200;i++) buf[i] = i;
		Flash_Page_Write(BANK0,buf,sizeof(buf));
	} else if(fm_step==2) {
		fm_step++;
		Flash_Page_Read(BANK0,fm_buf,sizeof(fm_buf));
	}
}

uint32_t flash_store_value[FLASH_STORE_LEN];
void Load_Parameters(void)
{
	uint8_t buf[FLASH_STORE_LEN*4],i;
	
	SPI1_Read_Long(1,RDID,fm25v01_id,9);
	//Flash_Page_Read(BANK0,buf,sizeof(buf));
	Flash_Page_Read(BANK0,buf,sizeof(buf));
	for(i=0;i<FLASH_STORE_LEN;i++) {
		flash_store_value[i] = ((uint32_t)buf[i*4+3]<<24)|((uint32_t)buf[i*4+2]<<16)|((uint32_t)buf[i*4+1]<<8)|(uint32_t)buf[i*4];
	}
	
	accel_offset[0].x = (int16_t)(flash_store_value[0]>>16);
	accel_offset[0].y = (int16_t)(flash_store_value[0]&0x0000FFFF);
	accel_offset[0].z = (int16_t)(flash_store_value[1]>>16);
	accel_offset[1].x = (int16_t)(flash_store_value[1]&0x0000FFFF);
	accel_offset[1].y = (int16_t)(flash_store_value[2]>>16);
	accel_offset[1].z = (int16_t)(flash_store_value[2]&0x0000FFFF);
}

void Save_Parameters(void)
{
	uint8_t buf[FLASH_STORE_LEN*4],i;
	
	for(i=0;i<FLASH_STORE_LEN;i++) {
		buf[i*4] = flash_store_value[i]&0x000000FF;
		buf[i*4+1] = (flash_store_value[i]&0x0000FF00)>>8;
		buf[i*4+2] = (flash_store_value[i]&0x00FF0000)>>16;
		buf[i*4+3] = (flash_store_value[i]&0xFF000000)>>24;
	}
	Flash_Page_Write(BANK0,buf,sizeof(buf));
}
