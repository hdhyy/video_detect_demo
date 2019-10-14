// VideoActiveXCtrl.cpp : CVideoActiveXCtrl ActiveX 控件类的实现。

#include "pch.h"
#include "framework.h"
#include "VideoActiveX.h"
#include "VideoActiveXCtrl.h"
#include "VideoActiveXPropPage.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CVideoActiveXCtrl, COleControl)

// 消息映射

BEGIN_MESSAGE_MAP(CVideoActiveXCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_CREATE()
	ON_WM_MOUSEHWHEEL()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

// 调度映射

BEGIN_DISPATCH_MAP(CVideoActiveXCtrl, COleControl)
	DISP_FUNCTION_ID(CVideoActiveXCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CVideoActiveXCtrl, "JsUseTest", dispidJsUseTest, JsUseTest, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CVideoActiveXCtrl, "OpenByWholeURL", dispidOPENURL, OpenByWholeURL, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CVideoActiveXCtrl, "OpenByNecessaryText", dispidOpenByNecessaryText, OpenByNecessaryText, VT_EMPTY, VTS_I4 VTS_I4 VTS_BSTR VTS_I4 VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_I4)
END_DISPATCH_MAP()

// 事件映射

BEGIN_EVENT_MAP(CVideoActiveXCtrl, COleControl)
END_EVENT_MAP()

// 属性页

// TODO: 根据需要添加更多属性页。请记住增加计数!
BEGIN_PROPPAGEIDS(CVideoActiveXCtrl, 1)
	PROPPAGEID(CVideoActiveXPropPage::guid)
END_PROPPAGEIDS(CVideoActiveXCtrl)

// 初始化类工厂和 guid

IMPLEMENT_OLECREATE_EX(CVideoActiveXCtrl, "MFCACTIVEXCONTRO.VideoActiveXCtrl.1",
	0x84ede05c,0xc758,0x4ef4,0xa2,0x77,0x0e,0xe9,0x94,0xfb,0xba,0x21)

// 键入库 ID 和版本

IMPLEMENT_OLETYPELIB(CVideoActiveXCtrl, _tlid, _wVerMajor, _wVerMinor)

// 接口 ID

const IID IID_DVideoActiveX = {0x7f39cbcf,0xb1c8,0x48c3,{0xa6,0xa5,0xf6,0x81,0x3b,0xe8,0x88,0x98}};
const IID IID_DVideoActiveXEvents = {0xa6295121,0x0a3a,0x485a,{0xa4,0xf5,0x25,0xb4,0x4f,0xe5,0xee,0xf2}};

// 控件类型信息

static const DWORD _dwVideoActiveXOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CVideoActiveXCtrl, IDS_VIDEOACTIVEX, _dwVideoActiveXOleMisc)

// CVideoActiveXCtrl::CVideoActiveXCtrlFactory::UpdateRegistry -
// 添加或移除 CVideoActiveXCtrl 的系统注册表项

BOOL CVideoActiveXCtrl::CVideoActiveXCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO:  验证您的控件是否符合单元模型线程处理规则。
	// 有关更多信息，请参考 MFC 技术说明 64。
	// 如果您的控件不符合单元模型规则，则
	// 必须修改如下代码，将第六个参数从
	// afxRegInsertable | afxRegApartmentThreading 改为 afxRegInsertable。

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_VIDEOACTIVEX,
			IDB_VIDEOACTIVEX,
			afxRegInsertable | afxRegApartmentThreading,
			_dwVideoActiveXOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

// CVideoActiveXCtrl::CVideoActiveXCtrl - 构造函数

CVideoActiveXCtrl::CVideoActiveXCtrl()
{
	InitializeIIDs(&IID_DVideoActiveX, &IID_DVideoActiveXEvents);
	// TODO:  在此初始化控件的实例数据。
}

// CVideoActiveXCtrl::~CVideoActiveXCtrl - 析构函数

CVideoActiveXCtrl::~CVideoActiveXCtrl()
{
	// TODO:  在此清理控件的实例数据。
}

// CVideoActiveXCtrl::OnDraw - 绘图函数

void CVideoActiveXCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& /* rcInvalid */)
{
	if (!pdc)
		return;

	// TODO:  用您自己的绘图代码替换下面的代码。
	//pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	//pdc->Ellipse(rcBounds);
	m_MyDlg.MoveWindow(rcBounds, true);
}

// CVideoActiveXCtrl::DoPropExchange - 持久性支持

void CVideoActiveXCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: 为每个持久的自定义属性调用 PX_ 函数。
}


// CVideoActiveXCtrl::OnResetState - 将控件重置为默认状态

void CVideoActiveXCtrl::OnResetState()
{
	COleControl::OnResetState();  // 重置 DoPropExchange 中找到的默认值

	// TODO:  在此重置任意其他控件状态。
}


// CVideoActiveXCtrl::AboutBox - 向用户显示“关于”框

void CVideoActiveXCtrl::AboutBox()
{
	CDialogEx dlgAbout(IDD_ABOUTBOX_VIDEOACTIVEX);
	dlgAbout.DoModal();
}

// IObjectSafety的接口映射 
BEGIN_INTERFACE_MAP(CVideoActiveXCtrl, COleControl)
	INTERFACE_PART(CVideoActiveXCtrl, IID_IObjectSafety, ObjSafe)
END_INTERFACE_MAP()


//////////////////////////////////// ///////////////////////////////////////// 
// IObjectSafety 成员函数 
// AddRef, Release, QueryInterface 
ULONG FAR EXPORT CVideoActiveXCtrl::XObjSafe::AddRef()
{
	METHOD_PROLOGUE(CVideoActiveXCtrl, ObjSafe)
		return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CVideoActiveXCtrl::XObjSafe::Release()
{
	METHOD_PROLOGUE(CVideoActiveXCtrl, ObjSafe)
		return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CVideoActiveXCtrl::XObjSafe::QueryInterface(
	REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(CVideoActiveXCtrl, ObjSafe)
		return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

const DWORD dwSupportedBits =
INTERFACESAFE_FOR_UNTRUSTED_CALLER |
INTERFACESAFE_FOR_UNTRUSTED_DATA;
const DWORD dwNotSupportedBits = ~dwSupportedBits;

/////////////////////////////////////////////////////// ////////////////////// 
// CXXXCtrl::XObjSafe::GetInterfaceSafetyOptions 
HRESULT STDMETHODCALLTYPE
CVideoActiveXCtrl::XObjSafe::GetInterfaceSafetyOptions(
	REFIID riid,
	DWORD __RPC_FAR* pdwSupportedOptions,
	DWORD __RPC_FAR* pdwEnabledOptions
)
{
	METHOD_PROLOGUE(CVideoActiveXCtrl, ObjSafe)

		HRESULT retval = ResultFromScode(S_OK);

	// 接口是否存在 
	IUnknown FAR* punkInterface;
	retval = pThis->ExternalQueryInterface(&riid, (void**)& punkInterface);
	if (retval != E_NOINTERFACE) // 接口存在 
	{
		punkInterface->Release(); // 释放引用 
	}

	// We support both kinds of safety and have always both set, 
	// regardless of interface. 
	*pdwSupportedOptions = *pdwEnabledOptions = dwSupportedBits;
	return retval; // E_NOINTERFACE if QI failed 
}

//////////////////////////////////////////////////////////// ///////////////// 
// CXXXCtrl::XObjSafe::SetInterfaceSafetyOptions 
HRESULT STDMETHODCALLTYPE
CVideoActiveXCtrl::XObjSafe::SetInterfaceSafetyOptions(
	REFIID riid,
	DWORD dwOptionSetMask,
	DWORD dwEnabledOptions
)
{
	METHOD_PROLOGUE(CVideoActiveXCtrl, ObjSafe)

		// 接口是否存在 
		IUnknown FAR* punkInterface;
	pThis->ExternalQueryInterface(&riid, (void**)& punkInterface);
	if (punkInterface) // 接口存在 
	{
		punkInterface->Release(); // 释放引用 
	}
	else // 接口不存在 
	{
		return ResultFromScode(E_NOINTERFACE);
	}
	// Can't set bits we don't support. 
	if (dwOptionSetMask & dwNotSupportedBits)
	{
		return ResultFromScode(E_FAIL);
	}

	// Can't set bits we do support to zero 
	dwEnabledOptions &= dwSupportedBits;
	// (We already know there are no extra bits in mask. ) 
	if ((dwOptionSetMask & dwEnabledOptions) != dwOptionSetMask)
	{
		return ResultFromScode(E_FAIL);
	}

	// Don't need to change anything since we're always safe. 
	return ResultFromScode(S_OK);
}


// CVideoActiveXCtrl 消息处理程序


int CVideoActiveXCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_MyDlg.Create(IDD_DIALOG1, this);  //初始化对话框
	return 0;
}

STDMETHODIMP CVideoActiveXCtrl::OpenByWholeURL(BSTR url)
{
	CString str(url);
	m_MyDlg.OpenByJSWholeURL(str);
	return S_OK;
}

STDMETHODIMP CVideoActiveXCtrl::OpenByNecessaryText(long brand, long protocol, BSTR url_ip, long url_port, BSTR username, BSTR password, long channel, long subtype, long old_verison, long codec)
{
	CString ip(url_ip);
	CString username_c(username);
	CString password_c(password);
	m_MyDlg.OpenByJSNecessaryText(brand,protocol,ip, url_port, username_c, password_c, channel, subtype, old_verison, codec);
	return S_OK;
}
STDMETHODIMP CVideoActiveXCtrl::JsUseTest()
{
	return S_OK;
}



void CVideoActiveXCtrl::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	COleControl::OnMouseHWheel(nFlags, zDelta, pt);
}


void CVideoActiveXCtrl::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	COleControl::OnWindowPosChanging(lpwndpos);

	// TODO: 在此处添加消息处理程序代码
	CWnd* pWnd = GetParent();
	while (pWnd->GetParent() != NULL)
	{
		pWnd->Invalidate();
		pWnd = pWnd->GetParent();
	}
}

void MyGetClippingCoordinates(LPCRECT pPosRect, LPCRECT pClipRect,
	LPRECT pIntersectRect, LPPOINT pOffsetPoint)
{
	int clipLeft = 0;
	int clipTop = 0;

	if ((pClipRect == NULL) || IsRectEmpty(pClipRect))
	{
		CopyRect(pIntersectRect, pPosRect);
	}
	else
	{
		IntersectRect(pIntersectRect, pPosRect, pClipRect);
		clipLeft = pClipRect->left;
		clipTop = pClipRect->top;
	}

	//pOffsetPoint->x = min(0, pPosRect->left - clipLeft);
	//pOffsetPoint->y = min(0, pPosRect->top - clipTop);
	//我改的
	pOffsetPoint->x = 0;//min(0, pPosRect->left - clipLeft);
	pOffsetPoint->y = 0;//min(0, pPosRect->top - clipTop);
}

BOOL CVideoActiveXCtrl::OnSetObjectRects(LPCRECT lpRectPos, LPCRECT lpRectClip)
{
	// return COleControl::OnSetObjectRects(lprcPosRect, lprcClipRect);
	ASSERT(lpRectPos != NULL);

	// Remember the position rectangle.
	m_rcPos = *lpRectPos;

	// Calculate complete rectangle, include the tracker if it is present.
	CRect rectPos = m_rcPos;
	if (m_bUIActive && m_pRectTracker != NULL)
	{
		// Save new clipping rectangle (for DestroyTracker).
		if (lpRectClip != NULL)
			m_pRectTracker->m_rectClip = *lpRectClip;

		// Adjust tracker rectangle to new dimensions.
		CRect rectTmp = rectPos;
		rectTmp.OffsetRect(-rectTmp.left, -rectTmp.top);
		m_pRectTracker->m_rect = rectTmp;

		// Adjust the "true" rectangle to include handles/hatching.
		UINT nHandleSize = m_pRectTracker->m_nHandleSize - 1;
		rectPos.InflateRect(nHandleSize, nHandleSize);
	}

	// Now clip the rectangle as appropriate.
	CRect rectClip;
	MyGetClippingCoordinates(rectPos, lpRectClip, rectClip, &m_ptOffset);

	// Move the outer window first, and then move the inner window.

	if (!m_bInPlaceSiteWndless)
	{
		CWnd* pWndOuter = GetOuterWindow();

		//BEGIN CHANGE.
		if (pWndOuter != NULL)
		{
			static CRect oldClipRect(0, 0, 0, 0);
			if (oldClipRect != rectClip)
				::MoveWindow(pWndOuter->m_hWnd,
					rectClip.left, rectClip.top,
					rectClip.Width(), rectClip.Height(),
					TRUE);
			oldClipRect = rectClip;
		}
		//END CHANGE.
		if (pWndOuter != this)
			MoveWindow(m_ptOffset.x, m_ptOffset.y,
				rectPos.Width(), rectPos.Height());
	}

	return TRUE;
	//return COleControl::OnSetObjectRects(lpRectPos, lpRectClip);
}
