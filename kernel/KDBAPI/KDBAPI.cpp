// KDBAPI.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"

#ifdef WIN32

#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE KDBAPIDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// ���ʹ�� lpReserved���뽫���Ƴ�
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("KDBAPI.DLL ���ڳ�ʼ����\n");
		
		// ��չ DLL һ���Գ�ʼ��
		if (!AfxInitExtensionModule(KDBAPIDLL, hInstance))
			return 0;

		// ���� DLL ���뵽��Դ����
		// ע��: �������չ DLL ��
		//  MFC ���� DLL(�� ActiveX �ؼ�)��ʽ���ӵ�
		//  �������� MFC Ӧ�ó������ӵ�������Ҫ
		//  �����д� DllMain ���Ƴ������������һ��
		//  �Ӵ���չ DLL �����ĵ����ĺ����С�ʹ�ô���չ DLL ��
		//  ���� DLL Ȼ��Ӧ��ʽ
		//  ���øú����Գ�ʼ������չ DLL������
		//  CDynLinkLibrary ���󲻻ḽ�ӵ�
		//  ���� DLL ����Դ���������������ص�
		//  ���⡣

		new CDynLinkLibrary(KDBAPIDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("KDBAPI.DLL ������ֹ��\n");

		// �ڵ�����������֮ǰ��ֹ�ÿ�
		AfxTermExtensionModule(KDBAPIDLL);
	}
	return 1;   // ȷ��
}

#endif