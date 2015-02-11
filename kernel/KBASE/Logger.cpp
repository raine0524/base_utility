/* KLog.cpp 日志类
*/
#include "stdafx.h"
#include "KBASE/Logger.h"
#include "KBASE/AutoLock.h"
#include "KBASE/Utils.h"
#include "KBASE/DateTime.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define MAX_LOG_SIZE 200*1024*1024

KLogger* g_pLogger  = NULL;
int      g_ModuleId = 0;
FILE*    g_LogFile  = NULL;


#ifndef WIN32
int getProcessName(char* processName, int len)
{
    int count = 0;
    int nIndex = 0;
    char chPath[256] = {0};
    char cParam[100] = {0};
    char *cTem = chPath;
    int tmp_len;
 
    pid_t pId = getpid();
    sprintf(cParam, "/proc/%d/exe", pId);
    count = readlink(cParam, chPath, 256);
    if (count < 0 || count >= 256)
    {
        return -1;
    }
    else
    {
        nIndex = count - 1;
        for( ; nIndex >= 0; nIndex--)
        {
            if( chPath[nIndex] == '/')
            {
                nIndex++;
                cTem += nIndex;
                break; 
            }
        }
    }
    tmp_len = strlen(cTem);
    if (0 == tmp_len) 
    {
        return -1;
    }
    
    if (len <= tmp_len + 1) 
    {
        return -1;
    }
    
    strcpy(processName, cTem);
    
    return 0;
}
#endif

bool IsExist(std::string path)
{
#ifdef WIN32
	WIN32_FIND_DATAA wfd;
	bool rValue = 0 , res = false;
	HANDLE nFind = FindFirstFileA(path.c_str(),&wfd);

	if((nFind == INVALID_HANDLE_VALUE && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
       || (path.substr(1.1).compare(":")==0))
    {
	  res = true;
	}
    else
    {
	  res = false;
	}
	FindClose(nFind);
	return res;
#else
	if (access(path.c_str(),0) == 0)
    {
	  return true;
	}
    else
    {
	  return false;
	}
#endif
}

void CreateDirectoryR(std::string path)
{
	#ifdef WIN32
	char *key = "\\";
	#else
	char *key = "/";
	#endif
	{
		std::string parentPath = path.substr(0,path.find_last_of(key));
	
		if(!IsExist(parentPath)){
			CreateDirectoryR(parentPath);
		}
		#ifdef WIN32
		CreateDirectoryA(path.c_str(),NULL);
		#else
		mkdir(path.c_str(),0755);
	    #endif
	}
}

unsigned long get_file_size(const char *filename)  
{  

#ifdef WIN32
	return 0;
#else
	struct stat f_stat;
	if (stat(filename, &f_stat) == -1)
	{
		printf("get_file_size failed\r\n");
		return -1;
	}
	return (long)f_stat.st_size;
#endif
} 

// void InitFd()
// {	
// 	char filePath[MAX_PATH]={0};
// 
// 	KFormatDateTime dtNow;
// 	GetNow(dtNow);
// 	char pDayInfo[50];
// 	sprintf(pDayInfo , "%d-%d-%d",dtNow.GetYear(),dtNow.GetMonth(),dtNow.GetMDay());
// 
// 	{
// 	#ifdef WIN32
// 		GetModuleFileNameA(NULL, filePath, MAX_PATH);
// 		std::string LogDir("\\log\\avcond\\");
// 		std::string LogName("\\avconlog-");
// 		char *key = "\\";
// 	#else
// 		strcpy(filePath,"/opt/avcon/avcond");
// 		std::string LogDir("/log/avcond/");
// 		std::string LogName("/avconlog-");
// 		char *key = "/";
// 	#endif
// 		std::string p(filePath);
// 	#ifdef WIN32
// 		std::string processName = p.substr(p.find_last_of(key)+1,p.length());
// 		processName=processName.substr(0,processName.find_last_of(".")==-1?processName.length():processName.find_last_of("."));
// 		p = p.substr(0 , p.find_last_of(key)==0?p.length():p.find_last_of(key));
// 	#else
// 		char pProcessName[20];
// 		getProcessName(pProcessName,sizeof(pProcessName));
// 		std::string processName(pProcessName);
// 	#endif
// 		p += LogDir;
// 		p += processName;
// 		CreateDirectoryR(p);
// 		p += LogName;
// 		p += pDayInfo;
// 		p += ".txt";
// 		g_LogFile = fopen(p.c_str() , "w");
// 	}
// }


//---------------------------------------------------------------------------------------
KLogger::KLogger()
	:m_nLevel(LL_ERROR)
{
}

//---------------------------------------------------------------------------------------
KLogger::~KLogger()
{
}


void KLogger::InitFd()
{	
#ifndef WIN32
	m_strFileDir = "/opt/avcon/avcond/log/avcond/";
	char pProcessName[20];
	getProcessName(pProcessName,sizeof(pProcessName));
	std::string processName(pProcessName);
	m_strFileDir += processName + "/";
	CreateDirectoryR(m_strFileDir);	

	if(m_strFilePath.empty())
	{
		CreateNewFile();
	}
#endif
}

void KLogger::CreateNewFile()
{
	KFormatDateTime dtNow;
	GetNow(dtNow);
	char pLogFile[50];
	sprintf(pLogFile , "%04d%02d%02d-%02d%02d%02d.log",dtNow.GetYear(),dtNow.GetMonth(),dtNow.GetMDay(),dtNow.GetHour(),dtNow.GetMinute(),dtNow.GetSecond());
	m_strFilePath = m_strFileDir + pLogFile;
}

//---------------------------------------------------------------------------------------
// 初始化日志文件
bool KLogger::Open(LOG_LEVEL nLevel)
{
	//日志级别
	if(nLevel!=LL_NONE && nLevel!=LL_ERROR && nLevel!=LL_INFO && nLevel!=LL_DEBUG && nLevel!=LL_FILE)
	{
		return false;
	}

	m_nLevel = nLevel;
	InitFd();

	return true;
}

//---------------------------------------------------------------------------------------
// 关闭日志文件
void KLogger::Close()
{
	/*fclose(g_LogFile);*/
}

//---------------------------------------------------------------------------------------
void KLogger::SetLevel(LOG_LEVEL nLevel)
{
	if(nLevel!=LL_NONE && nLevel!=LL_ERROR && nLevel!=LL_INFO && nLevel!=LL_DEBUG && nLevel!=LL_FILE)
	{
		return;
	}
	m_nLevel = nLevel;
}

//---------------------------------------------------------------------------------------
LOG_LEVEL KLogger::GetLevel(void)
{
    return m_nLevel;
}

//---------------------------------------------------------------------------------------
// 写日志
void KLogger::Write(const char* fmt,va_list args)
{
	if(m_nLevel==LL_NONE)
	{
		return;
	}

    //合成日志信息
    KFormatDateTime dtNow;
	GetNow(dtNow);
	std::string strNow = "";
	dtNow.GetDateTime(strNow);

    char buffer1[2048]={0};
#ifdef WIN32
	_vsnprintf(buffer1,sizeof(buffer1),fmt,args);
#else
	vsnprintf(buffer1,sizeof(buffer1),fmt,args);
#endif
    char buffer2[4096]={0};
	snprintf(buffer2, sizeof(buffer2), "[%s] %s",strNow.c_str(),buffer1);

#ifdef _WINDOWS
	TRACE(buffer2);
#endif
    //日志输出到控制台
	printf(buffer2);
	fflush(stdout);
}

//---------------------------------------------------------------------------------------
//写日志到文件
void KLogger::WriteFile(const char* fmt,va_list args)
{
	if(m_nLevel==LL_NONE)
	{
		return;
	}

    KFormatDateTime dtNow;
	GetNow(dtNow);
	std::string strNow = "";
	dtNow.GetDateTime(strNow);

	char buffer1[2048]={0};
	char ServId[10];
#ifdef WIN32
	_vsnprintf(buffer1,sizeof(buffer1),fmt,args);
#else
	vsnprintf(buffer1,sizeof(buffer1),fmt,args);
#endif

	char buffer2[4096]={0};
	snprintf(buffer2, sizeof(buffer2), "[%s] %s",strNow.c_str(),buffer1);

	printf(buffer2);
	fflush(stdout);

#ifndef WIN32
	unsigned ulFileSize = get_file_size(m_strFilePath.c_str());
	if(ulFileSize > MAX_LOG_SIZE || ulFileSize < 0)
	{
		printf("file size is %d\r\n",ulFileSize);
		CreateNewFile();
	}

	FILE* pFile = fopen(m_strFilePath.c_str(),"a+");
	if(NULL == pFile)
	{
		printf("[KBASE] Open log file failed\r\n");
		return;
	}
	fprintf(pFile , "%s" , buffer2);
	fflush(pFile);
	fclose(pFile);
#endif
}

//---------------------------------------------------------------------------------------
bool LOG::START(LOG_LEVEL nLevel)
{
    if(g_pLogger==NULL)
    {
        g_pLogger = new KLogger();
        return g_pLogger->Open(nLevel);
    }
    return true;
}

//---------------------------------------------------------------------------------------
void LOG::STOP()
{
    if(g_pLogger)
    {
        g_pLogger->Close();
        delete g_pLogger;
        g_pLogger=NULL;
    }
}

//---------------------------------------------------------------------------------------
void LOG::SETLEVEL(LOG_LEVEL nLevel)
{
	if(g_pLogger)
	{
		g_pLogger->SetLevel(nLevel);
	}
}

//---------------------------------------------------------------------------------------
// DEBUG
void LOG::DBG(const char* fmt,...)
{
    if(g_pLogger)
    {
        LOG_LEVEL nLevel = g_pLogger->GetLevel();
        if(nLevel >= LL_DEBUG)
        {
	        va_list args;
	        va_start(args, fmt);
			if (nLevel == LL_FILE)
			{
				g_pLogger->WriteFile(fmt,args);
			}else{
				g_pLogger->Write(fmt,args);
			}
	        va_end(args);
        }
    }
}

//---------------------------------------------------------------------------------------
// INFO
void LOG::INF( const char* fmt,...)
{
    if(g_pLogger)
    {
        LOG_LEVEL nLevel = g_pLogger->GetLevel();
        if(nLevel >= LL_INFO)
        {
	        va_list args;
	        va_start(args, fmt);
			if (nLevel == LL_FILE)
			{
				g_pLogger->WriteFile(fmt,args);
			}else{
				g_pLogger->Write(fmt,args);
			}
	        va_end(args);
        }
    }
}

//---------------------------------------------------------------------------------------
// ERROR
void LOG::ERR(const char* fmt,...)
{
    if(g_pLogger)
    {
        LOG_LEVEL nLevel = g_pLogger->GetLevel();
        if(nLevel >= LL_ERROR)
        {
	        va_list args;
	        va_start(args, fmt);
			if (nLevel == LL_FILE)
			{
				g_pLogger->WriteFile(fmt,args);
			}else{
				g_pLogger->Write(fmt,args);
			}
			va_end(args);
        }
    }
}
