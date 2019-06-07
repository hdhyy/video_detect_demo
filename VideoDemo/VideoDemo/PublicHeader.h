#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include "afxstr.h"

#include <vector>
#include <queue>

#include "CvvImage.h"
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

constexpr auto IMAGE_HEIGHT = 600;
constexpr auto  IMAGE_WIDTH = 900;
constexpr auto  IMAGE_CHANNELS = 3;

extern Mat g_matSourceFrame;
extern CString g_cstrROIFile;
extern double scale;
extern int real_width;
extern int real_height;