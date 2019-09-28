#include "ProtocolUART.h"

#ifndef LOBYTE
#define LOBYTE(l)           ((BYTE)(l))
#endif

#ifndef HIBYTE
#define HIBYTE(l)           ((BYTE)(l >> 8))
#endif

uint8_t getCheckSum(const uint8_t *pData, int len) {
	int sum = 0;
	for (int i = 0; i < len; ++i) {
		sum += pData[i];
	}

	//return (uint8_t) (~sum + 1);
		return (uint8_t)sum&0xff;
}

/**
 * ���ܣ�����Э��
 * ������pData Э�����ݣ�len ���ݳ���
 * ����ֵ��ʵ�ʽ���Э��ĳ���
 */

//uint8_t cheak = 0;

int parseProtocol(const BYTE *pData, UINT len) {
	UINT remainLen = len;	// ʣ�����ݳ���
	UINT dataLen;	// ���ݰ�����
	UINT frameLen;	// ֡����

	/**
	 * ���²�����Ҫ����Э���ʽ������Ӧ���޸ģ�������ÿһ֡������
	 */
	while (remainLen >= DATA_PACKAGE_MIN_LEN) {
		// �ҵ�һ֡���ݵ�����ͷ
		while ((remainLen >= 2) && ((pData[0] != CMD_HEAD1) || (pData[1] != CMD_HEAD2))) {
			pData++;
			remainLen--;

			continue;
		}

		if (remainLen < DATA_PACKAGE_MIN_LEN) {
			break;
		}
		
		dataLen = MAKEWORD(pData[3], pData[2]);
		if (dataLen != sizeof(SProtocolData)) {
			//LOGE("The data length is inconsistent!!!");
			//LOGE("dataLen:%d, frameLen:%d", remainLen - DATA_PACKAGE_MIN_LEN, sizeof(SProtocolData));
		}
		//dataLen = pData[4];
		frameLen = dataLen + DATA_PACKAGE_MIN_LEN;
		if (frameLen > remainLen) {
			// �������ݲ�ȫ
			break;
		}

		// ��ӡһ֡���ݣ���Ҫʱ��CommDef.h�ļ��д�DEBUG_PRO_DATA��
#ifdef DEBUG_PRO_DATA
		for (UINT i = 0; i < frameLen; ++i) {
			LOGD("%x ", pData[i]);
		}
		LOGD("\n");
#endif
		// ֧��checksumУ�飬��Ҫʱ��CommDef.h�ļ��д�PRO_SUPPORT_CHECK_SUM��
#ifdef PRO_SUPPORT_CHECK_SUM
		// ���У����
		if (getCheckSum(pData, frameLen - 1) == pData[frameLen - 1]) {
			// ����һ֡����
			procParse(pData, frameLen);
		} else {
			//LOGE("CheckSum error!!!!!!\n");
		}
#else
		// ����һ֡����
		procParse(pData, frameLen);
#endif

		pData += frameLen;
		remainLen -= frameLen;
	}

	return len - remainLen;
}

/**
 * ��Ҫ����Э���ʽ����ƴ�ӣ�����ֻ�Ǹ�ģ��
 */
BYTE dataBuf[DATA_LEN + DATA_PACKAGE_MIN_LEN];
uint8_t sendProtocol(BYTE ID,const BYTE *pData, int len) {
	
	// �ж����ݳ����Ƿ񳬳�����
	if (len > DATA_LEN) {
		//LOGE("The data length over the limit!!!");
		return 1;
	}

	dataBuf[0] = CMD_HEAD1;
	dataBuf[1] = CMD_HEAD2;			// ͬ��֡ͷ

	dataBuf[2] = HIBYTE(len);
	dataBuf[3] = LOBYTE(len);
	UINT frameLen = 4;

	// ����
	for (int i = 0; i < len; ++i) {
		dataBuf[frameLen] = pData[i];
		frameLen++;
	}

#ifdef PRO_SUPPORT_CHECK_SUM
	// У����
	//extern BYTE getCheckSum(const BYTE *pData, int len);
	dataBuf[frameLen] = getCheckSum(dataBuf, frameLen);
	frameLen++;
#endif
	protocolDataSend(ID,dataBuf,frameLen);
	return 0;
}
