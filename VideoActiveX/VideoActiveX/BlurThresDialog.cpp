// BlurThresDialog.cpp: 实现文件
//

#include "pch.h"
#include "VideoActiveX.h"
#include "BlurThresDialog.h"
#include "afxdialogex.h"


// BlurThresDialog 对话框

IMPLEMENT_DYNAMIC(BlurThresDialog, CDialogEx)

BlurThresDialog::BlurThresDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BLUR_THRESHOLD_DIALOG, pParent)
{

}

BlurThresDialog::~BlurThresDialog()
{
}

void BlurThresDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cb_blur_thres);
}


BEGIN_MESSAGE_MAP(BlurThresDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &BlurThresDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// BlurThresDialog 消息处理程序


BOOL BlurThresDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_cb_blur_thres.AddString(_T("1.0"));
	m_cb_blur_thres.AddString(_T("1.5"));
	m_cb_blur_thres.AddString(_T("2.0"));
	m_cb_blur_thres.AddString(_T("2.5"));
	m_cb_blur_thres.AddString(_T("3.0"));
	m_cb_blur_thres.AddString(_T("3.5"));
	m_cb_blur_thres.AddString(_T("4.0"));
	m_cb_blur_thres.AddString(_T("4.5"));
	m_cb_blur_thres.AddString(_T("5.0"));
	m_cb_blur_thres.AddString(_T("5.5"));
	m_cb_blur_thres.AddString(_T("6.0"));

	m_cb_blur_thres.SetCurSel(6);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void BlurThresDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	int thresindex = m_cb_blur_thres.GetCurSel();
	threshold = (thresindex + 2) * 0.5;
	CDialogEx::OnOK();
}
