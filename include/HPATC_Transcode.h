//HPATCCodec.h
#ifndef __HPATC_TRANSCODE_H__
#define __HPATC_TRANSCODE_H__

#include "HPATC_Export.h"


class HPATC_API HPATC_Transcode;
class HPATC_TranscodeCallback
{
public:
	virtual void OnHPATC_TranscodeCallbackPacketData(HPATC_Transcode*pTranscode,void*pPacketData,int nPacketLen,unsigned long ulTimestamp/*ԭ��Ƶ����ʱ��*/)=0;
};

//���ΪAAC���������ݣ�����Ϊ��AUDEC��Ƶ��ͷ����Ƶ����
class HPATC_API HPATC_Transcode 
{
public:
	HPATC_Transcode(void){};
	virtual~HPATC_Transcode(void){};
public:
	virtual int Open(int nSampleRate=44100,int nSamplesPerFrame=1024)=0;
	virtual void Close(void)=0;

	/*���룺pPacketData����AUDEC��Ƶ��ͷ����Ƶ���ݣ�nPacketLen��Ƶ�������ݳ��ȣ�
	 *�����0��ת��ɹ�����0��ת��ʧ�ܣ�
	 *�ص���AAC���������� */
	virtual int TranscodePacketData(void*pPacketData,int nPacketLen)=0;

	static HPATC_Transcode*Create(HPATC_TranscodeCallback&rCallback);
};


#endif