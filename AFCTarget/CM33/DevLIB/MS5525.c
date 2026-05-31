#include "MS5525.h"
#include "Ms5525I2C.h" 
#include "math.h"
#include "main.h"

#include "ACGCommonAPI.h"
#include "AFCTask.h"

#define  CONSTANTS_AIR_DENSITY_SEA_LEVEL_15C  1.225f

bool bCalDiffPressPaZeroOff = false;
float diff_press_pa_raw =0;
float diff_press_vel =0;     //鏈护娉㈢殑IAS
float diff_press_vel_iir =0; //婊ゆ尝鍚庣殑IAS
float diff_press_vel_now =0; 
float diff_press_init =0;
float ms5525_temperature =0; //MS5525鐨勮姱鐗囨俯搴

float gMS5525SampleTim;  // MS5525鐨勯噰鏍锋椂闂

bool g_bUsedOfMs5525 = false;   // 绌洪€熺Ms5525浣跨敤鏍囧織

 /**
  * Calculate indicated airspeed (IAS).
  *
  * Note that the indicated airspeed is not the true airspeed because it
  * lacks the air density and instrument error compensation.
  *
  * @param differential_pressure total_ pressure - static pressure
  * @return IAS in m/s
  */
 float calc_IAS(float differential_pressure)//鍘嬪樊杞琁AS 鍗曚綅锛歮/s
 {
	   if(differential_pressure < 0) differential_pressure = 0.001f;      // V5.04.240710: 澶勭悊寮傚父鎯呭喌锛岄槻姝 V = Nan
     return sqrtf((2.0f * differential_pressure) / CONSTANTS_AIR_DENSITY_SEA_LEVEL_15C);
 }
 
 float ms5525_vel_KalmanFilter(float ResrcData,float ProcessNiose_Q,float MeasureNoise_R,float InitialPrediction)
{
    float R = MeasureNoise_R;
    float Q = ProcessNiose_Q;

    static  float x_last;

    float x_mid = x_last;
    float x_now;

    static  float p_last;

    float p_mid ;
    float p_now;
    float kg;

    x_mid=x_last; //x_last=x(k-1|k-1),x_mid=x(k|k-1)
    p_mid=p_last+Q; //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=脭毛脡霉
    kg=p_mid/(p_mid+R); //kg脦陋kalman filter拢卢R脦陋脭毛脡霉
    x_now=x_mid+kg*(ResrcData-x_mid);//鹿脌录脝鲁枚碌脛脳卯脫脜脰碌

    p_now=(1-kg)*p_mid;//脳卯脫脜脰碌露脭脫娄碌脛covariance

    p_last = p_now; //赂眉脨脗covariance脰碌
    x_last = x_now; //赂眉脨脗脧碌脥鲁脳麓脤卢脰碌

    return x_now;
}

// 璁＄畻宸帇淇″彿鐨勯浂鍋忔牎鍑嗗€笺€傛牎鍑嗘爣蹇梑CalDiffPressPaZeroOff涓虹湡琛ㄧず宸叉牎鍑嗭紝闆跺亸鏍″噯鍊煎瓨鏀惧湪diff_press_init涓
//  400娆￠棿闅50ms閲囨牱闆跺亸鍊硷紙绾20s锛夊彇鍧囧€硷紝浣滀负闆跺亸鏍″噯鍊
void calDiffPressZeroOff(uint32_t iCaliCounter,bool bCaliDoing)
{
	if(g_bUsedOfMs5525){
		if((iCaliCounter==1)&&bCaliDoing){
			diff_press_init = diff_press_pa_raw;
			iCounter = 0;
		}
		
		// V5.05.240925: 鍙栨秷initDiffPressZeroOff鍜宑alDiffPressZeroOff涓diff_press_pa_raw宸帇鍘熷淇″彿(20,150)鐨勯檺骞呭鐞
//		if((diff_press_pa_raw > 20) && (diff_press_pa_raw < 150)){
			  diff_press_init = (diff_press_init + diff_press_pa_raw)/2;
//		}
//		else diff_press_init = 0;
	}
}


// 璁＄畻宸帇淇″彿鐨勯浂鍋忔牎鍑嗗€笺€傛牎鍑嗘爣蹇梑CalDiffPressPaZeroOff涓虹湡琛ㄧず宸叉牎鍑嗭紝闆跺亸鏍″噯鍊煎瓨鏀惧湪diff_press_init涓
//  40s闂撮殧10ms閲囨牱闆跺亸鍊硷紙绾20s锛夊彇鍧囧€硷紝浣滀负闆跺亸鏍″噯鍊
uint32_t T1,T2,T3,T4;

void initDiffPressZeroOff(float fSampleTim)
{
	if(g_bUsedOfMs5525){
		float diffPressSum = 0;
		uint16_t iSampleNum = 0,TotalNum = 40;
		bCalDiffPressPaZeroOff = false;   // 鍚姩鏍″噯
		
		if(fSampleTim > 0)	TotalNum = (uint16_t)(fSampleTim*1000/60);
		
		T1 = HAL_GetTick();
		for (uint16_t i = 0; i < TotalNum; i++) {
			T2 = HAL_GetTick();
			MS5525_get_diff_pressure_temp(&diff_press_pa_raw,&ms5525_temperature);   // 璇ュ嚱鏁版墽琛岄渶瑕佽€楁椂54ms
			T3 = HAL_GetTick() - T2;
			HAL_Delay(5);      // HAL_Delay鍗曚綅涓烘绉

			// V5.05.240925: 鍙栨秷initDiffPressZeroOff鍜宑alDiffPressZeroOff涓diff_press_pa_raw宸帇鍘熷淇″彿(20,150)鐨勯檺骞呭鐞
//			if((diff_press_pa_raw > 20) && (diff_press_pa_raw < 150)){
				 diffPressSum = diffPressSum + diff_press_pa_raw ; iSampleNum++;			
//			}
		}
		T4 = HAL_GetTick() - T1;

		if(iSampleNum > 0){
			diff_press_init = diffPressSum/iSampleNum;
			bCalDiffPressPaZeroOff= true;   // 琛ㄧず鏍″噯姝ｅ父
		}
	}
}

void initMS5525(float sampleTim)
{
//	float ms5525_vel0 = 0 , ms5525_diff_press = 0 , ms5525_vel_init = 0 , vel_add = 0 , sta1 = 0 , vel_now = 0 ;	
	ms5525_iic_init();
	MS5525_write_byte(MS5525_CMD_RESET);HAL_Delay(3);
	MS5525_write_byte(MS5525_CMD_RESET);HAL_Delay(3);
	//	my_printf("diff_press_vel_init %.1f %.1f \n",diff_press_init,sta1);
	g_bUsedOfMs5525 = true;
	
	gMS5525SampleTim = sampleTim;
	
	initDiffPressZeroOff(5);   // 鍒濆鍖栨椂鐢5s閲囨牱鏃堕棿娈电殑鍧囧€间綔涓洪浂鍋忔牎鍑嗗€笺€
}

//ms5525鐨勬暟鎹幏鍙
void MS5525_ReadData(void)
{
		//鑾峰彇ms5525鐨勫帇宸紝鍘嬪樊杞寲鐨勯€熷害IAS锛宮s5525鐨勬俯搴
		MS5525_get_diff_pressure_temp(&diff_press_pa_raw,&ms5525_temperature);
	
	  
	
	  //杈撳嚭鐨勫帇宸负褰撳墠鍘嬪樊鍑忓幓鏍″噯鍊
		float diff_press_pa_raw_out = diff_press_pa_raw - diff_press_init; 
	  //寰楀埌閫熷害
	  diff_press_vel = calc_IAS(diff_press_pa_raw_out);
	
  	  //瀵归€熷害杩涜鍗″皵鏇兼护娉紝Q<R 锛岃鏍规嵁鍏蜂綋杩愯棰戠巼璋冭妭Q锛孯
	  diff_press_vel_iir = ms5525_vel_KalmanFilter(diff_press_vel,0.05,0.4,0);
	
}
