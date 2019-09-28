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

/* 发送命令 */
#define Send_BaseTestOver  (0x01)   // 基本要求测试完成
#define Send_Base_Rin	  (0x02)   // 发送输入电阻
#define Send_Base_Rout	  (0x03)   // 发送输出电阻
#define Send_Base_BJT_BD  (0x04)   // 发送增益
#define Send_Base_FV       (0x05)  // 发送幅频
#define Send_Base_Error    (0x06)  // 发送基本测试失败 
#define Send_Test_Error    (0x07)  // 发挥部分错误代码
#define Send_ConTest      (0xFF)  // 通信成功
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

/* 接收命令 */
#define Rec_Base_test  (0x01)  // 开始基础测试
#define Rec_ReBase_test (0x02)  // 重新开始基础要求测试
#define Rec_Test_Cheak (0x03)  // 故障检测 发挥部分

#define Rec_MCU_rst          (0xFE)
#define Rec_test   			  0xFF

void cmd_init(void);
void cmd_run(void);
void SendCmd(uint8_t CMD,uint32_t Data);
#ifdef __cplusplus
}
#endif

#endif
