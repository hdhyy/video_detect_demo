#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include "afxstr.h"

#include <vector>
#include <queue>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "CvvImage.h"
#include "time.h"
#include <cxcore.hpp>
#include <cvaux.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

#define WM_UPDATE_MESSAGE (WM_USER+200)
#define WM_UPDARA_CONFIG (WM_USER+300)
//#define OLD_ALGO
//#define TRAIN_MODE
//#define DEBUG_OUTPUT

#ifdef TRAIN_MODE
extern bool trainComplete;
#endif


constexpr auto pixel_wirth = 1600;
constexpr auto pixel_height = 700;
constexpr auto IMAGE_HEIGHT = 500;
constexpr auto  IMAGE_WIDTH = 750;
constexpr auto  IMAGE_CHANNELS = 3;

extern Mat g_matSourceFrame;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
extern CString g_cstrROIFile;
extern double scale;
extern int real_width;
extern int real_height;