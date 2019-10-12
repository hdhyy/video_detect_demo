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

//自定义
	MyDlg m_MyDlg;
protected:
	afx_msg STDMETHODIMP JsUseTest();
	afx_msg STDMETHODIMP OpenByWholeURL(BSTR url);
	//brand: 0:360 1:大华 2:海康 3：其他
	//protocol: 0:http 1:rtmp 2:rtsp
	//url_ip:IP地址
	//url_port:端口号
	//username：用户名
	//password：密码
	//channel：通道号
	//subtype：主码流为0，辅码流为1(ps:海康的第三码流则传3，以此类推)
	//old_version:海康需要传，默认为老版本
	//codec：海康需要传，0:h264 1:MPEG-4 2:mpeg4 (默认传0)
	afx_msg STDMETHODIMP OpenByNecessaryText(long brand, long protocol, BSTR url_ip, long url_port, BSTR username, BSTR password, long channel, long subtype, long old_verison=0, long codec = 0);

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
		dispidOPENURL = 1L,
		dispidJsUseTest = 2L,
		dispidOpenByNecessaryText = 3L
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
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual BOOL OnSetObjectRects(LPCRECT lpRectPos, LPCRECT lpRectClip);
};

