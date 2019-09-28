#ifndef __adc_analyse_H
#define __adc_analyse_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define ADC_Time    htim3

typedef struct
{
	float Vpp;   // ��ѹ���ֵ
	
	float Vavg_AC;  // ������ϵ�ѹƽ��ֵ  ���Ƽ�ʹ��
	float Vmax_AC;  // ������ϵ�ѹ���ֵ
   float Vmin_AC;  // ������ϵ�ѹ��Сֵ	
	
	float Vavg_DC;  // ֱ����ϵ�ѹƽ��ֵ
	float Vmax_DC;  // ֱ����ϵ�ѹ���ֵ
   float Vmin_DC;  // ֱ����ϵ�ѹ��Сֵ
	
}adc_data_t;

extern  adc_data_t ADC_IN1,ADC_IN2;
extern uint32_t ADC_FreqSet;

/* ���Թ��̼�¼ */

extern uint32_t Cheak_Start_Flag; // �Ƿ�ʼ���Ա�־
extern uint32_t Cheak_Sum; // �����¼
extern uint32_t Amp_Re_Cheak; 
void DualADC_Start(void);
void ADC_SetFreq(uint32_t Freq);
void ADC_Analysis(void);

#ifdef __cplusplus
}
#endif
#endif

