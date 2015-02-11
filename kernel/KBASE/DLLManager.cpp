#include "stdafx.h"
#include "KBASE/DLLManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
// ���캯��
KDLLManager::KDLLManager(void)
:m_hDLL(NULL)
{
}

//---------------------------------------------------------------------------------------
// ��������
KDLLManager::~KDLLManager(void)
{
	UnLoad();
}

//---------------------------------------------------------------------------------------
// ��̬����DLL
bool KDLLManager::Load(const char* pDllName)
{
#ifdef WIN32
	USES_CONVERSION;
	m_hDLL = LoadLibrary(A2W(pDllName));
#else
	// flag: RTLD_LAZY	= �����ڶ�̬���ӿ�ĺ�������ִ��ʱ���
	//       RTLD_NOW	= ������dlopen����ǰ�ͽ������δ����ķ���,һ��δ���,dlopen�����ش���
	//					  dlopen����ʧ��ʱ,������NULLֵ,���򷵻ص��ǲ������.
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
// ж��DLL
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
// ��ȡ����ָ��
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
