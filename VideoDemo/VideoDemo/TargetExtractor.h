//
//  TargetExtractor.h
//  FlameDetection
//
//  Created by liberize on 14-4-11.
//  Copyright (c) 2014Äê liberize. All rights reserved.
//

#pragma once
#ifndef __FlameDetection__TargetExtractor__
#define __FlameDetection__TargetExtractor__

#include "PublicHeader.h"
#include "Utils.h"

struct Target;


struct ContourInfo {
	vector<Point> contour;
	double area;
	Rect boundRect;
};


class MyRectangle : public Rect {
public:
	MyRectangle();
	MyRectangle(const Rect& r);

	bool my_near(const MyRectangle& r);
	void merge(const MyRectangle& r);
};


class MyRegion {
public:
	vector<ContourInfo*> contours;
	MyRectangle rect;

	MyRegion();
	MyRegion(ContourInfo* contour, const MyRectangle& rect);
	MyRegion(const vector<ContourInfo*>& contours, const MyRectangle& rect);
	bool my_near(const MyRegion& r);
	void merge(const MyRegion& r);
};


class TargetExtractor {
private:
	static const int MAX_MASK_QUEUE_SIZE = 10;

	Mat mFrame;
	Mat mMask;
	queue<Mat> mMaskQueue;
	Mat mMaskSum;
	vector<ContourInfo> mContours;

	Mat mBackground;
	Ptr<BackgroundSubtractorMOG2> mMOG;

#ifdef OLD_ALGO
	void movementDetect2(int threshold = 30, double learningRate = 0.01);
	void colorDetect2(int threshold = 20);
	void regionGrow2(int areaThreshold, int diffThreshold);
#endif

	void movementDetect(double learningRate = -1);
	void colorDetect(int redThreshold = 150, double saturationThreshold = 0.4);
	void denoise(int ksize = 7, int threshold = 6);
	void fill(int ksize = 7, int threshold = 6);
	void regionGrow(int threshold = 20);
	void smallAreaFilter(int threshold = 10, int keep = 5);
	void accumulate(int threshold = 5);
	void blobTrack(map<int, Target>& targets);

public:
	TargetExtractor();
	const Mat& getMask() const { return mMask; }
	void extract(const Mat& frame, map<int, Target>& targets, bool track);
};

#endif /* defined(__FlameDetection__TargetExtractor__) */
