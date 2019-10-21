#include "stdafx.h"
#include "ObjectTracking.h"

ObjectTracking::ObjectTracking()
{

}

ObjectTracking::ObjectTracking(string prototxt, string weights)
{
	this->prototxt = prototxt;
	this->weights = weights;
	// Load names of classes
	string classesFile = "models\\yolo3\\coco.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

	loadModel();
}

Ptr<Tracker> ObjectTracking::createTrackerByName(string trackerType)
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

void ObjectTracking::getRandomColors(vector<Scalar>& colors, int numColors)
{
	RNG rng(0);
	for (int i = 0; i < numColors; i++)
		colors.push_back(Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
}

void ObjectTracking::play_demo()
{
	/*cout << "Default tracking algoritm is CSRT" << endl;
	cout << "Available tracking algorithms are:" << endl;*/
	for (vector<string>::iterator it = trackerTypes.begin(); it != trackerTypes.end(); ++it)
		std::cout << " " << *it << endl;

	// Set tracker type. Change this to try different trackers.
	string trackerType = "CSRT";

	// set default values for tracking algorithm and video
	string videoPath = "video/smoke.avi";

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
		Sleep(33);//º‰∏Ù33∫¡√Î
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
}

void ObjectTracking::postprocess()
{

}

void ObjectTracking::postprocess(Mat& frame, const vector<Mat>& outs)
{
	vector<int> classIds;
	vector<float> confidences;
	vector<Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confThreshold)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	vector<int> indices;
	dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		Rect box = boxes[idx];
		//only person
		if (classIds[idx] == 0) {
			drawPred(classIds[idx], confidences[idx], box.x, box.y,
				box.x + box.width, box.y + box.height, frame);
		}
	}
}

// Draw the predicted bounding box
void ObjectTracking::drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

	//Get the label for the class name and its confidence
	string label = format("%.2f", conf);
	if (!classes.empty())
	{
		CV_Assert(classId < (int)classes.size());
		label = classes[classId] + ":" + label;
		//cout << classes[classId] << "	" << classId << endl;
	}

	//Display the label at the top of the bounding box
	int baseLine;
	Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = max(top, labelSize.height);
	rectangle(frame, Point(left, top - round(1.5 * labelSize.height)), Point(left + round(1.5 * labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
	putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}
// Get the names of the output layers
vector<String> ObjectTracking::getOutputsNames(const dnn::Net& net)
{
	static vector<String> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		vector<String> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

void ObjectTracking::loadModel()
{
	// Load the network
	net = dnn::readNetFromDarknet(prototxt, weights);
	net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
	net.setPreferableTarget(dnn::DNN_TARGET_CPU);
}

void ObjectTracking::processImage()
{
	// Open a video file or an image file or a camera stream.
	string str, outputFile;
	VideoCapture cap;
	VideoWriter video;
	Mat  blob;
	// Create a 4D blob from a frame.
	dnn::blobFromImage(image, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);

	//Sets the input to the network
	net.setInput(blob);

	// Runs the forward pass to get output of the output layers
	vector<Mat> outs;
	net.forward(outs, getOutputsNames(net));

	// Remove the bounding boxes with low confidence
	postprocess(image, outs);

	//return frame;
}