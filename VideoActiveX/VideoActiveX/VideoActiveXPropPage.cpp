// VideoActiveXPropPage.cpp : CVideoActiveXPropPage 属性页类的实现。

#include "pch.h"
#include "framework.h"
#include "VideoActiveX.h"
#include "VideoActiveXPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CVideoActiveXPropPage, COlePropertyPage)

// 消息映射

BEGIN_MESSAGE_MAP(CVideoActiveXPropPage, COlePropertyPage)
	ON_WM_MOUSEHWHEEL()
END_MESSAGE_MAP()

// 初始化类工厂和 guid

IMPLEMENT_OLECREATE_EX(CVideoActiveXPropPage, "MFCACTIVEXCONT.VideoActiveXPropPage.1",
	0x4adf350a,0x1618,0x4a21,0x91,0x91,0x1a,0xb8,0x0e,0xaf,0x61,0xa4)

// CVideoActiveXPropPage::CVideoActiveXPropPageFactory::UpdateRegistry -
// 添加或移除 CVideoActiveXPropPage 的系统注册表项

BOOL CVideoActiveXPropPage::CVideoActiveXPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_VIDEOACTIVEX_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, nullptr);
}

// CVideoActiveXPropPage::CVideoActiveXPropPage - 构造函数

CVideoActiveXPropPage::CVideoActiveXPropPage() :
	COlePropertyPage(IDD, IDS_VIDEOACTIVEX_PPG_CAPTION)
{
}

// CVideoActiveXPropPage::DoDataExchange - 在页和属性间移动数据

void CVideoActiveXPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}

// CVideoActiveXPropPage 消息处理程序


void CVideoActiveXPropPage::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	COlePropertyPage::OnMouseHWheel(nFlags, zDelta, pt);
}
