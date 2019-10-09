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
	, m_c_url_v(FALSE)
{

}

URLInputDialog::~URLInputDialog()
{
}

void URLInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_URL_INPUT, m_input_url);
	DDX_Control(pDX, IDC_BRAND, m_cb_brand);
	DDX_Control(pDX, IDC_URL_TYPE, m_cb_urltype);
	DDX_Control(pDX, IDC_STREAM_TYPE, m_cb_streamtype);
	DDX_Control(pDX, IDC_CODEC, m_cb_codec);
	DDX_Control(pDX, IDC_RADIO1, m_c_url);
	DDX_Radio(pDX, IDC_RADIO1, m_c_url_v);
}


BEGIN_MESSAGE_MAP(URLInputDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &URLInputDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &URLInputDialog::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &URLInputDialog::OnBnClickedRadio2)
END_MESSAGE_MAP()


// URLInputDialog 消息处理程序
CString URLInputDialog::getInputUrl()
{
	if (!m_c_url_v)
	{
		m_final_url = m_input_url;
	}
	else
	{

	}
	return m_final_url;
}

void URLInputDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


BOOL URLInputDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_cb_brand.AddString(_T("360"));   //添加字符串 
	m_cb_brand.AddString(_T("大华"));
	m_cb_brand.AddString(_T("海康"));
	m_cb_brand.AddString(_T("其他"));

	m_cb_urltype.AddString(_T("rtmp"));
	m_cb_urltype.AddString(_T("rtsp"));
	m_cb_urltype.AddString(_T("http"));

	m_cb_codec.AddString(_T("h264"));
	m_cb_codec.AddString(_T("MPEG-4"));
	m_cb_codec.AddString(_T("mpeg4"));

	m_cb_streamtype.AddString(_T("主码流"));
	m_cb_streamtype.AddString(_T("辅码流"));


	m_cb_brand.SetCurSel(0);   //默认选择第一个
	m_cb_urltype.SetCurSel(0);
	m_cb_codec.SetCurSel(0);
	m_cb_streamtype.SetCurSel(0);

	Control_gen_url(false);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void URLInputDialog::OnBnClickedRadio1()
{
	GetDlgItem(IDC_URL_INPUT)->EnableWindow(true);
	Control_gen_url(false);
}


void URLInputDialog::OnBnClickedRadio2()
{
	GetDlgItem(IDC_URL_INPUT)->EnableWindow(false);
	Control_gen_url(true);
}

void URLInputDialog::Control_gen_url(bool enable)
{
	GetDlgItem(IDC_BRAND)->EnableWindow(enable);
	GetDlgItem(IDC_URL_TYPE)->EnableWindow(enable);
	GetDlgItem(IDC_STREAM_TYPE)->EnableWindow(enable);
	GetDlgItem(IDC_USERNAME)->EnableWindow(enable);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(enable);
	GetDlgItem(IDC_CODEC)->EnableWindow(enable);
	GetDlgItem(IDC_CHANNALS)->EnableWindow(enable);
	GetDlgItem(IDC_URL_PORT)->EnableWindow(enable);
	GetDlgItem(IDC_URL_IP)->EnableWindow(enable);
}
