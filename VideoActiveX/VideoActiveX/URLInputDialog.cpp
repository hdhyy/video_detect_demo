// URLInputDialog.cpp: 实现文件
//

#include "pch.h"
#include "VideoActiveX.h"
#include "URLInputDialog.h"
#include "afxdialogex.h"


// URLInputDialog 对话框

IMPLEMENT_DYNAMIC(URLInputDialog, CDialogEx)

URLInputDialog::URLInputDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_URLINPUT_DIALOG, pParent)
	, m_input_url(_T(""))
{

}

URLInputDialog::~URLInputDialog()
{
}

void URLInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_URL_INPUT, m_input_url);
}


BEGIN_MESSAGE_MAP(URLInputDialog, CDialogEx)
END_MESSAGE_MAP()


// URLInputDialog 消息处理程序
CString URLInputDialog::getInputUrl()
{
	return m_input_url;
}