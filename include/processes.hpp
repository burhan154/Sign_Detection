
#ifndef Processes_hpp
#define Processes_hpp


extern "C" {
    #include "vc.h"
}

#include<string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>



using namespace std;
using namespace cv;

class Processes
{

private:
    int process(IVC *src);
    IVC * Processes::process2(IVC *src,int color);
    Mat * screen;

    string text;
    int ImageColor;
    OVC* blobs;
    int blobId;
    int labelsnum;
    string Processes::getSign();
public:
    Mat *start(Mat *frame);
    void drawText();
};


#endif