#pragma once

#include "afxstr.h"
#include <vector>
#include <queue>

#include"CvvImage.h"
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

using namespace std;
using namespace cv;

#define WM_UPDATE_MESSAGE (WM_USER+200)
#define WM_UPDARA_CONFIG (WM_USER+300)

constexpr auto IMAGE_HEIGHT = 600;
constexpr auto  IMAGE_WIDTH = 900;
constexpr auto  IMAGE_CHANNELS = 3;

extern Mat g_matSourceFrame;
extern CString g_cstrROIFile;
extern double scale;
extern int real_width;
extern int real_height;