// VideoActiveX.cpp: CVideoActiveXApp 和 DLL 注册的实现。

#include "pch.h"
#include "framework.h"
#include "VideoActiveX.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CVideoActiveXApp theApp;

const GUID CDECL _tlid = {0x150b4ea9,0x2752,0x4c00,{0x8c,0xdd,0x96,0xc1,0x89,0x46,0xd2,0xa8}};
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;



// CVideoActiveXApp::InitInstance - DLL 初始化

BOOL CVideoActiveXApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO:  在此添加您自己的模块初始化代码。
	}

	return bInit;
}



// CVideoActiveXApp::ExitInstance - DLL 终止

int CVideoActiveXApp::ExitInstance()
{
	// TODO:  在此添加您自己的模块终止代码。

	return COleControlModule::ExitInstance();
}



// DllRegisterServer - 将项添加到系统注册表

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}



// DllUnregisterServer - 将项从系统注册表中移除

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
