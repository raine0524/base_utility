/************************************************************************/
/*¼����                                                            */
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

#define ERR_LIVE_FAILED					70110		//����Nginx����ʧ��;
#define ERR_LIVE_PAUSE					70111		//����Ƶ�������󣬲���ֱ��;
#define ERR_LIVE_FINISH					70112		//ֱ������;

typedef enum _RECERR_CODE
{
	REC_START_SUCCESS,			//¼��ʼ�ɹ�
	REC_START_RECMAXCNT,		//�ﵽ��Ȩ����
	REC_STREAM_FAILED,			//����ʧ��
	REC_START_EXIST,			//�Ѿ���¼
	REC_START_DISKFULL,			//���̿ռ䲻��
	REC_START_NODISE,			//û�д���
	REC_CREATEFILE_FAILED,		//�����ļ�ʧ��
	REC_ONLYCAN_CHLREC,			//ֻ�ܵ�·¼�񣨺�̨���ã�
	REC_ONLYCAN_CONFREC,		//ֻ�ܻ���¼�񣨺�̨���ã�
	REC_START_OTHER,			//��������
}RECERR_CODE;

//�洢ģʽ
typedef enum _RECSAVE_TYPE
{
	REC_SAVE_LOCAL,				//���ش洢
	REC_SAVE_SAN,				//SAN�洢��������
	REC_SAVE_DAS,				//DASֱ���洢
}RECSAVE_TYPE;

//¼��ģʽ
typedef enum _RECMODE_TYPE
{
	REC_MODE_NORMAL,			//����¼��,���̲���ֹͣ
	REC_MODE_CYCLE,				//����¼��,���̲�����ɾ��			
}RECMODE_TYPE;

//¼��������ʽ
typedef enum _RECSTART_TYPE
{
	RECSTART_TYPE_NORMAL,	//�����ֶ�¼��
	RECSTART_TYPE_ALARM,	//����¼��
	RECSTART_TYPE_PLAN,		//�ƻ�¼��
	RECSTART_TYPE_CONF,		//����¼��
}RECSTART_TYPE;

typedef enum _RECDEL_TYPE
{
	RECDEL_TYPE_SIGNED,	//���ݿ��־����Ҫɾ����
	RECDEL_TYPE_EARLY,	//��������Ҫɾ������һ���
}RECDEL_TYPE;

typedef enum _RECSTOP_TYPE
{
	RECSTOP_TYPE_NORMAL,	//�����ֶ�ֹͣ
	RECSTOP_TYPE_CUT,		//�ָ�ֹͣ
	RECSTOP_TYPE_DISKFULL,	//������ֹͣ
	RECSTOP_TYPE_OTHER,		//������ʽֹͣ
}RECSTOP_TYPE;

typedef enum _RECOVER_RESULT
{
	RECOVER_RESULT_NOFILE,		//�ļ�������
	RECOVER_RESULT_NOTACTIVE,	//�ļ��Ƿ�
	RECOVER_RESULT_COMPLITE,	//���
	RECOVER_RESULT_NORMALAVC,	//���������޸�
}RECOVER_RESULT;

//ֱ������ʽ;
typedef enum _STREAM_LIVE_TYPE
{
	STREAM_LIVE_TYPE_DEFAULT		= 0,	//��ֱ��;		
	STREAM_LIVE_TYPE_ALL			= 1,	//ֱ������Ƶ;
	STREAM_LIVE_TYPE_VIDEO			= 2,	//ֱ����Ƶ;
	STREAM_LIVE_TYPE_AUDIO			= 3,	//ֱ����Ƶ;
}STREAM_LIVE_TYPE;

//¼���ļ�����;
typedef enum _REC_FILE_TYPE
{
	REC_FILE_TYPE_MP4			= 1,		//mp4����;
	REC_FILE_TYPE_FLV			= 2,		//flv����;
	REC_FILE_TYPE_AVMS			= 3,		//avms����;
	REC_FILE_TYPE_MKV			= 4,		//mkv����;
	REC_FILE_TYPE_AVI			= 5,		//avi����;
}REC_FILE_TYPE;


typedef enum _REC_TASK_STATUS
{
	REC_TASK_STATUS_NOR			= 0,		//δִ��;
	REC_TASK_STATUS_CMMMIT		= 1,		//�ύִ��;
	REC_TASK_STATUS_PROCESS		= 2,		//ִ����;
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


