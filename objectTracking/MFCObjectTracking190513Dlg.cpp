
// MFCObjectTracking190513Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCObjectTracking190513.h"
#include "MFCObjectTracking190513Dlg.h"
#include "afxdialogex.h"
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/highgui/highgui_c.h>

#include "Resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace cv;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCObjectTracking190513Dlg dialog



CMFCObjectTracking190513Dlg::CMFCObjectTracking190513Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCOBJECTTRACKING190513_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCObjectTracking190513Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCObjectTracking190513Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCObjectTracking190513Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCObjectTracking190513Dlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMFCObjectTracking190513Dlg message handlers

BOOL CMFCObjectTracking190513Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCObjectTracking190513Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCObjectTracking190513Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCObjectTracking190513Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//#pragma region mfcAttachOpencv
//
//bool CMFCObjectTracking190513Dlg::attachWindow(string &pic, const char* name, int ID)
//{
//	pic = string(name);
//	cv::namedWindow(pic, 1);
//	HWND hWnd = (HWND)cvGetWindowHandle(name);
//	HWND hParent = ::GetParent(hWnd);
//	::SetParent(hWnd, GetDlgItem(ID)->m_hWnd);
//	::ShowWindow(hParent, SW_HIDE);
//	return true;
//}
//
//bool CMFCObjectTracking190513Dlg::showImage(string pic, int id, cv::Mat mat)
//{
//	CRect rect;
//	GetDlgItem(id)->GetClientRect(&rect);
//	cv::resize(mat, mat, cv::Size(rect.Width(), rect.Height()), CV_INTER_CUBIC);
//	imshow(pic, mat);
//	return true;
//}
//
//#pragma endregion


void CMFCObjectTracking190513Dlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	/*string pic;*/
	//attachWindow(pic, "win", IDC_STATIC);
	//Mat mat = imread("demo.jpg");
	//showImage(pic, IDC_STATIC, mat);

	CRect rect;
	GetDlgItem(IDC_STATIC)->GetClientRect(rect); //IDC_PIC为picture控件ID号
	cv::namedWindow("windows", 1);
	cv::resizeWindow("windows", rect.Width(), rect.Height());

	HWND hWnd = (HWND)cvGetWindowHandle("windows");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STATIC)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	//Mat image = imread("demo.jpg");
	//cv::imshow("windows", image);
#pragma region play video
	VideoCapture cap("videos/los_angeles.mp4"); // open the default camera
	//if (!cap.isOpened())  // check if we succeeded
	//	return -1;

	//Mat edges;
	//namedWindow("edges", 1);
	for (;;)
	{
		Mat frame;
		cap >> frame; // get a new frame from camera
		//cvtColor(frame, edges, COLOR_BGR2GRAY);
		//GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		//Canny(edges, edges, 0, 30, 3);
		imshow("windows", frame);
		if (waitKey(25) >= 0) break;
	}
#pragma endregion

}

#pragma region
// MultiTrackerOpencv190525pch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "pch.h"
//#include <iostream>

/*
  Copyright 2018 BIG VISION LLC ALL RIGHTS RESERVED
*/

//#include <opencv2/opencv.hpp>
//#include <opencv2/tracking.hpp>

//using namespace cv;
//using namespace std;

vector<string> trackerTypes = { "BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };

// create tracker by name
Ptr<Tracker> createTrackerByName(string trackerType)
{
	Ptr<Tracker> tracker;
	if (trackerType == trackerTypes[0])
		tracker = TrackerBoosting::create();
	else if (trackerType == trackerTypes[1])
		tracker = TrackerMIL::create();
	else if (trackerType == trackerTypes[2])
		tracker = TrackerKCF::create();
	else if (trackerType == trackerTypes[3])
		tracker = TrackerTLD::create();
	else if (trackerType == trackerTypes[4])
		tracker = TrackerMedianFlow::create();
	else if (trackerType == trackerTypes[5])
		tracker = TrackerGOTURN::create();
	else if (trackerType == trackerTypes[6])
		tracker = TrackerMOSSE::create();
	else if (trackerType == trackerTypes[7])
		tracker = TrackerCSRT::create();
	else {
		cout << "Incorrect tracker name" << endl;
		cout << "Available trackers are: " << endl;
		for (vector<string>::iterator it = trackerTypes.begin(); it != trackerTypes.end(); ++it)
			std::cout << " " << *it << endl;
	}
	return tracker;
}

// Fill the vector with random colors
void getRandomColors(vector<Scalar> &colors, int numColors)
{
	RNG rng(0);
	for (int i = 0; i < numColors; i++)
		colors.push_back(Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
}

//int main(int argc, char * argv[])
//{
//	cout << "Default tracking algoritm is CSRT" << endl;
//	cout << "Available tracking algorithms are:" << endl;
//	for (vector<string>::iterator it = trackerTypes.begin(); it != trackerTypes.end(); ++it)
//		std::cout << " " << *it << endl;
//
//	// Set tracker type. Change this to try different trackers.
//	string trackerType = "CSRT";
//
//	// set default values for tracking algorithm and video
//	string videoPath = "videos/run.mp4";
//
//	// Initialize MultiTracker with tracking algo
//	vector<Rect> bboxes;
//
//	// create a video capture object to read videos
//	cv::VideoCapture cap(videoPath);
//	Mat frame;
//
//	// quit if unabke to read video file
//	if (!cap.isOpened())
//	{
//		cout << "Error opening video file " << videoPath << endl;
//		return -1;
//	}
//
//	// read first frame
//	cap >> frame;
//
//	// draw bounding boxes over objects
//	// selectROI's default behaviour is to draw box starting from the center
//	// when fromCenter is set to false, you can draw box starting from top left corner
//	bool showCrosshair = true;
//	bool fromCenter = false;
//	cout << "\n==========================================================\n";
//	cout << "OpenCV says press c to cancel objects selection process" << endl;
//	cout << "It doesn't work. Press Escape to exit selection process" << endl;
//	cout << "\n==========================================================\n";
//	cv::selectROIs("MultiTracker", frame, bboxes, showCrosshair, fromCenter);
//
//	// quit if there are no objects to track
//	if (bboxes.size() < 1)
//		return 0;
//
//	vector<Scalar> colors;
//	getRandomColors(colors, bboxes.size());
//
//	// Create multitracker
//	Ptr<MultiTracker> multiTracker = cv::MultiTracker::create();
//
//	// initialize multitracker
//	for (int i = 0; i < bboxes.size(); i++)
//		multiTracker->add(createTrackerByName(trackerType), frame, Rect2d(bboxes[i]));
//
//	// process video and track objects
//	cout << "\n==========================================================\n";
//	cout << "Started tracking, press ESC to quit." << endl;
//	while (cap.isOpened())
//	{
//		// get frame from the video
//		cap >> frame;
//
//		// stop the program if reached end of video
//		if (frame.empty()) break;
//
//		//update the tracking result with new frame
//		multiTracker->update(frame);
//
//		// draw tracked objects
//		for (unsigned i = 0; i < multiTracker->getObjects().size(); i++)
//		{
//			rectangle(frame, multiTracker->getObjects()[i], colors[i], 2, 1);
//		}
//
//		// show frame
//		imshow("MultiTracker", frame);
//
//		// quit on x button
//		//if (waitKey(1) == 27) break;
//		if (waitKey(1000) == 's') {
//			cout << "&& s &&" << endl;
//#pragma region add new tracker
//			cv::selectROIs("MultiTracker", frame, bboxes, showCrosshair, fromCenter);
//
//			// quit if there are no objects to track
//			//if (bboxes.size() < 1)
//			//	return 0;
//
//			//vector<Scalar> colors;
//			getRandomColors(colors, bboxes.size());
//
//			// Create multitracker
//			//Ptr<MultiTracker> multiTracker = cv::MultiTracker::create();
//
//			// initialize multitracker
//			for (int i = 0; i < bboxes.size(); i++)
//				multiTracker->add(createTrackerByName(trackerType), frame, Rect2d(bboxes[i]));
//#pragma endregion
//		}
//	}
//
//
//}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#pragma endregion

void CMFCObjectTracking190513Dlg::OnBnClickedButton2()
{
	/*cout << "Default tracking algoritm is CSRT" << endl;
	cout << "Available tracking algorithms are:" << endl;*/
	for (vector<string>::iterator it = trackerTypes.begin(); it != trackerTypes.end(); ++it)
		std::cout << " " << *it << endl;

	// Set tracker type. Change this to try different trackers.
	string trackerType = "CSRT";

	// set default values for tracking algorithm and video
	string videoPath = "videos/los_angeles.mp4";

	// Initialize MultiTracker with tracking algo
	vector<Rect> bboxes;

	// create a video capture object to read videos
	cv::VideoCapture cap(videoPath);
	Mat frame;

	// quit if unabke to read video file
	/*if (!cap.isOpened())
	{
		cout << "Error opening video file " << videoPath << endl;
		return -1;
	}*/

	// read first frame
	cap >> frame;

	// draw bounding boxes over objects
	// selectROI's default behaviour is to draw box starting from the center
	// when fromCenter is set to false, you can draw box starting from top left corner
	bool showCrosshair = true;
	bool fromCenter = false;
	/*cout << "\n==========================================================\n";
	cout << "OpenCV says press c to cancel objects selection process" << endl;
	cout << "It doesn't work. Press Escape to exit selection process" << endl;
	cout << "\n==========================================================\n";*/
	cv::selectROIs("MultiTracker", frame, bboxes, showCrosshair, fromCenter);

	// quit if there are no objects to track
	/*if (bboxes.size() < 1)
		return 0;*/

	vector<Scalar> colors;
	getRandomColors(colors, bboxes.size());

	// Create multitracker
	Ptr<MultiTracker> multiTracker = cv::MultiTracker::create();

	// initialize multitracker
	for (int i = 0; i < bboxes.size(); i++)
		multiTracker->add(createTrackerByName(trackerType), frame, Rect2d(bboxes[i]));

	// process video and track objects
	/*cout << "\n==========================================================\n";
	cout << "Started tracking, press ESC to quit." << endl;*/

	/*namedWindow("MultiTracker",1);
	HWND hWnd = (HWND)cvGetWindowHandle("MultiTracker");
	HWND hParentWnd = ::GetParent(hWnd);
	::SetWindowPos(hWnd,HWND_TOPMOST,100,100,100,100,SWP_NOSIZE);*/

	while (cap.isOpened())
	{
		// get frame from the video
		cap >> frame;

		// stop the program if reached end of video
		if (frame.empty()) break;

		//update the tracking result with new frame
		multiTracker->update(frame);

		// draw tracked objects
		for (unsigned i = 0; i < multiTracker->getObjects().size(); i++)
		{
			rectangle(frame, multiTracker->getObjects()[i], colors[i], 2, 1);
		}

		// show frame
		imshow("MultiTracker", frame);

		// quit on x button
		//if (waitKey(1) == 27) break;
		if (waitKey(1) == 's') {
			cout << "&& s &&" << endl;
#pragma region add new tracker
			cv::selectROIs("MultiTracker", frame, bboxes, showCrosshair, fromCenter);

			// quit if there are no objects to track
			//if (bboxes.size() < 1)
			//	return 0;

			//vector<Scalar> colors;
			getRandomColors(colors, bboxes.size());

			// Create multitracker
			//Ptr<MultiTracker> multiTracker = cv::MultiTracker::create();

			// initialize multitracker
			for (int i = 0; i < bboxes.size(); i++)
				multiTracker->add(createTrackerByName(trackerType), frame, Rect2d(bboxes[i]));
#pragma endregion
		}
	}





	//	// TODO: Add your control notification handler code here
	//	CRect rect;
	//	GetDlgItem(IDC_STATIC2)->GetClientRect(rect); //IDC_PIC为picture控件ID号
	//	cv::namedWindow("windows2", 1);
	//	cv::resizeWindow("windows2", rect.Width(), rect.Height());
	//
	//	HWND hWnd = (HWND)cvGetWindowHandle("windows2");
	//	HWND hParent = ::GetParent(hWnd);
	//	::SetParent(hWnd, GetDlgItem(IDC_STATIC2)->m_hWnd);
	//	::ShowWindow(hParent, SW_HIDE);
	//	//Mat image = imread("demo.jpg");
	//	//cv::imshow("windows", image);
	//#pragma region play video
	//	VideoCapture cap("videos/los_angeles.mp4"); // open the default camera
	//	//if (!cap.isOpened())  // check if we succeeded
	//	//	return -1;
	//
	//	//Mat edges;
	//	//namedWindow("edges", 1);
	//	for (;;)
	//	{
	//		Mat frame;
	//		cap >> frame; // get a new frame from camera
	//		//cvtColor(frame, edges, COLOR_BGR2GRAY);
	//		//GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
	//		//Canny(edges, edges, 0, 30, 3);
	//		imshow("windows2", frame);
	//		if (waitKey(25) >= 0) break;
	//	}
}
