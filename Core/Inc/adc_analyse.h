#ifndef __adc_analyse_H
#define __adc_analyse_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define ADC_Time    htim3

typedef struct
{
	float Vpp;   // 电压峰峰值
	
	float Vavg_AC;  // 交流耦合电压平均值  不推荐使用
	float Vmax_AC;  // 交流耦合电压最大值
   float Vmin_AC;  // 交流耦合电压最小值	
	
	float Vavg_DC;  // 直流耦合电压平均值
	float Vmax_DC;  // 直流耦合电压最大值
   float Vmin_DC;  // 直流耦合电压最小值
	
}adc_data_t;

extern  adc_data_t ADC_IN1,ADC_IN2;
extern uint32_t ADC_FreqSet;

/* 测试过程记录 */

extern uint32_t Cheak_Start_Flag; // 是否开始测试标志
extern uint32_t Cheak_Sum; // 步骤记录
extern uint32_t Amp_Re_Cheak; 
void DualADC_Start(void);
void ADC_SetFreq(uint32_t Freq);
void ADC_Analysis(void);

#ifdef __cplusplus
}
#endif
#endif

