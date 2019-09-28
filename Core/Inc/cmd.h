#ifndef __cmd_H__
#define __cmd_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include <string.h>

#define MotionUP   0x01
#define MotionSTOP 0x00
#define MotionDOWN 0x02

/* �������� */
#define Send_BaseTestOver  (0x01)   // ����Ҫ��������
#define Send_Base_Rin	  (0x02)   // �����������
#define Send_Base_Rout	  (0x03)   // �����������
#define Send_Base_BJT_BD  (0x04)   // ��������
#define Send_Base_FV       (0x05)  // ���ͷ�Ƶ
#define Send_Base_Error    (0x06)  // ���ͻ�������ʧ�� 
#define Send_Test_Error    (0x07)  // ���Ӳ��ִ������
#define Send_ConTest      (0xFF)  // ͨ�ųɹ�
#define Send_XY			  (0x04|0x80)

#define R1_Duan  (0x01)
#define R1_Kai   (0x02)
#define R2_Duan  (0x03)
#define R2_Kai   (0x04)
#define R3_Duan  (0x05)
#define R3_Kai   (0x06)
#define R4_Duan  (0x07)
#define R4_Kai   (0x08)

#define C1_Kai   (0x12)
#define C2_Kai   (0x14)
#define C3_Kai   (0x16)

/* �������� */
#define Rec_Base_test  (0x01)  // ��ʼ��������
#define Rec_ReBase_test (0x02)  // ���¿�ʼ����Ҫ�����
#define Rec_Test_Cheak (0x03)  // ���ϼ�� ���Ӳ���

#define Rec_MCU_rst          (0xFE)
#define Rec_test   			  0xFF

void cmd_init(void);
void cmd_run(void);
void SendCmd(uint8_t CMD,uint32_t Data);
#ifdef __cplusplus
}
#endif

#endif
