// URLInputDialog.cpp: 实现文件
//

#include "stdafx.h"
#include "VideoDemo.h"
#include "URLInputDialog.h"
#include "afxdialogex.h"


// URLInputDialog 对话框

IMPLEMENT_DYNAMIC(URLInputDialog, CDialogEx)

URLInputDialog::URLInputDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT_URL, pParent)
	, m_input_url_text(_T(""))
{

}

URLInputDialog::~URLInputDialog()
{
}

void URLInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URLINPUT, m_url_input);
	DDX_Text(pDX, IDC_URLINPUT, m_input_url_text);
}


BEGIN_MESSAGE_MAP(URLInputDialog, CDialogEx)
END_MESSAGE_MAP()


// URLInputDialog 消息处理程序
CString URLInputDialog::getInputUrl()
{
	return m_input_url_text;
}