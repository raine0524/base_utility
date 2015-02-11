/************************************************************************/
/*录像定义                                                            */
/************************************************************************/

#ifndef __RECORD_DEFINE_H__
#define __RECORD_DEFINE_H__

#ifdef WIN32
#define RECORD_EXPORT
#ifdef RECORD_EXPORT
#define RECORD_API _declspec(dllexport)
#elif RECORD_DLL
#define RECORD_API _declspec(dllimport)
#else
#define RECORD_API
#endif

#else
#define RECORD_API
#define TRACE  printf
#endif

#include <map>
#include <string>
using namespace std;

#define ERR_LIVE_FAILED					70110		//连接Nginx服务失败;
#define ERR_LIVE_PAUSE					70111		//因视频编码有误，不能直播;
#define ERR_LIVE_FINISH					70112		//直播结束;

typedef enum _RECERR_CODE
{
	REC_START_SUCCESS,			//录像开始成功
	REC_START_RECMAXCNT,		//达到授权数量
	REC_STREAM_FAILED,			//接流失败
	REC_START_EXIST,			//已经在录
	REC_START_DISKFULL,			//磁盘空间不足
	REC_START_NODISE,			//没有磁盘
	REC_CREATEFILE_FAILED,		//创建文件失败
	REC_ONLYCAN_CHLREC,			//只能单路录像（后台设置）
	REC_ONLYCAN_CONFREC,		//只能会议录像（后台设置）
	REC_START_OTHER,			//其它错误
}RECERR_CODE;

//存储模式
typedef enum _RECSAVE_TYPE
{
	REC_SAVE_LOCAL,				//本地存储
	REC_SAVE_SAN,				//SAN存储区域网络
	REC_SAVE_DAS,				//DAS直联存储
}RECSAVE_TYPE;

//录像模式
typedef enum _RECMODE_TYPE
{
	REC_MODE_NORMAL,			//正常录像,磁盘不足停止
	REC_MODE_CYCLE,				//覆盖录像,磁盘不足则删除			
}RECMODE_TYPE;

//录像启动方式
typedef enum _RECSTART_TYPE
{
	RECSTART_TYPE_NORMAL,	//正常手动录像
	RECSTART_TYPE_ALARM,	//警报录像
	RECSTART_TYPE_PLAN,		//计划录像
	RECSTART_TYPE_CONF,		//会议录像
}RECSTART_TYPE;

typedef enum _RECDEL_TYPE
{
	RECDEL_TYPE_SIGNED,	//数据库标志的需要删除的
	RECDEL_TYPE_EARLY,	//磁盘满需要删除最早一天的
}RECDEL_TYPE;

typedef enum _RECSTOP_TYPE
{
	RECSTOP_TYPE_NORMAL,	//正常手动停止
	RECSTOP_TYPE_CUT,		//分割停止
	RECSTOP_TYPE_DISKFULL,	//磁盘满停止
	RECSTOP_TYPE_OTHER,		//其他方式停止
}RECSTOP_TYPE;

typedef enum _RECOVER_RESULT
{
	RECOVER_RESULT_NOFILE,		//文件不存在
	RECOVER_RESULT_NOTACTIVE,	//文件非法
	RECOVER_RESULT_COMPLITE,	//完成
	RECOVER_RESULT_NORMALAVC,	//正常无需修复
}RECOVER_RESULT;

//直播流方式;
typedef enum _STREAM_LIVE_TYPE
{
	STREAM_LIVE_TYPE_DEFAULT		= 0,	//不直播;		
	STREAM_LIVE_TYPE_ALL			= 1,	//直播音视频;
	STREAM_LIVE_TYPE_VIDEO			= 2,	//直播视频;
	STREAM_LIVE_TYPE_AUDIO			= 3,	//直播音频;
}STREAM_LIVE_TYPE;

//录像文件类型;
typedef enum _REC_FILE_TYPE
{
	REC_FILE_TYPE_MP4			= 1,		//mp4类型;
	REC_FILE_TYPE_FLV			= 2,		//flv类型;
	REC_FILE_TYPE_AVMS			= 3,		//avms类型;
	REC_FILE_TYPE_MKV			= 4,		//mkv类型;
	REC_FILE_TYPE_AVI			= 5,		//avi类型;
}REC_FILE_TYPE;


typedef enum _REC_TASK_STATUS
{
	REC_TASK_STATUS_NOR			= 0,		//未执行;
	REC_TASK_STATUS_CMMMIT		= 1,		//提交执行;
	REC_TASK_STATUS_PROCESS		= 2,		//执行中;
}REC_TASK_STATUS;

typedef struct _TASK_STREAM
{
	std::string		strSourceID;
	std::string		strDevID;
	unsigned short	usSourceType;
	unsigned short	ustreamType;		
	int				nWndowIndex;
	void*			pStream;

	std::string		nodeid;				
	std::string		mcuid;
	std::string		mcuaddr;
	unsigned short	mcuport;
	std::string		nataddr;
	std::string		localaddr;
	unsigned short  localport;
	std::string		termtype;
	unsigned long	videocid;
	unsigned long	audiocid;

	_TASK_STREAM& operator=(const _TASK_STREAM& mtn)
	{
		strSourceID = mtn.strSourceID;
		strDevID	= mtn.strDevID;
		usSourceType= mtn.usSourceType;
		ustreamType = mtn.ustreamType;
		nWndowIndex	= mtn.nWndowIndex;
		pStream		= mtn.pStream;

		nodeid		= mtn.nodeid;
		mcuid		= mtn.mcuid;
		mcuaddr		= mtn.mcuaddr;
		mcuport		= mtn.mcuport;
		nataddr		= mtn.nataddr;
		localaddr	= mtn.localaddr;
		localport	= mtn.localport;
		termtype	= mtn.termtype;
		videocid	= mtn.videocid;
		audiocid	= mtn.audiocid;
		
		return *this;
	}

	_TASK_STREAM()
	{
		strSourceID = "";
		strDevID	= "";
		usSourceType= 0;
		ustreamType = 0;
		nWndowIndex	= -1;
		pStream		= NULL;
		nodeid		= "";		
		mcuid		= "";
		mcuaddr		= "";
		mcuport		= 0;
		nataddr		= "";
		localaddr	= "";
		localport	= 0;
		termtype	= "";
		videocid	= 0;
		audiocid	= 0;
	}
}TASK_STREAM, *PTASK_STREAM;


typedef std::map<string, TASK_STREAM>		MAP_STREAM;

typedef struct _tagTASKINFO
{
	_tagTASKINFO()
	{
		strTaskid	= "";
		taskname	= "";
		bSimpleStream = true;
		bUseHigh	= false;
		uHighWidth	= 1024;
		highSize	= "1280X720";
		bUseMid		= false;
		uMidWidth	= 512;
		MidSize		= "640X480";
		bPublic		= false;
		bLive		= false;
		bRec		= false;
		uTMid		= 1;
		uTmWndwCnt	= 1;

	}
	_tagTASKINFO& operator=(const _tagTASKINFO& mtn)
	{
		strTaskid		= mtn.strTaskid;
		taskname	= mtn.taskname;
		bSimpleStream = mtn.bSimpleStream;
		bUseHigh	= mtn.bUseHigh;
		uHighWidth	= mtn.uHighWidth;
		highSize	= mtn.highSize;
		bUseMid		= mtn.bUseMid;
		uMidWidth	= mtn.uMidWidth;
		MidSize		= mtn.MidSize;
		bPublic		= mtn.bPublic;
		bLive		= mtn.bLive;
		bRec		= mtn.bRec;
		uTMid		= mtn.uTMid;
		uTmWndwCnt	= mtn.uTmWndwCnt;

		return *this;
	}

	std::string		strTaskid;
	std::string		taskname;
	bool			bSimpleStream;
	bool			bUseHigh;
	unsigned int	uHighWidth;
	std::string		highSize;
	bool			bUseMid;
	unsigned int	uMidWidth;
	std::string		MidSize;
	bool			bPublic;
	bool			bLive;
	bool			bRec;
	unsigned int	uTMid;
	unsigned int	uTmWndwCnt;

}REC_TASKINFO,*LPREC_TASKINFO;





#endif


