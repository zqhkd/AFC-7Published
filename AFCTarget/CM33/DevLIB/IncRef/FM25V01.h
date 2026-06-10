#ifndef __FM25V01_H__
#define __FM25V01_H__

#include "stdint.h"
#include "stdbool.h"
#include "AFCGlobalDef.h"

#define FLASH_STORE_LEN 30

extern uint32_t flash_store_value[FLASH_STORE_LEN];
extern bool g_bUsedOfFM25V01;

SSaveParamFlash loadDefaultPara(void);
// 读取参数正确，则返回true,否则false
bool LoadFromFM25V01(SSaveParamFlash *pBuff);
// 将待保存参数写入FM25V01中
void SaveToFM25V01(SSaveParamFlash *pBuff);

void FM25V01_Test(void);
void Load_Parameters(void);
void Save_Parameters(void);
// 授权管理操作
void initSysChipIdValid(void);
// 返回随机数
float fGetRandVal(void);

// 检查产品授权信息
bool bChkProductAuth(TProductConfig cfg);

// 检查是否配置了FM25V01芯片。如果没有安装该芯片，FirstInitFlg设置为0xaabb
void vCheckFM25V01Exist(void);
#endif


