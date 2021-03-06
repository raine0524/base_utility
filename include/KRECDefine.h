#pragma once
#include "HPRecordDefine.h"

#define TERM_TYPE_RECSVR	"remote"

typedef struct tag_RECREMOTE
{
	std::string			serverid;
	std::string			serverip;
	std::string			servernatip;
	std::string			storageaddr;
	unsigned int		policystorage;
	unsigned int		policytime;
	unsigned int		policysize;
	unsigned short		status;
	unsigned int		userport;
	unsigned int		licport;
	unsigned long		freespace;
	unsigned long		totalspace;

	tag_RECREMOTE()
	{
		serverid = "";
		serverip = "";
		servernatip = "";
		storageaddr = "";
		policystorage = 0;
		policytime = 60;
		policysize = 500;
		status = 0;
		userport = 0;
		licport = 0;
		freespace = 0;
		totalspace = 0;
	}
}RECREMOTE, *PRECREMOTE;

typedef enum 
{
	ENUM_RECFILE_DEL			= -1,
	ENUM_RECFILE_START  		= 1,
	ENUM_RECFILE_REC			= 2,
	ENUM_RECFILE_END			= 3,
	ENUM_RECFILE_EXCEPTION		= 4,
	ENUM_RECFILE_ERROR			= 5,
}RECFILESTATUS;

typedef enum
{
	EXCEPTION_RECSVR,
	EXCEPTION_CHANNEL,
	EXCEPTION_DEV,
	EXCEPTION_USER,
}RECFILEEXCEPTION;

typedef enum
{
	USERPORTOPT_ADD,
	USERPORTOPT_SUB
}USERPORTOPT;

typedef enum
{
	REC_AV,
	REC_CONF,
}REMOTE_REC_TYPE;

#define REC_TASK_NUM 32

typedef enum
{
    TASK_WAITING    = 0,
    TASK_PROCESSING = 1,
    TASK_STOP       = 2,
    TASK_FAILED     = 3,
    TASK_FINISHED   = 4,
}TASK_STATUS;

