#pragma once
#ifndef __GUIDEFILTER__
#define __GUIDEFILTER__

#include "PublicHeader.h"

extern void GuideFilter(Mat& source, Mat& guided_image, Mat& output, int radius, double epsilon);


#endif


