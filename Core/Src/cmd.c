#include "cmd.h"
#include "usart.h"
#include "ProtocolUART.h"

#include "ringbuff/ringbuff.h"
#include "adc_analyse.h"
SProtocolData uartrx[20];
SProtocolData uarttx[40];
ringbuff_t uartrxbuff;
ringbuff_t uarttxbuff;

__IO uint32_t txflag;
void SendCmd(uint8_t CMD,uint32_t Data);


/* 接收参数 */


/* 发送参数 */


void cmd_init(void)
{ 
	/* 串口发送接收环形缓冲器 */
	ringbuff_init(&uartrxbuff,uartrx,sizeof(uartrx));
	ringbuff_init(&uarttxbuff,uarttx,sizeof(uarttx));
	/* 开启串口空闲中断 ，不定长接收 */
	__HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1,uart1_rx_buffer,100);
	
	txflag = 1;
}


SProtocolData recd;
SProtocolData senduart;
/*
CMD串口处理函数
*/
uint8_t re;
void cmd_run(void)
{
	size_t rx_len; // 读到的长度
	size_t tx_len; // 读到的长度
	rx_len = ringbuff_read(&uartrxbuff,&recd,sizeof(recd));
	if(rx_len != 0)
	{
		switch(recd.cmdID)
		{
			case Rec_test:
				HAL_GPIO_TogglePin(D2_GPIO_Port,D2_Pin);
			   SendCmd(Send_ConTest,0x01);
				break;
			case Rec_Base_test:
				if(recd.data == 0x01)
				{
					HAL_NVIC_DisableIRQ(TIM5_IRQn);
					Cheak_Sum = 0;
					Cheak_Start_Flag = 1;
					Amp_Re_Cheak = 0;
				}
			break;
			case Rec_ReBase_test :
				if(recd.data == 0x01)
				{
					Cheak_Sum = 0;
				}
			break;
			case Rec_Test_Cheak:
			{
				if(recd.data == 0x00)
				{
					HAL_NVIC_DisableIRQ(TIM5_IRQn);
					Cheak_Start_Flag = 0;
				}
				else
				{
					HAL_NVIC_EnableIRQ(TIM5_IRQn);
					Cheak_Start_Flag = 2;
				}
			}
				break;
			case Rec_MCU_rst:
				HAL_NVIC_SystemReset();
				break;
			default:
				break;
			
		}
		
	}
	if(txflag == 1)
	{
		tx_len = ringbuff_read(&uarttxbuff,&senduart,sizeof(senduart));
		if(tx_len != 0)
		{
			re = sendProtocol(recd.cmdID,(BYTE *)&senduart, sizeof(senduart));
		}
	}
}

SProtocolData Send;
void SendCmd(uint8_t CMD,uint32_t Data) {
	
	Send.cmdID = CMD;
	Send.data = Data;
	ringbuff_write(&uarttxbuff,(const uint8_t *)&Send,sizeof(Send));
}

/**
 * 硬件发送函数 
 */
void protocolDataSend(BYTE ID,BYTE *pData, UINT len)
{
	txflag = 0;
	HAL_UART_Transmit_IT(&huart1,(uint8_t *)pData,len);
}



/**
 * 解析每一帧数据 解析协议回调函数
 */
void procParse(const BYTE *pData, UINT len) {
	
 	 SProtocolData Spr;
	 memcpy(&Spr, pData + DATA_PACKAGE_MIN_LEN-1, (sizeof(Spr)));
	 ringbuff_write(&uartrxbuff,(const uint8_t *)&Spr,sizeof(Spr));
	
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	 if(huart == &huart1)
	 {
		 txflag = 1;
	 }
}

