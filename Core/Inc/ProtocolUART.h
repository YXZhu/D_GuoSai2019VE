#ifndef __ProtocolUART_H__
#define __ProtocolUART_H__

#ifdef __cplusplus
 extern "C" {
#endif
	 
	 
#include "main.h"

#ifndef BYTE
typedef unsigned char	BYTE;
#endif
#ifndef UINT
typedef unsigned int	UINT;
#endif
#ifndef UINT16
typedef unsigned short  UINT16;
#endif

#ifndef MAKEWORD
#define MAKEWORD(low, high)		(((BYTE)(low)) | (((BYTE)(high)) << 8))
#endif


#ifndef TABLESIZE
#define TABLESIZE(table)    (sizeof(table)/sizeof(table[0]))
#endif

#define PRO_SUPPORT_CHECK_SUM

/* SynchFrame DataLen Data  */
/*     2Byte 	2Byte  N Byte   */
// ��CheckSum�������С����: 2 + 2 = 4

// ��CheckSum�������С����: 2 + 2 + 1 = 5
// ��CheckSum�������С����: 2 + 2 = 4

#ifdef PRO_SUPPORT_CHECK_SUM
#define DATA_PACKAGE_MIN_LEN		5
#else
#define DATA_PACKAGE_MIN_LEN		4
#endif

// ͬ��֡ͷ
#define CMD_HEAD1	0xFF
#define CMD_HEAD2	0x55
#define DATA_LEN 5	// ���ݿ����󳤶ȣ���Ҫ����2K

// ���ݿ鶨��(�û���Ч����)
#pragma pack(push,1)
typedef struct {
	// �û��Զ�������
	uint8_t    cmdID;		// ����id
	uint32_t   data;     // ����
} SProtocolData;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
	// �û��Զ�������
	uint16_t   X;		// ����id
	uint16_t   Y;     // ����
} Position_x_y;
#pragma pack(pop)

/**
 * Ӳ�����ͺ��� 
 */
extern void protocolDataSend(BYTE ID,BYTE *pData, UINT len);
/**
 * ����ÿһ֡���� Э������ص�����
 */
extern void procParse(const BYTE *pData, UINT len);

// Э��������� �ڴ��ڿ����ж�������
int parseProtocol(const BYTE *pData, UINT len);

// Э�鷢�ͺ���
uint8_t sendProtocol(BYTE ID,const BYTE *pData, int len);

#ifdef __cplusplus
}
#endif

#endif

