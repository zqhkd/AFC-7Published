#ifndef W25Q128_W25QXX_H_
#define W25Q128_W25QXX_H_
 
#include "stm32H7xx_hal.h" //HAL搴撴枃浠跺０鏄
#include "stdbool.h"
#include "AFCGlobalVar.h"
#include "ACGCommonAPI.h"

//FLASH_CS 鐗囬€夊紩鑴氬畾涔
#define W25Q128_CS_GPIO_Port GPIOD
#define W25Q128_CS_Pin GPIO_PIN_10
 
//25绯诲垪FLASH鑺墖鍘傚晢涓庡閲忎唬鍙凤紙鍘傚晢浠ｅ彿EF锛
#define W25Q80    0XEF13
#define W25Q16    0XEF14
#define W25Q32    0XEF15
#define W25Q64    0XEF16
#define W25Q128   0XEF17
#define W25Q256 0XEF18
#define W25Q01JV 0XEF20
#define EX_FLASH_ADD 0x000000 //W25Q128鐨勫湴鍧€鏄24浣嶅
extern uint16_t W25QXX_TYPE;//瀹氫箟W25QXX鑺墖鍨嬪彿
extern SPI_HandleTypeDef hspi2;
//
//鎸囦护琛
#define W25X_WriteEnable             0x06
#define W25X_WriteDisable            0x04
#define W25X_ReadStatusReg1      0x05
#define W25X_ReadStatusReg2      0x35
#define W25X_ReadStatusReg3      0x15
#define W25X_WriteStatusReg1         0x01
#define W25X_WriteStatusReg2         0x31
#define W25X_WriteStatusReg3     0x11
#define W25X_ReadData             0x03
#define W25X_FastReadData         0x0B
#define W25X_FastReadDual         0x3B
#define W25X_PageProgram          0x02
#define W25X_BlockErase              0xD8
#define W25X_SectorErase          0x20
#define W25X_ChipErase            0xC7
#define W25X_PowerDown            0xB9
#define W25X_ReleasePowerDown    0xAB
#define W25X_DeviceID             0xAB
#define W25X_ManufactDeviceID    0x90
#define W25X_JedecDeviceID           0x9F
#define W25X_Enable4ByteAddr         0xB7
#define W25X_Exit4ByteAddr        0xE9
uint8_t SPI1_ReadWriteByte(uint8_t  TxData);//SPI1鎬荤嚎搴曞眰璇诲啓
void W25QXXtest(void);
void W25QXX_CS(uint8_t a);//W25QXX鐗囬€夊紩鑴氭帶鍒
uint8_t W25QXX_Init(void);//鍒濆鍖朩25QXX鍑芥暟
uint16_t  W25QXX_ReadID(void);//璇诲彇FLASH ID
uint8_t W25QXX_ReadSR(uint8_t regno);//璇诲彇鐘舵€佸瘎瀛樺櫒
void W25QXX_4ByteAddr_Enable(void);//浣胯兘4瀛楄妭鍦板潃妯″紡
void W25QXX_Write_SR(uint8_t regno,uint8_t  sr);//鍐欑姸鎬佸瘎瀛樺櫒
void W25QXX_Write_Enable(void);//鍐欎娇鑳
void W25QXX_Write_Disable(void);//鍐欎繚鎶
void W25QXX_Write_NoCheck(uint8_t*  pBuffer,uint32_t WriteAddr,uint16_t  NumByteToWrite);//鏃犳楠屽啓SPI FLASH
void W25QXX_Read(uint8_t* pBuffer,uint32_t  ReadAddr,uint16_t NumByteToRead);//璇诲彇flash
void W25QXX_Write(uint8_t* pBuffer,uint32_t  WriteAddr,uint16_t NumByteToWrite);//鍐欏叆flash
void W25QXX_Erase_Chip(void);//鏁寸墖鎿﹂櫎
void W25QXX_Erase_Sector(uint32_t  Dst_Addr);//鎵囧尯鎿﹂櫎
void W25QXX_Wait_Busy(void);//绛夊緟绌洪棽
void W25QXX_PowerDown(void);//杩涘叆鎺夌數妯″紡
void W25QXX_WAKEUP(void);//鍞ら啋
void W25QXX_Write_Page(uint8_t*  pBuffer,uint32_t WriteAddr,uint16_t  NumByteToWrite);

// 搴旂敤绋嬪簭鎺ュ彛鍑芥暟
extern bool g_bUsedOfUSBDisk;
void initUSBDisk(char *fileLeadName,uint8_t varNum,char *varName, uint16_t iWriteFreq);
void writeUSBDisk(uint8_t iChannel,float fVal); 
#endif /* W25Q128_W25QXX_H_ */
