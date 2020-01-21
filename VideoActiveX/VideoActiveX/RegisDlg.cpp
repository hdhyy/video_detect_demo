// RegisDlg.cpp: 实现文件
//

#include "pch.h"
#include "VideoActiveX.h"
#include "RegisDlg.h"
#include "afxdialogex.h"


// RegisDlg 对话框

IMPLEMENT_DYNAMIC(RegisDlg, CDialogEx)

RegisDlg::RegisDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGIS_DIALOG, pParent)
{

}

RegisDlg::~RegisDlg()
{
}

void RegisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(RegisDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &RegisDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// RegisDlg 消息处理程序


void RegisDlg::OnBnClickedOk()
{
	// 注册成功则直接返回
	CDialogEx::OnOK();
}
