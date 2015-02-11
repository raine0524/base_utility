/* 
*基于H264+AAC发送rtmp包接口类

*注意：发送之前最好请求一下关键帧,且发送数据是去除华平头的裸数据
*视频和音频流库内部加锁了同一线程发送
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
	//nResult =0 连接成功，1：连接失败 2：断开连接
	virtual void OnPubishResut(int nResult,const std::string& strUrl,const std::string& strServerIP)=0;
};
class RTMP_API IHPRtmpStream
{
public:
	IHPRtmpStream(){};
	virtual ~IHPRtmpStream(){};

	/******************************************************************************
	* SetLiveServerInfo
	*描述：设置直播服务器信息
	*输入：
	ServerMap		-直播服务器地址,至少一个地址key为连接的优先顺序key必须按顺序0,1,2,3 排列
	strAppName	    -直播服务器APP，对应直播系统中APP配置
	uPort			-直播端口
	直播输入的URT:rtmp://Server:uPort/strAppName/strChannelName
	其中uport 默认1935 ；strAppName 默认为live ;strChannelName不支持中文
	*输出： 无
	*返回值：无
	*****************************************************************************/
	virtual void SetLiveServerInfo(const map<int,std::string>& ServerMap,const std::string& strAppName,unsigned int uPort, const std::string strChannelName) =0;


	/******************************************************************************
	* Connect
	*描述：启动线程，只调用一次，启动后主动去发布直播流，若断开内部会循环重连直播服务器
	*输入：无
	*输出： true为成功，false为失败
	*****************************************************************************/
	virtual bool Connect()=0;


	/******************************************************************************
	* ResetServer
	*描述：重新设置直播服务器，若与之前没有变化则不执行断开重连，主要提供过直播服务切换用
	*输入：无
	*输出：无
	*****************************************************************************/
	virtual void ResetServer(const std::string& strServerIP)=0;


	/******************************************************************************
	* ResetServer
	*描述：重新连接发布流，外部检测到分辨率变化后需要调用此方法
	*输入：无
	*输出：无
	*****************************************************************************/
	virtual void ReConnect()=0;


	/******************************************************************************
	* SendVideoData
	*描述：PublishLiveStream成功后调用，目前只支持发送H264的裸流
	*输入：
	data			-H264视频数据
	len				-H264视频数据长度
	uTimeStamp		-视频数据时间戳
	bKeyFrame		-是否是关键帧
	*返回值：成功为true,数据发送失败
	*****************************************************************************/
	virtual bool SendVideoData(unsigned char*data,unsigned int len,unsigned long uTimeStamp,bool bKeyFrame)=0;


	/******************************************************************************
	* SetAudioSpec
	*描述：设置音频的SPEC为了解码用,音频数据发送前必须设置，否则使用写死的默认值
			body[0] = 0xAF;
			body[1] = 0x00;
			body[2] = 0x12;
			body[3] = 0x10;
			采样率：44100 通道数2
	       此值应该是编码端通过faacEncGetDecoderSpecificInfo获取到的
	*输入：
	data			-AAC音频数据
	len				-AAC音频数据长度，应该是4bytes
	uTimeStamp		-音频数据时间戳
	*返回值：成功为true,数据发送失败
	*****************************************************************************/
	virtual void SetAudioSpec(unsigned char*data,unsigned int len)=0;


	/******************************************************************************
	* SendAudioData
	*描述：PublishLiveStream成功后调用，目前只支持发送AAC编码的裸流
	*输入：
	data			-AAC音频数据
	len				-AAC音频数据长度
	uTimeStamp		-音频数据时间戳
	*返回值：成功为true,数据发送失败
	*****************************************************************************/
	virtual bool SendAudioData(unsigned char*data,unsigned int len,unsigned long uTimeStamp)=0;
	

	/******************************************************************************
	* Close
	*描述：停止直播
	*输入：无
	*返回值：无
	*****************************************************************************/
	virtual std::string GetStreamURL() =0;


	/******************************************************************************
	* Release
	*描述：全部销毁回收资源外部delte此类
	*输入：无
	*返回值：无
	*****************************************************************************/
	virtual void Release() =0;


	/******************************************************************************
	* Create
	*描述：创建直播对象
	*输入：	无
	*输出： 无
	*返回值：成功返回IHPRtmpStream的指针，失败返回NULL
	*****************************************************************************/
	static IHPRtmpStream* Create(IHPRtmpStreamNotify& rNotity);
};

#endif