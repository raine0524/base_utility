/* 
*����H264+AAC����rtmp���ӿ���

*ע�⣺����֮ǰ�������һ�¹ؼ�֡,�ҷ���������ȥ����ƽͷ��������
*��Ƶ����Ƶ�����ڲ�������ͬһ�̷߳���
*
*
*creat 201-06-03 by lubaojun
*/ 

//RTMP_.h
#ifndef __HP_H264AAC_RTMP_H__
#define __HP_H264AAC_RTMP_H__

#include "IRtmpStreamDefine.h"


class IHPRtmpStreamNotify
{
public:
	//nResult =0 ���ӳɹ���1������ʧ�� 2���Ͽ�����
	virtual void OnPubishResut(int nResult,const std::string& strUrl,const std::string& strServerIP)=0;
};
class RTMP_API IHPRtmpStream
{
public:
	IHPRtmpStream(){};
	virtual ~IHPRtmpStream(){};

	/******************************************************************************
	* SetLiveServerInfo
	*����������ֱ����������Ϣ
	*���룺
	ServerMap		-ֱ����������ַ,����һ����ַkeyΪ���ӵ�����˳��key���밴˳��0,1,2,3 ����
	strAppName	    -ֱ��������APP����Ӧֱ��ϵͳ��APP����
	uPort			-ֱ���˿�
	ֱ�������URT:rtmp://Server:uPort/strAppName/strChannelName
	����uport Ĭ��1935 ��strAppName Ĭ��Ϊlive ;strChannelName��֧������
	*����� ��
	*����ֵ����
	*****************************************************************************/
	virtual void SetLiveServerInfo(const map<int,std::string>& ServerMap,const std::string& strAppName,unsigned int uPort, const std::string strChannelName) =0;


	/******************************************************************************
	* Connect
	*�����������̣߳�ֻ����һ�Σ�����������ȥ����ֱ���������Ͽ��ڲ���ѭ������ֱ��������
	*���룺��
	*����� trueΪ�ɹ���falseΪʧ��
	*****************************************************************************/
	virtual bool Connect()=0;


	/******************************************************************************
	* ResetServer
	*��������������ֱ��������������֮ǰû�б仯��ִ�жϿ���������Ҫ�ṩ��ֱ�������л���
	*���룺��
	*�������
	*****************************************************************************/
	virtual void ResetServer(const std::string& strServerIP)=0;


	/******************************************************************************
	* ResetServer
	*�������������ӷ��������ⲿ��⵽�ֱ��ʱ仯����Ҫ���ô˷���
	*���룺��
	*�������
	*****************************************************************************/
	virtual void ReConnect()=0;


	/******************************************************************************
	* SendVideoData
	*������PublishLiveStream�ɹ�����ã�Ŀǰֻ֧�ַ���H264������
	*���룺
	data			-H264��Ƶ����
	len				-H264��Ƶ���ݳ���
	uTimeStamp		-��Ƶ����ʱ���
	bKeyFrame		-�Ƿ��ǹؼ�֡
	*����ֵ���ɹ�Ϊtrue,���ݷ���ʧ��
	*****************************************************************************/
	virtual bool SendVideoData(unsigned char*data,unsigned int len,unsigned long uTimeStamp,bool bKeyFrame)=0;


	/******************************************************************************
	* SetAudioSpec
	*������������Ƶ��SPECΪ�˽�����,��Ƶ���ݷ���ǰ�������ã�����ʹ��д����Ĭ��ֵ
			body[0] = 0xAF;
			body[1] = 0x00;
			body[2] = 0x12;
			body[3] = 0x10;
			�����ʣ�44100 ͨ����2
	       ��ֵӦ���Ǳ����ͨ��faacEncGetDecoderSpecificInfo��ȡ����
	*���룺
	data			-AAC��Ƶ����
	len				-AAC��Ƶ���ݳ��ȣ�Ӧ����4bytes
	uTimeStamp		-��Ƶ����ʱ���
	*����ֵ���ɹ�Ϊtrue,���ݷ���ʧ��
	*****************************************************************************/
	virtual void SetAudioSpec(unsigned char*data,unsigned int len)=0;


	/******************************************************************************
	* SendAudioData
	*������PublishLiveStream�ɹ�����ã�Ŀǰֻ֧�ַ���AAC���������
	*���룺
	data			-AAC��Ƶ����
	len				-AAC��Ƶ���ݳ���
	uTimeStamp		-��Ƶ����ʱ���
	*����ֵ���ɹ�Ϊtrue,���ݷ���ʧ��
	*****************************************************************************/
	virtual bool SendAudioData(unsigned char*data,unsigned int len,unsigned long uTimeStamp)=0;
	

	/******************************************************************************
	* Close
	*������ֱֹͣ��
	*���룺��
	*����ֵ����
	*****************************************************************************/
	virtual std::string GetStreamURL() =0;


	/******************************************************************************
	* Release
	*������ȫ�����ٻ�����Դ�ⲿdelte����
	*���룺��
	*����ֵ����
	*****************************************************************************/
	virtual void Release() =0;


	/******************************************************************************
	* Create
	*����������ֱ������
	*���룺	��
	*����� ��
	*����ֵ���ɹ�����IHPRtmpStream��ָ�룬ʧ�ܷ���NULL
	*****************************************************************************/
	static IHPRtmpStream* Create(IHPRtmpStreamNotify& rNotity);
};

#endif