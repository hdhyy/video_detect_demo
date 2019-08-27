#include "stdafx.h"
#include "FaceFactory.h"

FaceFactory::FaceFactory()
{
	FD_model = new seeta::ModelSetting("model/seetaface/fd_2_00.dat", device, id);
	PD_model = new seeta::ModelSetting("model/seetaface/pd_2_00_pts5.dat", device, id);
	FR_model = new seeta::ModelSetting("model/seetaface/fr_2_10.dat", device, id);

	engine = new seeta::FaceEngine(*FD_model, *PD_model, *FR_model, 2, 16);
	//set face detector's min face size
	engine->FD.set(seeta::FaceDetector::PROPERTY_MIN_FACE_SIZE, 30);
	GalleryIndex = new std::vector<int64_t>(GalleryImageFilename.size());

	for (size_t i = 0; i < GalleryImageFilename.size(); ++i)
	{
		//register face into facedatabase
		std::string& filename = GalleryImageFilename[i];
		int64_t& index = (*GalleryIndex)[i];
		std::cerr << "Registering... " << filename << std::endl;
		seeta::cv::ImageData image = cv::imread(filename);
		auto id = engine->Register(image);
		index = id;
		std::cerr << "Registered id = " << id << std::endl;
	}

	for (size_t i = 0; i < GalleryIndex->size(); ++i)
	{
		// save index and name pair
		if ((*GalleryIndex)[i] < 0) continue;
		GalleryIndexMap.insert(std::make_pair((*GalleryIndex)[i], GalleryImageFilename[i]));
	}
}

void FaceFactory::play_demo()
{

	std::cout << "----open camera----" << std::endl;
	// Open default USB camera
	cv::VideoCapture* capture = new cv::VideoCapture("video/demo.mp4");

	cv::Mat frame;
	int count = 0;
	std::vector<SeetaFaceInfo> faces;
	while (capture->isOpened())
	{
		(*capture) >> frame;
		OutputDebugString(_T("----open camera----\n"));
		std::cout << "----open camera----" << std::endl;
		if (frame.empty()) continue;
		count++;
		seeta::cv::ImageData image = frame;

		// Detect all faces
		faces = engine->DetectFaces(image);

		for (SeetaFaceInfo& face : faces)
		{
			// Query top 1
			int64_t index = -1;
			float similarity = 0;

			auto points = engine->DetectPoints(image, face);
			auto queried = engine->QueryTop(image, points.data(), 1, &index, &similarity);

			cv::rectangle(frame, cv::Rect(face.pos.x, face.pos.y, face.pos.width, face.pos.height), CV_RGB(128, 128, 255), 3);
			for (int i = 0; i < 5; ++i)
			{
				auto& point = points[i];
				cv::circle(frame, cv::Point(int(point.x), int(point.y)), 2, CV_RGB(128, 255, 128), -1);
			}

			// no face queried from database
			if (queried < 1) continue;

			// similarity greater than threshold, means recognized
			if (similarity > threshold)
			{
				cv::putText(frame, GalleryIndexMap[index], cv::Point(face.pos.x, face.pos.y - 5), 3, 1, CV_RGB(255, 128, 128));
			}
		}

		cv::imshow("Frame", frame);

		auto key = cv::waitKey(20);
		if (key == 27)
		{
			break;
		}
	}
}

cv::Mat FaceFactory::get_detect_result(int detect_type, cv::Mat *src)
{
	std::vector<SeetaFaceInfo> faces;
	switch (detect_type)
	{
		case SEETAFACE2:
			seeta::cv::ImageData image = *src;

			// Detect all faces
			faces = engine->DetectFaces(image);

			for (SeetaFaceInfo& face : faces)
			{
				// Query top 1
				int64_t index = -1;
				float similarity = 0;

				auto points = engine->DetectPoints(image, face);
				auto queried = engine->QueryTop(image, points.data(), 1, &index, &similarity);

				cv::rectangle(*src, cv::Rect(face.pos.x, face.pos.y, face.pos.width, face.pos.height), CV_RGB(128, 128, 255), 3);
				for (int i = 0; i < 5; ++i)
				{
					auto& point = points[i];
					cv::circle(*src, cv::Point(int(point.x), int(point.y)), 2, CV_RGB(128, 255, 128), -1);
				}

				// no face queried from database
				if (queried < 1) continue;

				// similarity greater than threshold, means recognized
				if (similarity > threshold)
				{
					cv::putText(*src, GalleryIndexMap[index], cv::Point(face.pos.x, face.pos.y - 5), 3, 1, CV_RGB(255, 128, 128));
				}
			}
	}
	return *src;
}
