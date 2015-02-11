#include "stdafx.h"
#include "KBASE/DLLManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
// 构造函数
KDLLManager::KDLLManager(void)
:m_hDLL(NULL)
{
}

//---------------------------------------------------------------------------------------
// 析构函数
KDLLManager::~KDLLManager(void)
{
	UnLoad();
}

//---------------------------------------------------------------------------------------
// 动态调用DLL
bool KDLLManager::Load(const char* pDllName)
{
#ifdef WIN32
	USES_CONVERSION;
	m_hDLL = LoadLibrary(A2W(pDllName));
#else
	// flag: RTLD_LAZY	= 表明在动态链接库的函数代码执行时解决
	//       RTLD_NOW	= 表明在dlopen返回前就解决所有未定义的符号,一旦未解决,dlopen将返回错误
	//					  dlopen调用失败时,将返回NULL值,否则返回的是操作句柄.
	m_hDLL = dlopen(pDllName,RTLD_LAZY);
#endif
	if(m_hDLL==NULL)
	{
#ifdef WIN32
#else
		char* error=dlerror();
		printf("dlopen error:%s.\n",error);
#endif
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------------------
// 卸载DLL
void KDLLManager::UnLoad(void)
{
#ifdef WIN32
	if(m_hDLL)
	{
		FreeLibrary(m_hDLL);
	}
#else
	if(m_hDLL)
	{
		dlclose(m_hDLL);
	}
#endif
}

//---------------------------------------------------------------------------------------
// 获取函数指针
void* KDLLManager::GetFunction(const char* pFunctionName)
{
	void* pFunc = NULL;

	if(m_hDLL)
	{
#ifdef WIN32
		pFunc = (void*)GetProcAddress(m_hDLL,pFunctionName);
#else
		pFunc = dlsym(m_hDLL,pFunctionName);
		if(pFunc==NULL)
		{
			printf("dlsym %s error:%s.\n",pFunctionName,dlerror());
		}
#endif
	}
	return pFunc;
}
