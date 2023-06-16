#ifndef CvControl_hpp
#define CvControl_hpp

#include<string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include "Processes.hpp"

using namespace std;
using namespace cv;

class CvControl
{

private:
    Processes process;
    Mat frame;
    
    int show(string windowName);

    struct Video
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	};

    Video video;
public:
    int startVideo(string path, string windowName);
    int startImage(string path, string windowName);
    int startCamera(string windowName);
};


#endif