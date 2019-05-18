#include "stdafx.h"
#include "ROIThread.h"

// ROIThread
IMPLEMENT_DYNCREATE(ROIThread, CWinThread)

ROIThread::ROIThread()
{
}

ROIThread::~ROIThread()
{
}

BOOL ROIThread::InitInstance()
{
	//在此执行任意逐线程初始化
	m_dilogROI.DoModal();
	return TRUE;
}

int ROIThread::ExitInstance()
{
	//在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(ROIThread, CWinThread)
END_MESSAGE_MAP()

// ROIThread 消息处理程序
int ROIThread::Run() {
	::SendMessage(m_hWnd, WM_UPDARA_CONFIG, 0, 0);//向主界面发送消息，更新配置文件
	return CWinThread::Run();
}
