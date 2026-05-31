#include "W25QXX.h"
#include "main.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

uint16_t W25QXX_TYPE=W25Q01JV;//榛樿鏄疻25Q128
//4Kbytes涓轰竴涓猄ector
//16涓墖鍖轰负1涓狟lock
//W25Q128
//瀹归噺涓16M瀛楄妭,鍏辨湁128涓狟lock,4096涓猄ector
//SPI2鎬荤嚎璇诲啓涓€涓瓧鑺
//鍙傛暟鏄啓鍏ョ殑瀛楄妭锛岃繑鍥炲€兼槸璇诲嚭鐨勫瓧鑺
#define W25QXX_FILE 0
#define W25QXX_DMA  1
#define W25QXX_MODE W25QXX_FILE
#define MAX_FLOAT_NUM_W25QXX  120
FATFS FlashFatFS; 
const TCHAR* FlashPath = "0:/";
uint8_t  file_ready = 0;
FIL fil;
FRESULT retW25QXX;
uint32_t bw;
uint32_t fre_clust, fre_sect=0, tot_sect=0;
uint16_t write_success_cnt = 0;
uint8_t FATS_Buff[_MAX_SS];
float g_fW25QXXData[MAX_FLOAT_NUM_W25QXX];  
bool g_bUsedOfUSBDisk = false;
uint8_t g_iSaveVarNum;
char filpath[256];

uint8_t W25QXX_Init(void)
{	
    uint8_t temp;//瀹氫箟涓€涓彉閲弔emp
		HAL_GPIO_WritePin(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
//	if(HAL_GPIO_ReadPin(GPIOB,  GPIO_PIN_12)!= GPIO_PIN_SET)
//	Error_Handler();
//	HAL_GPIO_ReadPin(GPIOD,  GPIO_PIN_10);
    W25QXX_TYPE = W25QXX_ReadID();//璇诲彇FLASH  ID.
    if(W25QXX_TYPE == W25Q01JV)//SPI FLASH涓篧25Q256鏃舵墠鐢ㄨ缃负4瀛楄妭鍦板潃妯″紡
    {
       temp = W25QXX_ReadSR(3);//璇诲彇鐘舵€佸瘎瀛樺櫒3锛屽垽鏂湴鍧€妯″紡
       if((temp&0x01)==0)//濡傛灉涓嶆槸4瀛楄妭鍦板潃妯″紡,鍒欒繘鍏4瀛楄妭鍦板潃妯″紡
       {
           W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
           SPI1_ReadWriteByte(W25X_Enable4ByteAddr);//鍙戦€佽繘鍏4瀛楄妭鍦板潃妯″紡鎸囦护
           W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
       }
    }
		return temp;
}

void W25QXXtest(void)
{
  char fileName[] = "hello";
	static uint8_t W25QXX_init = 0;
	FATFS *fs1;
//	uint8_t data[200];
	uint32_t W25QXX_index=0;
//	uint8_t *buf=NULL;
	snprintf(filpath, sizeof(filpath),"0:/%s.csv", fileName);
	#if SD_MODE == SD_FILE
	if(W25QXX_init==0) {
		W25QXX_init = 1;
		retW25QXX = f_mount(NULL, "", 0); 
		retW25QXX = f_mount(&USERFatFS, (TCHAR const *)FlashPath, 1);
		if (retW25QXX) return;
		
		retW25QXX = f_getfree((const TCHAR*)FlashPath, (DWORD*)&fre_clust, &fs1);
		if (retW25QXX == 0)
		{
			tot_sect = (USERFatFS.n_fatent-2)*fs1->csize;
			fre_sect = fre_clust*fs1->csize;
		}
		
		retW25QXX = f_open(&fil, filpath, FA_CREATE_ALWAYS|FA_WRITE);  //鍒涘缓骞舵墦鍗℃枃浠
		if (retW25QXX) return;
		file_ready = 1;
	}
	if(file_ready) {
//		for (int i = 0; i < 200; i++)
//		{
//			data[i] = i + 1 + W25QXX_index;
//		}
//		retW25QXX = f_write(&fil, &data, sizeof(data), (void *)&bw);
		for(int i=1;i<=5;i++){	
			char d='0'+i;
			retW25QXX=f_write(&fil, &d, sizeof(d), (void *)&bw);
			f_write( &fil, ",", 1,(void *)&bw); 
			}
		f_write( &fil,"\r\n", 1,(void *)&bw); 
		
		for(int i=1;i<=5;i++){	
			char d='6'-i;
			f_write(&fil, &d, sizeof(d), (void *)&bw);
			f_write( &fil, ",", 1,(void *)&bw); 
			}
		f_write( &fil,"\r\n", 1,(void *)&bw); 
		//鍐欏叆鏂囦欢
		if (retW25QXX)
		{
//			g_iWriteW25QXXFailedCnt++;
		}
		else
		{
			write_success_cnt++;
		}
		
		W25QXX_index++;
		
		retW25QXX=f_sync(&fil);
//		if (retW25QXX) Error_Handler();
	}
	#elif SD_MODE == SD_DMA
	if(sd_init==0) {
		sd_init = 1;
		
		if(HAL_SD_Init(&hsd1)==HAL_OK) {
			HAL_SD_GetCardCID(&hsd1,&SDCard_CID);	//鑾峰彇CID
			HAL_SD_GetCardInfo(&hsd1,&SDCardInfo); //鑾峰彇SD鍗′俊鎭
			file_ready = 1;
		}
	} else {
		if(file_ready) {
			for (int i = 0; i < 20; i++)
			{
				data[i] = i + 1 + sd_index;
			}
			HAL_SD_ReadBlocks_DMA(&hsd1,buf_s,sd_index,1);
			sd_index++;
			HAL_SD_WriteBlocks_DMA(&hsd1,data,sd_index,1);
		}
	}
	#endif
}

uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;//瀹氫箟涓€涓彉閲廟xdata
     HAL_SPI_TransmitReceive(&hspi2,&TxData,&Rxdata,1,1000);//璋冪敤鍥轰欢搴撳嚱鏁版敹鍙戞暟鎹
    return Rxdata;//杩斿洖鏀跺埌鐨勬暟鎹
}

void W25QXX_CS(uint8_t a)//杞欢鎺у埗鍑芥暟锛0涓轰綆鐢靛钩锛屽叾浠栧€间负楂樼數骞筹級
{
    if(a==0)HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port, W25Q128_CS_Pin, GPIO_PIN_RESET);
    else  HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port,  W25Q128_CS_Pin, GPIO_PIN_SET);

}


#define MAX_FILE_BYTE_NUM_PER_LINE 1024
BYTE work[_MAX_SS];
//鍒濆鍖朥SB鐨凢LASH鐩橈紝鍖呮嫭SPI FLASH鐨処O鍙
void initUSBDisk(char *fileLeadName,uint8_t varNum,char *varName, uint16_t iWriteFreq){
  W25QXX_Init();
  char fileName[30];
	char tmpName[30];
	FATFS *fs1;
	uint32_t i,bw;
	uint8_t buff[MAX_FILE_BYTE_NUM_PER_LINE];
	g_iSaveVarNum = varNum;
	g_iUSBDiskWriteFreq = iWriteFreq;
	
	retW25QXX = FR_DISK_ERR;
	if(f_mount(&FlashFatFS,(TCHAR const *)FlashPath,1) ==0){
		retW25QXX = f_getfree((const TCHAR*)FlashPath, (DWORD*)&fre_clust, &fs1);
	}
	else
		retW25QXX = f_mkfs((TCHAR const*)fileLeadName, FM_FAT, 0, FATS_Buff, sizeof(FATS_Buff));
	if (retW25QXX == FR_OK)
	{
		tot_sect = (FlashFatFS.n_fatent-2)*fs1->csize;
		fre_sect = fre_clust*fs1->csize;
		
		if(fileLeadName!=NULL)
			 sprintf(tmpName,"%s",fileLeadName);
		else
			 sprintf(tmpName,"tFile");
			
		sprintf(fileName,"%s.dat",tmpName);

		uint8_t tmpC = 1;
		// 涓洪槻姝㈢敤鎴峰疄楠屾椂蹇樿鏇存敼瀛樼洏鏂囦欢鍚嶏紝褰撳湪SD Card涓嚭鐜板悓鍚嶆枃浠舵椂锛屼細鍦ㄨ鏂囦欢鍚嶅悗鍔燺A鏉ユ鏌ユ槸鍚︿粛鐒跺悓鍚嶏紝濡傛灉杩樺悓鍚嶏紝缁х画杩藉姞锛岀洿鑷充笉鍚屽悕銆
		while(f_open(&fil,fileName,FA_READ)==0){
			f_close(&fil);
			if(tmpC > 99){
				sprintf(fileName,"%s_01.dat",tmpName);   // 濡傛灉杩炵画寰幆鑷崇26涓枃浠堕兘閲嶅锛屽垯瑕嗙洊_A鏂囦欢銆
				break;
			}
			else{
				sprintf(fileName,"%s_%02d.dat",tmpName,tmpC);
				tmpC = tmpC + 1;
			}
		}

	  g_bUsedOfUSBDisk = false;
		retW25QXX = f_open(&fil, fileName, FA_CREATE_ALWAYS|FA_WRITE);  //鍒涘缓骞舵墦鍗℃枃浠
		if (!retW25QXX){
				file_ready = 1;
			
				// 鍐欏叆鏁版嵁涓暟鍜屽彉閲忓悕title
				uint16_t iLen = strlen(varName);
				buff[0] = varNum;
			  
			  g_bUsedOfUSBDisk = true;
			
		// 娴嬭瘯涓彂鐜版湁鍙橀噺鍚嶈緝澶氭椂鍑虹幇寮傚父銆俈5.05.241013涓皢MAX_FILE_BYTE_NUM_PER_LINE鐢260涓洿鏀逛负1024銆傝繖鏍峰彲纭繚100涓彉閲忔儏鍐典笅锛屽钩鍧囨瘡涓彉閲10涓瓧绗
			  if(iLen > MAX_FILE_BYTE_NUM_PER_LINE) iLen = MAX_FILE_BYTE_NUM_PER_LINE - 1;  
				for(i = 0; i < iLen; i++) buff[i+1] = (uint8_t)varName[i];
				buff[++i] = 0x0d; buff[++i] = 0x0a;
				f_write(&fil,buff,iLen+3,(void *)&bw);     // V5.02.231002: 鐢变簬澧炲姞浜0x0d,0a涓や釜瀛楄妭锛岄暱搴﹀啀鍔2锛屽彟澶栦竴涓槸鍙橀噺涓暟锛屽叡澧炲姞3涓€
			  f_sync(&fil);
		}
	}
}


//璇诲彇W25QXX鐨勭姸鎬佸瘎瀛樺櫒锛學25QXX涓€鍏辨湁3涓姸鎬佸瘎瀛樺櫒
//鐘舵€佸瘎瀛樺櫒1锛
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:榛樿0,鐘舵€佸瘎瀛樺櫒淇濇姢浣,閰嶅悎WP浣跨敤
//TB,BP2,BP1,BP0:FLASH鍖哄煙鍐欎繚鎶よ缃
//WEL:鍐欎娇鑳介攣瀹
//BUSY:蹇欐爣璁颁綅(1,蹇;0,绌洪棽)
//榛樿:0x00
//鐘舵€佸瘎瀛樺櫒2锛
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//鐘舵€佸瘎瀛樺櫒3锛
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS (R) (R)
//regno:鐘舵€佸瘎瀛樺櫒鍙凤紝鑼:1~3
//杩斿洖鍊:鐘舵€佸瘎瀛樺櫒鍊
uint8_t W25QXX_ReadSR(uint8_t regno)
{
    uint8_t byte=0,command=0;
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;//璇荤姸鎬佸瘎瀛樺櫒1鎸囦护
            break;
        case 2:
            command=W25X_ReadStatusReg2;//璇荤姸鎬佸瘎瀛樺櫒2鎸囦护
            break;
        case 3:
            command=W25X_ReadStatusReg3;//璇荤姸鎬佸瘎瀛樺櫒3鎸囦护
            break;
        default:
            command=W25X_ReadStatusReg1;//璇荤姸鎬佸瘎瀛樺櫒1鎸囦护
            break;
    }
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(command);//鍙戦€佽鍙栫姸鎬佸瘎瀛樺櫒鍛戒护
    byte=SPI1_ReadWriteByte(0Xff);//璇诲彇涓€涓瓧鑺
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    return byte;//杩斿洖鍙橀噺byte
}

//鍐橶25QXX鐘舵€佸瘎瀛樺櫒
void W25QXX_Write_SR(uint8_t regno,uint8_t  sr)
{
    uint8_t command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;//鍐欑姸鎬佸瘎瀛樺櫒1鎸囦护
            break;
        case 2:
            command=W25X_WriteStatusReg2;//鍐欑姸鎬佸瘎瀛樺櫒2鎸囦护
            break;
        case 3:
            command=W25X_WriteStatusReg3;//鍐欑姸鎬佸瘎瀛樺櫒3鎸囦护
            break;
        default:
            command=W25X_WriteStatusReg1;
            break;
    }
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(command);//鍙戦€佸啓鍙栫姸鎬佸瘎瀛樺櫒鍛戒护
    SPI1_ReadWriteByte(sr);//鍐欏叆涓€涓瓧鑺
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
}

//W25QXX鍐欎娇鑳
//灏哤EL缃綅
void W25QXX_Write_Enable(void)
{
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_WriteEnable);//鍙戦€佸啓浣胯兘
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
}

//W25QXX鍐欑姝
//灏哤EL娓呴浂
void W25QXX_Write_Disable(void)
{
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_WriteDisable);//鍙戦€佸啓绂佹鎸囦护
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
}

//璇诲彇鑺墖ID
//楂8浣嶆槸鍘傚晢浠ｅ彿锛堟湰绋嬪簭涓嶅垽鏂巶鍟嗕唬鍙凤級
//浣8浣嶆槸瀹归噺澶у皬
//0XEF13鍨嬪彿涓篧25Q80
//0XEF14鍨嬪彿涓篧25Q16
//0XEF15鍨嬪彿涓篧25Q32
//0XEF16鍨嬪彿涓篧25Q64
//0XEF17鍨嬪彿涓篧25Q128锛堢洰鍓嶆磱妗2鍙峰紑鍙戞澘浣跨敤128瀹归噺鑺墖锛
//0XEF18鍨嬪彿涓篧25Q256
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
//    SPI1_ReadWriteByte(0x90);//鍙戦€佽鍙朓D鍛戒护
//    SPI1_ReadWriteByte(0x00);
//    SPI1_ReadWriteByte(0x00);
//    SPI1_ReadWriteByte(0x00);

//    Temp|=SPI1_ReadWriteByte(0xFF)<<8;
//    Temp|=SPI1_ReadWriteByte(0xFF);
		uint8_t tx_buf[6] = {0x90, 0x00, 0x00, 0x00, 0xFF, 0xFF}; // 鍛戒护+鍦板潃+2娆¤
		uint8_t rx_buf[6];
		HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, 6, 1000);
		Temp = (rx_buf[4] << 8) | rx_buf[5];
		//SPI1_ReadWriteByte(0x9F);        // 鍙戦€ JEDEC ID 鍛戒护
		//uint8_t manuf_id = SPI1_ReadWriteByte(0xFF); // 璇诲彇鍒堕€犲晢 ID (0xEF)
		//uint8_t mem_type = SPI1_ReadWriteByte(0xFF); // 瀛樺偍绫诲瀷 (0x40)
		//uint8_t capacity = SPI1_ReadWriteByte(0xFF);
		//Temp = (mem_type << 8) | capacity; 
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂


 // 瀹归噺鏍囪瘑 (0x17)

// 璁惧 ID: 0x4017
return Temp;
}

//璇诲彇SPI FLASH
//鍦ㄦ寚瀹氬湴鍧€寮€濮嬭鍙栨寚瀹氶暱搴︾殑鏁版嵁
//pBuffer:鏁版嵁瀛樺偍鍖
//ReadAddr:寮€濮嬭鍙栫殑鍦板潃(24bit)
//NumByteToRead:瑕佽鍙栫殑瀛楄妭鏁(鏈€澶65535)
void W25QXX_Read(uint8_t* pBuffer,uint32_t  ReadAddr,uint16_t NumByteToRead)
{
    uint16_t i;
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_ReadData);//鍙戦€佽鍙栧懡浠
     SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>24));
     SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>16));//鍙戦€24bit鍦板潃
     SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI1_ReadWriteByte((uint8_t)ReadAddr);
    for(i=0;i<NumByteToRead;i++)
    {
         pBuffer[i]=SPI1_ReadWriteByte(0XFF);//寰幆璇绘暟
    }
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
}

//SPI鍦ㄤ竴椤(0~65535)鍐呭啓鍏ュ皯浜256涓瓧鑺傜殑鏁版嵁
//鍦ㄦ寚瀹氬湴鍧€寮€濮嬪啓鍏ユ渶澶256瀛楄妭鐨勬暟鎹
//pBuffer:鏁版嵁瀛樺偍鍖
//WriteAddr:寮€濮嬪啓鍏ョ殑鍦板潃(24bit)
//NumByteToWrite:瑕佸啓鍏ョ殑瀛楄妭鏁(鏈€澶256),璇ユ暟涓嶅簲璇ヨ秴杩囪椤电殑鍓╀綑瀛楄妭鏁!!!
void W25QXX_Write_Page(uint8_t*  pBuffer,uint32_t WriteAddr,uint16_t  NumByteToWrite)
{
    uint16_t i;
    W25QXX_Write_Enable();//SET WEL
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_PageProgram);//鍙戦€佸啓椤靛懡浠
     SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>24));
     SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>16));//鍙戦€24bit鍦板潃
     SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI1_ReadWriteByte((uint8_t)WriteAddr);
     for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//寰幆鍐欐暟
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    W25QXX_Wait_Busy();//绛夊緟鍐欏叆缁撴潫
}

//鏃犳楠屽啓SPI FLASH
//蹇呴』纭繚鎵€鍐欑殑鍦板潃鑼冨洿鍐呯殑鏁版嵁鍏ㄩ儴涓0XFF,鍚﹀垯鍦ㄩ潪0XFF澶勫啓鍏ョ殑鏁版嵁灏嗗け璐!
//鍏锋湁鑷姩鎹㈤〉鍔熻兘
//鍦ㄦ寚瀹氬湴鍧€寮€濮嬪啓鍏ユ寚瀹氶暱搴︾殑鏁版嵁,浣嗘槸瑕佺‘淇濆湴鍧€涓嶈秺鐣!
//pBuffer:鏁版嵁瀛樺偍鍖
//WriteAddr:寮€濮嬪啓鍏ョ殑鍦板潃(24bit)
//NumByteToWrite:瑕佸啓鍏ョ殑瀛楄妭鏁(鏈€澶65535)
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t*  pBuffer,uint32_t WriteAddr,uint16_t  NumByteToWrite)
{
    uint16_t pageremain;
    pageremain=256-WriteAddr%256; //鍗曢〉鍓╀綑鐨勫瓧鑺傛暟
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//涓嶅ぇ浜256涓瓧鑺
    while(1)
    {
       W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
       if(NumByteToWrite==pageremain)break;//鍐欏叆缁撴潫浜
        else //NumByteToWrite>pageremain
       {
           pBuffer+=pageremain;
           WriteAddr+=pageremain;
           NumByteToWrite-=pageremain;            //鍑忓幓宸茬粡鍐欏叆浜嗙殑瀛楄妭鏁
           if(NumByteToWrite>256)pageremain=256; //涓€娆″彲浠ュ啓鍏256涓瓧鑺
           else pageremain=NumByteToWrite;     //涓嶅256涓瓧鑺備簡
       }
    };
}

//鍐橲PI FLASH
//鍦ㄦ寚瀹氬湴鍧€寮€濮嬪啓鍏ユ寚瀹氶暱搴︾殑鏁版嵁
//璇ュ嚱鏁板甫鎿﹂櫎鎿嶄綔!
//pBuffer:鏁版嵁瀛樺偍鍖
//WriteAddr:寮€濮嬪啓鍏ョ殑鍦板潃(24bit)
//NumByteToWrite:瑕佸啓鍏ョ殑瀛楄妭鏁(鏈€澶65535)
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t* pBuffer,uint32_t  WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t* W25QXX_BUF;
    W25QXX_BUF=W25QXX_BUFFER;
    secpos=WriteAddr/4096;//鎵囧尯鍦板潃
    secoff=WriteAddr%4096;//鍦ㄦ墖鍖哄唴鐨勫亸绉
    secremain=4096-secoff;//鎵囧尯鍓╀綑绌洪棿澶у皬
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//娴嬭瘯鐢
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//涓嶅ぇ浜4096涓瓧鑺
    while(1)
    {
       W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//璇诲嚭鏁翠釜鎵囧尯鐨勫唴瀹
       for(i=0;i<secremain;i++)//鏍￠獙鏁版嵁
       {
           if(W25QXX_BUF[secoff+i]!=0XFF)break;//闇€瑕佹摝闄
       }
       if(i<secremain)//闇€瑕佹摝闄
       {
           W25QXX_Erase_Sector(secpos);//鎿﹂櫎杩欎釜鎵囧尯
           for(i=0;i<secremain;i++)//澶嶅埗
           {
               W25QXX_BUF[i+secoff]=pBuffer[i];
           }
           W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//鍐欏叆鏁翠釜鎵囧尯
       }else  W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//鍐欏凡缁忔摝闄や簡鐨,鐩存帴鍐欏叆鎵囧尯鍓╀綑鍖洪棿.
       if(NumByteToWrite==secremain)break;//鍐欏叆缁撴潫浜
       else//鍐欏叆鏈粨鏉
       {
           secpos++;//鎵囧尯鍦板潃澧1
           secoff=0;//鍋忕Щ浣嶇疆涓0
           pBuffer+=secremain;  //鎸囬拡鍋忕Щ
           WriteAddr+=secremain;//鍐欏湴鍧€鍋忕Щ
           NumByteToWrite-=secremain;//瀛楄妭鏁伴€掑噺
           if(NumByteToWrite>4096)secremain=4096;//涓嬩竴涓墖鍖鸿繕鏄啓涓嶅畬
           else  secremain=NumByteToWrite;//涓嬩竴涓墖鍖哄彲浠ュ啓瀹屼簡
       }
    };
}

//鎿﹂櫎鏁翠釜鑺墖
//绛夊緟鏃堕棿瓒呴暱...
void W25QXX_Erase_Chip(void)
{
    W25QXX_Write_Enable();//SET WEL
    W25QXX_Wait_Busy();//绛夊緟蹇欑姸鎬
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_ChipErase);//鍙戦€佺墖鎿﹂櫎鍛戒护
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    W25QXX_Wait_Busy();//绛夊緟鑺墖鎿﹂櫎缁撴潫
}

//鎿﹂櫎涓€涓墖鍖
//Dst_Addr:鎵囧尯鍦板潃 鏍规嵁瀹為檯瀹归噺璁剧疆
//鎿﹂櫎涓€涓墖鍖虹殑鏈€灏戞椂闂:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
    Dst_Addr*=4096;
    W25QXX_Write_Enable();//SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_SectorErase);//鍙戦€佹墖鍖烘摝闄ゆ寚浠
    if(W25QXX_TYPE==W25Q01JV)//濡傛灉鏄疻25Q256鐨勮瘽鍦板潃涓4瀛楄妭鐨勶紝瑕佸彂閫佹渶楂8浣
    {
         SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>24));
    }
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>16));//鍙戦€24bit鍦板潃
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI1_ReadWriteByte((uint8_t)Dst_Addr);
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    W25QXX_Wait_Busy();//绛夊緟鎿﹂櫎瀹屾垚
}

//绛夊緟绌洪棽
void W25QXX_Wait_Busy(void)
{
    while((W25QXX_ReadSR(1)&0x01)==0x01);//绛夊緟BUSY浣嶆竻绌
}

//杩涘叆鎺夌數妯″紡
void W25QXX_PowerDown(void)
{
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_PowerDown);//鍙戦€佹帀鐢靛懡浠 0xB9
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    delay_us(3);//绛夊緟TPD
}

//鍞ら啋
void W25QXX_WAKEUP(void)
{
    W25QXX_CS(0);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);//鍙戦€佺數婧愬敜閱掓寚浠 0xAB
    W25QXX_CS(1);//0鐗囬€夊紑鍚紝1鐗囬€夊叧闂
    delay_us(3);//绛夊緟TRES1
}

void DataDMA2W25QXX(void)
{
	uint16_t iLen;
	iLen = sizeof(float)*g_iSaveVarNum;
	
	if(file_ready) {
			retW25QXX = f_write(&fil, (uint8_t *)& g_fW25QXXData, iLen, (void *)&bw);  //鍐欏叆鏂囦欢
			if (retW25QXX)	g_iWriteSDFailedCnt++;
			else	write_success_cnt++;
			
			f_sync(&fil);
	}
}

void writeUSBDisk(uint8_t iChannel,float fVal)
{
	 static int iSigNum = 0;
	 if(iChannel < g_iSaveVarNum){
		g_fW25QXXData[iChannel] = fVal;             // 璇ュ彉閲忎负浠庢ā鍨嬩覆鍙ｅ洖閫佺殑淇″彿
		iSigNum++;
		 if(iSigNum >= g_iSaveVarNum){          // 璇啓涓篻_iModelInSignalNum锛屼粠鑰屽鑷10ms鎵嶅彂閫佷竴甯
				DataDMA2W25QXX();
				iSigNum = 0;
		}
	 }
}
