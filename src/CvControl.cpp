#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include "CvControl.hpp"
#include "Processes.hpp"

using namespace cv;
using namespace std;

int CvControl::show(string windowName){
    frame = *process.start(&frame);
    imshow(windowName, frame);
    return 1;
}

int CvControl::startImage(string path, string windowName){
	frame = imread(path);
	show(windowName);
    waitKey(0);
	return 0;
}

int CvControl::startVideo(string path, string windowName){
	VideoCapture capture;
	string str;
	int key = 0;
    if(path==""){
        capture.open(0);
    }else{
        capture.open(path);
    }
    if (!capture.isOpened())
	{
		cerr << "Error opening video file!\n";
		return 0;
	}
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	namedWindow(windowName, cv::WINDOW_AUTOSIZE);
	uchar fillValue = 128;
	while (key != 'q') {
		capture.read(frame);
		if (frame.empty()) break;
		//drawText(frame);
        show(windowName);
		key = waitKey(1);
	}
	destroyWindow(windowName);
	capture.release();
    return 1;
}

int CvControl::startCamera(string windowName){
    startVideo("",windowName);
    return 1;
}