#include "adc_analyse.h"
#include "adc.h"
#include "tim.h"
#include "dac.h"
#include "gpio.h"

#include "cmd.h"
//#include "ringbuff/ringbuff.h"

/* 双缓冲变量 */
uint16_t ADC_IN1_Buff1[500];
uint16_t ADC_IN1_Buff2[500];

uint16_t ADC_IN2_Buff1[500];
uint16_t ADC_IN2_Buff2[500];

uint16_t (* ADC_IN1_Buff_addr)[sizeof(ADC_IN1_Buff1)/sizeof(uint16_t)];
uint16_t (* ADC_IN2_Buff_addr)[sizeof(ADC_IN2_Buff2)/sizeof(uint16_t)];


/* 开始处理标志 */
uint32_t StartAnalyseFlag1,StartAnalyseFlag2; 

/* ADC 频率设置 */
uint32_t ADC_FreqSet;

/* 测试过程记录 */
uint32_t Cheak_Start_Flag; // 是否开始测试标志
uint32_t Cheak_Sum; // 步骤记录
uint32_t Cheak_Set; // 测试设置标志

uint32_t DelayCheakTick,DelayCheakTick1; // 延时测试计数

#define  Cheak_Amp_Rin  (0)  // 测试输入电阻步骤
#define  Cheak_Amp_Rout (1)  // 测试输出电阻步骤
#define  Cheak_Amp_FC   (2)  // 测试幅频特性步骤

#define  Set_Rin_2000mV   (0x01)  // 串66.8K电阻 时 ADC IN2 2000mV

#define GetFreq2Run(__tick,__freq)  (__tick%(1000/__freq) == 0)
#define GetTick   HAL_GetTick  
static uint32_t tick1,tick2;

/* **************** 测试过程结果记录 ******************/

float Cheak_temp,Cheak_temp1,Cheak_temp2; // 测试变量缓存

/* 放大器输出电压无负载时总采样电阻 */
#define Amp_OUT_50KR (50000.f)

uint32_t Amp_Re_Cheak;  // 重复测量（1）（2）（3）计数

/* (1) 放大器输入电阻检测 Amp_Rin (3)放大器1KHz增益 BJT_BD*/
float Amp_Rin; // Amplifier放大器输入电阻
float Amp_Vin_2000mV; // 放大器输出2000mv时实际输入电压
float Amp_Vout_2000mV; // 放大器输出2000mv时实际采样电压
float BJT_BD;  // 三极管放大倍数 == Amplifier放大器 1KHz 时增益
float BJT_Rbe;  // 三极管内阻

/* (2) 放大器输出电阻检测 Amp_Rout*/
#define  Amp_OUT_5KR (4545.f)  // 放大器输出负载电阻 50K 并上 5K
float Amp_Rout;  // Amplifier放大器输出电阻
float Amp_Vout_5KR; // 放大器输出接5KΩ负载时实际输出电压

/* (4) 放大器幅频特性  */
uint32_t FV_Count;
float FVSave[40][2]; // [y][x] y 40组频率和幅值 x 0 频率 1 幅值 

/* 发挥部分 */
uint32_t Test_erro;


void DualADC_Start(void)
{
	ADC_IN1_Buff_addr =  &ADC_IN1_Buff1;
	ADC_IN2_Buff_addr =  &ADC_IN2_Buff1;
  /* 启动ADC1,ADC2 */
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_IN1_Buff_addr,sizeof(*ADC_IN1_Buff_addr)/sizeof(uint16_t));
	HAL_ADC_Start_DMA(&hadc2,(uint32_t*)ADC_IN2_Buff_addr,sizeof(*ADC_IN2_Buff_addr)/sizeof(uint16_t));
	
	/* 设置ADC采样频率 */
	ADC_FreqSet = 100000;   // 100 kHZ
	ADC_SetFreq(ADC_FreqSet); 
	StartAnalyseFlag1 = 0;
	StartAnalyseFlag2 = 0;
	
	Cheak_Start_Flag = 0;
	Cheak_Sum = 0;
	
	Cheak_Set = 0;
	tick1 = GetTick();
	DelayCheakTick = 0;
	DelayCheakTick1 = 0;
	
	Amp_Re_Cheak = 0;
	
	/* (4) */
	FV_Count = 0;
	
	Test_erro = 0;
}

void ADC_SetFreq(uint32_t Freq)
{
	#define TIM_Freq  (84000000)
	uint32_t ARR_ADCTim;  // ADC采样由TIM3 UPDATE 触发 84MHz
	
	HAL_TIM_Base_Stop(&ADC_Time);
	
	ARR_ADCTim = TIM_Freq / Freq - 1;
	if(ARR_ADCTim > 65534) ARR_ADCTim = 65534;
	__HAL_TIM_SetAutoreload(&ADC_Time,ARR_ADCTim);
	__HAL_TIM_SetCounter(&ADC_Time,0);
	
	/* 启动ADC触发定时器 */
	HAL_TIM_Base_Start(&ADC_Time);
	
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{		
	if(hadc == &hadc1)
	{
		
		//HAL_GPIO_TogglePin(D2_GPIO_Port,D2_Pin);
		
		if(ADC_IN1_Buff_addr == &ADC_IN1_Buff1) ADC_IN1_Buff_addr = &ADC_IN1_Buff2;
		else ADC_IN1_Buff_addr = &ADC_IN1_Buff1;
		StartAnalyseFlag1 = 1;
		HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_IN1_Buff_addr,sizeof(*ADC_IN1_Buff_addr)/sizeof(uint16_t));
	}
	else
	{
		StartAnalyseFlag2 =1;
		if(ADC_IN2_Buff_addr == &ADC_IN2_Buff1) ADC_IN2_Buff_addr = &ADC_IN2_Buff2;
		else ADC_IN2_Buff_addr = &ADC_IN2_Buff1;		
		HAL_ADC_Start_DMA(&hadc2,(uint32_t*)ADC_IN2_Buff_addr,sizeof(*ADC_IN2_Buff_addr)/sizeof(uint16_t));
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

	
}
uint32_t hightcount;
uint32_t lowcount;
float freq;
float duty;
size_t len;

adc_data_t ADC_IN1,ADC_IN2;

#define ADC_IN1_Max_Vol  (3300.f)  // 3.3V
#define ADC_IN2_Max_Vol  (12000.f * 3300.f / 2320.f) // 12V时ADC为2.3V


void ADC_Analysis(void)
{
	  /* ADC 读地址指针 */
	  uint16_t (* ADC_IN1_Buff_addrR)[sizeof(ADC_IN1_Buff1)/sizeof(uint16_t)];
	  uint16_t (* ADC_IN2_Buff_addrR)[sizeof(ADC_IN2_Buff1)/sizeof(uint16_t)];
	  /* ADC 求和变量 */
	  uint32_t ADC_IN1_Sum,ADC_IN2_Sum;
	  /* ADC 求最大值变量 */
	  uint32_t ADC_IN1_Max,ADC_IN2_Max;
	  /* ADC 求最小值变量 */
	  uint32_t ADC_IN1_Min,ADC_IN2_Min;	
	  /* 计算缓存变量 */
	  float tempCal; 

	  if(StartAnalyseFlag1 == 1)
	  {
		  StartAnalyseFlag1 = 0;
		  
		  /* 数据切换 双重缓冲 */
		  if(ADC_IN1_Buff_addr == &ADC_IN1_Buff1) ADC_IN1_Buff_addrR = &ADC_IN1_Buff2;
		  else ADC_IN1_Buff_addrR = &ADC_IN1_Buff1;
		
		  /* 变量赋值 */
		  ADC_IN1_Min = (*ADC_IN1_Buff_addrR)[0];
		  ADC_IN1_Max = 0;
		  ADC_IN1_Sum = 0;
		  
		  /* 数据处理 每500个处理一次*/
		  for(uint32_t i=0;i<(sizeof(*ADC_IN1_Buff_addrR)/sizeof(uint16_t));i++) 
		  {
//			  static uint32_t th,tl,flag;
//			  if((*ADC_IN1_Buff_addrR)[i] > 3800) 
//			  {
//				  th++;
//				  if(flag == 0) lowcount = tl,tl = 0,flag = 1;
//				  freq = (100000) / (float)(lowcount + hightcount);
//				  duty = (float)hightcount  / (float)(lowcount + hightcount);
//			  }
//			  else if((*ADC_IN1_Buff_addrR)[i] < 100)
//			  {			  
//				  tl ++;		  
//				  if(flag == 1) hightcount = th,th = 0,flag = 0;
//			  }
			  /* 找出最大值 */
			  if(ADC_IN1_Max < (*ADC_IN1_Buff_addrR)[i])  ADC_IN1_Max = (*ADC_IN1_Buff_addrR)[i];
			  /* 找出最小值 */
			  if(ADC_IN1_Min > (*ADC_IN1_Buff_addrR)[i])  ADC_IN1_Min = (*ADC_IN1_Buff_addrR)[i];	
			  /* 求和 */
			  ADC_IN1_Sum += (*ADC_IN1_Buff_addrR)[i];
		  }
		  
		  /* 信号直流耦合最大值 */
		  tempCal = ADC_IN1_Max;
		  ADC_IN1.Vmax_DC = tempCal * ADC_IN1_Max_Vol / 4096.f;
		  
		  /* 信号直流耦合最小值 */
		  tempCal = ADC_IN1_Min;
		  ADC_IN1.Vmin_DC = tempCal * ADC_IN1_Max_Vol / 4096.f;
		  
		  /* 峰峰值 */
		  ADC_IN1.Vpp = ADC_IN1.Vmax_DC - ADC_IN1.Vmin_DC;
		  
		  /* 信号直流耦合平均值 */
		  tempCal = ADC_IN1_Sum;
		  ADC_IN1.Vavg_DC = tempCal * ADC_IN1_Max_Vol / 4096.f / 500.f;
		  
		  /* 信号交流耦合最大值 */
		  ADC_IN1.Vmax_AC = ADC_IN1.Vmax_DC - ADC_IN1.Vavg_DC;
		  
		  /* 信号交流耦合最小值 */
		  ADC_IN1.Vmin_AC = ADC_IN1.Vmin_DC - ADC_IN1.Vavg_DC;
		   
		  /* 信号交流耦合平均值 */
		  ADC_IN1.Vavg_AC = ADC_IN1.Vmax_AC + ADC_IN1.Vmin_AC;
		  
	  }
	  if(StartAnalyseFlag2 == 1)
	  {
		  StartAnalyseFlag2 = 0;
		  
		  /* 数据切换 双重缓冲 */
		  if(ADC_IN2_Buff_addr == &ADC_IN2_Buff1) ADC_IN2_Buff_addrR = &ADC_IN2_Buff2;
		  else ADC_IN2_Buff_addrR = &ADC_IN2_Buff1;
		  
		  /* 变量赋值 */
		  ADC_IN2_Min = (*ADC_IN2_Buff_addrR)[0];
		  ADC_IN2_Max = 0;
		  ADC_IN2_Sum = 0;
		  
		  /* 数据处理 每500个处理一次*/
		  for(uint32_t i=0;i<(sizeof(*ADC_IN2_Buff_addrR)/sizeof(uint16_t));i++) 
		  {
			  /* 找出最大值 */
			  if(ADC_IN2_Max < (*ADC_IN2_Buff_addrR)[i])  ADC_IN2_Max = (*ADC_IN2_Buff_addrR)[i];
			  /* 找出最小值 */
			  if(ADC_IN2_Min > (*ADC_IN2_Buff_addrR)[i])  ADC_IN2_Min = (*ADC_IN2_Buff_addrR)[i];	
			  /* 求和 */
			  ADC_IN2_Sum += (*ADC_IN2_Buff_addrR)[i];
		  }
		  
        /* 信号直流耦合最大值 */
		  tempCal = ADC_IN2_Max;
		  ADC_IN2.Vmax_DC = tempCal * ADC_IN2_Max_Vol / 4096.f;
		  
		  /* 信号直流耦合最小值 */
		  tempCal = ADC_IN2_Min;
		  ADC_IN2.Vmin_DC = tempCal * ADC_IN2_Max_Vol / 4096.f;
		  
		  /* 峰峰值 */
		  ADC_IN2.Vpp = ADC_IN2.Vmax_DC - ADC_IN2.Vmin_DC;
		  
		  /* 信号直流耦合平均值 */
		  tempCal = ADC_IN2_Sum;
		  ADC_IN2.Vavg_DC = tempCal * ADC_IN2_Max_Vol / 4096.f / 500.f;
		  
		  /* 信号交流耦合最大值 */
		  ADC_IN2.Vmax_AC = ADC_IN2.Vmax_DC - ADC_IN2.Vavg_DC;
		  
		  /* 信号交流耦合最小值 */
		  ADC_IN2.Vmin_AC = ADC_IN2.Vmin_DC - ADC_IN2.Vavg_DC;
		  
		  /* 信号交流耦合平均值 */
		  ADC_IN2.Vavg_AC = ADC_IN2.Vmax_AC + ADC_IN2.Vmin_AC;
		  
	  }
	  
	  
	  if((GetTick() - tick1) >= 1)  // 1ms
	  {
		  tick1 = GetTick();
		  if(StartAnalyseFlag1 == 0 && StartAnalyseFlag2 == 0)
		  {
		   // HAL_GPIO_TogglePin(D3_GPIO_Port,D3_Pin);
			if(Cheak_Start_Flag == 1)
			{
				switch(Cheak_Sum)
				{
					case Cheak_Amp_Rin :
					{
						uint32_t temp1;
						
						DelayCheakTick1 ++;
						if(DelayCheakTick1 >= 50)  // 1 * 50ms
						{
							DelayCheakTick1 = 0; // 此时为50ms
						}
						/* 合上2号继电器 */
						SWITCH_Set(2,1); 
						/* 合上3号继电器 放大10倍*/
						SWITCH_Set(3,1);
						DAC_FreqSet = 100; // 1KHz
					   temp1 = ADC_IN2.Vmax_AC;
						if(temp1 < 2000) // 2000 mV
						{
							if(DelayCheakTick1 == 0)
							{
								if(temp1 > 1500) DAC_P_VolSet += 1;
								else DAC_P_VolSet += 2;
							}
							if(DAC_P_VolSet > 100)
							{
								/* 关上3号继电器 防止过放*/
								 SWITCH_Set(3,0);
								 SWITCH_Set(2,0);
								 DAC_P_VolSet = 20;
								if(Cheak_Sum == Cheak_Amp_Rin)
								{
									
									SendCmd(Send_Base_Error,0x01); // 电路故障
									Cheak_Start_Flag = 0;
									Cheak_Sum = 0;
								}
								DelayCheakTick1 = 0;
							}								
							DelayCheakTick = 0;
							Cheak_temp = 0;
							Cheak_temp1 = 0;
						}
						else
						{
							if(DelayCheakTick >= (20-1)) // 等待 20 ms
							{
								Cheak_temp += ADC_IN2.Vmax_AC;  // 计算平均输出电压
								
								Cheak_temp1 += ADC_IN1.Vmax_AC; //  计算平均输入电压 
								//Cheak_temp1 = DAC_P_VolSet / 2.f ; // 计算平均输入电压 
								
								DelayCheakTick ++;
								if(DelayCheakTick >= (50 + 20-1))
								{
									/* 关上3号继电器 防止过放*/
								    SWITCH_Set(3,0);
									
									Amp_Vout_2000mV = Cheak_temp / 50.f / 1000.f;    // 平均输出电压 单位V
									
									Amp_Vin_2000mV = Cheak_temp1 / 50.f / 1000.f / 10.f;   // 平均输入电压 单位V
									//Amp_Vin_2000mV = Cheak_temp1 / 1000.f;  // 平均输入电压 单位V
									
									/* 计算三极管放大倍数 */
									BJT_BD = Amp_Vout_2000mV / Amp_Vin_2000mV ;
									/* 计算三极管内阻 */
									BJT_Rbe = 200 + (1 + BJT_BD) * 26 / Amp_Vout_2000mV * 2.f ;
									/* 计算放大器输入电阻 */
									Cheak_temp2 = (45000.f * 15000.f) * BJT_Rbe;
									Cheak_temp1 =  ((45000.f + 15000.f) * BJT_Rbe + (45000.f * 15000.f));
									Amp_Rin = Cheak_temp2 / Cheak_temp1;
									/* 清除变量 */
									
									DelayCheakTick = 0;
									Cheak_temp = 0;
									Cheak_temp1 = 0;
									Cheak_temp2 = 0;
									if(Cheak_Sum == Cheak_Amp_Rin)
										Cheak_Sum ++;
									else Cheak_Sum = 0xff;
								}
							}
							else 
								DelayCheakTick ++ ;
							//SET_BIT(Cheak_Set,Set_Rin_2000mV);
						}
					}
					break;
					case Cheak_Amp_Rout :
					{
						SWITCH_Set(4,1); // 合上4号继电器
						if(DelayCheakTick >= (100-1)) // 100ms
						{
							Cheak_temp += ADC_IN2.Vmax_AC;  // 计算平均输出电压
							DelayCheakTick ++ ;
							if(DelayCheakTick >= (100 + 100-1))
							{
								Amp_Vout_5KR = Cheak_temp / 100.f / 1000.f;  // 单位 V
								/* 未接负载时输出电流 */
								Cheak_temp1 = Amp_Vout_2000mV / Amp_OUT_50KR;
								/* 接5KΩ负载时输出电流 */
								Cheak_temp2 = Amp_Vout_5KR / Amp_OUT_5KR;
								/* Amplifier放大器输出电阻 */
								Amp_Rout = (Amp_Vout_2000mV - Amp_Vout_5KR) / (Cheak_temp2 - Cheak_temp1);
								/* 清除变量 */
								Cheak_temp = 0;
								Cheak_temp1 = 0;
								Cheak_temp1 = 0;
								DelayCheakTick = 0;
								DelayCheakTick1 = 0;
								/* 断开4号继电器 */
								SWITCH_Set(4,0); 
								
								if(Amp_Re_Cheak > 0)	// 重复 0+2 两次
								{
									if(Cheak_Sum == Cheak_Amp_Rout)
									{
										SendCmd(Send_Base_Rin,(int32_t)Amp_Rin*10);
							         SendCmd(Send_Base_Rout,(int32_t)Amp_Rout*10);
							         SendCmd(Send_Base_BJT_BD,(int32_t)BJT_BD*10);
										Cheak_Sum ++;
									}
									else Cheak_Sum = 0xff;
									//Cheak_Sum ++ ;
								}
								else
								{
									Amp_Re_Cheak ++;
									Cheak_Sum = 0;
								}
							}
							
						}
						else
						{
							DelayCheakTick ++ ;
						}				
					}
					break;
					case Cheak_Amp_FC:
					{
						if(DelayCheakTick >= (100-1)) // 100ms
						{
							DelayCheakTick ++;
							if(DelayCheakTick >= (20 + (100 - 1)))
							{
								DelayCheakTick = 100 -1;
								
								Cheak_temp += ADC_IN2.Vmax_AC;
								
								DelayCheakTick1 ++;
								if(DelayCheakTick1 > 2) // 循环3次
								{
									FVSave[FV_Count][0] = DAC_FreqSet;
									FVSave[FV_Count][1] = Cheak_temp/3.f;
									uint32_t x,y;
									x = FVSave[FV_Count][0];
									y = FVSave[FV_Count][1];
									SendCmd(Send_Base_FV,(x<<16)|(y&0x0000FFFF));
									Cheak_temp = 0;
									DelayCheakTick1 = 0;
								}
								else break;
								
								if(FV_Count < 5) // 4个点
								{
									DAC_FreqSet += 3;
								}
								else if(FV_Count < 10) // 5个点
								{
									ADC_FreqSet = 25000; // 25KHz 采样 
									DAC_FreqSet += 10;
								}
								else if(FV_Count < 15) // 5个点
								{
									ADC_FreqSet = 100000; // 100KHz 采样 
									DAC_FreqSet += 100; // 1khz
								}
								else if(FV_Count < 20) // 5个点
								{
									ADC_FreqSet = 400000; //400khz
									DAC_FreqSet += 3500; // 30khz
								}
								else
								{
									DAC_FreqSet += 100; // 1khz
//									if(FVSave[FV_Count - 1][1] < FVSave[FV_Count][1])
//									{
//										FVSave[FV_Count - 1][1] = (FVSave[FV_Count][1] + FVSave[FV_Count - 2][1])/2.f;
//									}
								}
								FV_Count++;
								/* 记录完退出 变量复位 */
								if(FV_Count > 39) 
								{
									FV_Count = 0;
									DelayCheakTick = 0;
									DelayCheakTick1 = 0;
									Cheak_temp = 0;
									DAC_P_VolSet = 25;
									DAC_FreqSet = 100; // 1000HZ
									ADC_FreqSet = 100000; //100khz
									if(Cheak_Sum == Cheak_Amp_FC)
									{
										SendCmd(Send_BaseTestOver,0x01);
										Cheak_Sum ++;
									}
									else Cheak_Sum = 0xff;
								}
								
							}
							
						}
						else
						{
							DAC_P_VolSet = 80; // 100mV Vmax = 50mV
							DAC_FreqSet = 5; // 50HZ
							ADC_FreqSet = 12500; // 低频12.5KHzADC采样速率 40ms 处理一次
							DelayCheakTick ++ ;
						}

						//Cheak_Amp_FC ++;
					}
					break;
					default:
						break;
				}
				
			}
			else if(Cheak_Start_Flag == 2)
			{
				if(DelayCheakTick1 > 2 )
				{
					DelayCheakTick1 ++;
					DAC_P_VolSet = 100;
					if(DelayCheakTick1 > 400)
					{
						DelayCheakTick1 = 0;
						if(ADC_IN2.Vpp > 6500)
						{
							if(ADC_IN2.Vpp < 7400)
							{
								
									Test_erro = R2_Kai;
									SendCmd(Send_Test_Error,R2_Kai);
									//Cheak_Start_Flag = 0;								
							}
						}
						
					}
				}
				else
				{
					DAC_P_VolSet = 3300;
					SWITCH_Set(3,0);
					SWITCH_Set(2,1);
					if(DelayCheakTick > 20)
					{
						//
						DelayCheakTick = 0;
						if(PWM_IN_H_Duty > 0.89f)
						{
							PWM_IN_H_Duty = 0;
							if((ADC_IN2.Vmax_AC < 1300) && (ADC_IN2.Vmax_AC > 950))
							{
								if(ADC_IN2.Vpp > 11450)
								{
									Test_erro = R1_Kai;
									SendCmd(Send_Test_Error,R1_Kai);
									//Cheak_Start_Flag = 0;
								}
							}
						}
						if(ADC_IN2.Vavg_DC > 11000)	
						{
							
							if(ADC_IN2.Vpp < 200)
							{
								if(ADC_IN1.Vpp < 2900 && ADC_IN1.Vpp > 2750)
								{
									if(ADC_IN2.Vmax_DC > 11900)
									{
									  Test_erro = R2_Duan;
									  SendCmd(Send_Test_Error,R2_Duan);
									  //Cheak_Start_Flag = 0;								
									}
									else if(ADC_IN2.Vmax_DC > 11700)
									{
									Test_erro = R1_Duan;
									SendCmd(Send_Test_Error,R1_Duan);
									//Cheak_Start_Flag = 0;
									}
								}
								else if(ADC_IN1.Vpp > 3150)
								{
									if(ADC_IN2.Vavg_DC > 12300)
									{
										Test_erro = R3_Duan;
										SendCmd(Send_Test_Error,R3_Duan);
										//Cheak_Start_Flag = 0;								
									}
								}
							}
							else if(ADC_IN2.Vpp > 8000)
							{
								 if(ADC_IN2.Vpp < 8600)
								 {
									 	Test_erro = R4_Kai;
										SendCmd(Send_Test_Error,R4_Kai);
										//Cheak_Start_Flag = 0;	
								 }
							}
						}
						else if(ADC_IN2.Vavg_DC > 10000)	
						{
							if(ADC_IN2.Vpp > 11500)
							{
								if(ADC_IN2.Vmin_DC == 0) 
								{
									Test_erro = R4_Duan;
									SendCmd(Send_Test_Error,R4_Duan);
									//Cheak_Start_Flag = 0;								
								}
							}
							else if(ADC_IN2.Vpp < 11300)
							{
								if(ADC_IN2.Vavg_DC < 10700)
								{
									DelayCheakTick1 ++ ;
									//Test_erro = R2_Kai;
									//SendCmd(Send_Test_Error,R2_Kai);
									//Cheak_Start_Flag = 0;
								}								
								
							}
						}
						else if(ADC_IN2.Vpp > 380)
						{
							if(ADC_IN2.Vpp < 500)
							{
								if(ADC_IN1.Vpp > 3180)
								{
									Test_erro = R3_Kai;
									SendCmd(Send_Test_Error,R3_Kai);
									//Cheak_Start_Flag = 0;
								}
							}
							if(ADC_IN2.Vpp > 6300)
							{
								if(ADC_IN2.Vavg_DC > 7100)
								{
									if(ADC_IN2.Vavg_DC < 7500)
									{
										Test_erro = C2_Kai;
										SendCmd(Send_Test_Error,C2_Kai);
										//Cheak_Start_Flag = 0;
									}
								}								
							}
						}
						else if(ADC_IN2.Vpp < 310)
						{
							if(ADC_IN2.Vpp > 200)
							{
								if(ADC_IN1.Vpp > 3200)
								{
									Test_erro = C1_Kai;
									SendCmd(Send_Test_Error,C1_Kai);
									//Cheak_Start_Flag = 0;
								}
							}							
						}
						
					}
					else DelayCheakTick ++;
				}

			}
			else 
			{
				DAC_P_VolSet = 25;
				SWITCH_Set(3,0);
				SWITCH_Set(2,0);
				DelayCheakTick = 0;
				DelayCheakTick1 = 0;
			}
		}
	  }
	
}

