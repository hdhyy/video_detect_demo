#pragma once
#include "MyDlg.h"

// VideoActiveXCtrl.h : CVideoActiveXCtrl ActiveX 控件类的声明。
#include <objsafe.h> 

// CVideoActiveXCtrl : 请参阅 VideoActiveXCtrl.cpp 了解实现。

class CVideoActiveXCtrl : public COleControl
{
	DECLARE_DYNCREATE(CVideoActiveXCtrl)

// 构造函数
public:
	CVideoActiveXCtrl();
	MyDlg m_MyDlg;
// 重写
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

// 实现
protected:
	~CVideoActiveXCtrl();

	DECLARE_OLECREATE_EX(CVideoActiveXCtrl)    // 类工厂和 guid
	DECLARE_OLETYPELIB(CVideoActiveXCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CVideoActiveXCtrl)     // 属性页 ID
	DECLARE_OLECTLTYPE(CVideoActiveXCtrl)		// 类型名称和杂项状态

// 消息映射
	DECLARE_MESSAGE_MAP()

// 调度映射
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// 事件映射
	DECLARE_EVENT_MAP()

// 调度和事件 ID
public:
	enum {
	};
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	// 接口映射 
	DECLARE_INTERFACE_MAP()
	BEGIN_INTERFACE_PART(ObjSafe, IObjectSafety)
		STDMETHOD_(HRESULT, GetInterfaceSafetyOptions) (
			REFIID riid,
			DWORD __RPC_FAR* pdwSupportedOptions,
			DWORD __RPC_FAR* pdwEnabledOptions
			);

	STDMETHOD_(HRESULT, SetInterfaceSafetyOptions) (
		REFIID riid,
		DWORD dwOptionSetMask,
		DWORD dwEnabledOptions
		);
	END_INTERFACE_PART(ObjSafe);
};

