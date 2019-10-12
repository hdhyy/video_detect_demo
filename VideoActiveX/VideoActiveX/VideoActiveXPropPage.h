#pragma once

// VideoActiveXPropPage.h: CVideoActiveXPropPage 属性页类的声明。


// CVideoActiveXPropPage : 请参阅 VideoActiveXPropPage.cpp 了解实现。

class CVideoActiveXPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CVideoActiveXPropPage)
	DECLARE_OLECREATE_EX(CVideoActiveXPropPage)

// 构造函数
public:
	CVideoActiveXPropPage();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_VIDEOACTIVEX };

// 实现
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 消息映射
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
};

