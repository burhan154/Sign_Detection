#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include "processes.hpp"

using namespace cv;
using namespace std;

Mat* Processes::start(Mat * frame){
    //screen = frame;
    screen = frame;
    //uchar fillValue = 255;
    IVC *image = vc_image_new(frame->cols, frame->rows , 3, 255);
    memcpy(image->data, screen->data, frame->cols * frame->rows* 3);
    process(image);
    memcpy(screen->data, image->data, frame->cols * frame->rows* 3);
    drawText();

    vc_image_free(image);
    return screen;
}

int Processes::process(IVC *src){
    IVC *temp = vc_image_new(src->width, src->height, 1, 255);

    IVC *tempBlue = vc_image_new(src->width, src->height, 3, 255);
    memcpy(tempBlue->data, src->data, src->width * src->height* 3);

    IVC *tempRed = vc_image_new(src->width, src->height, 3, 255);
    memcpy(tempRed->data, src->data, src->width * src->height* 3);

    
    vc_show_red_objects(tempRed);
    temp = process2(tempRed,RED);
    //cout<<labelsnum<<endl;
    if (labelsnum == 0){
        //cout<<"-"<<labelsnum<<endl;
         vc_show_blue_objects(tempBlue);
        temp = process2(tempBlue,BLUE);
    }

    //convertToColorImage(dest,src);

    vc_binary_blob_info(temp, blobs, labelsnum);
    //vc_gray_edge_sobel(dest,dest2,240);
    if (blobs != NULL)
    {
        for (int i = 0; i < labelsnum; i++)
	    {
            if (blobs[i].area>8000&& blobs[i].area<50000) {
                drawBox(src, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
                blobs[i].sign = detect_sign(blobs,i,src);
            }
        }
        //cout<<blobs[0].area<<" "<<blobs[0].xc<<endl;
        //convertToColorImage(temp,src);
        //vc_drow_box_2(temp,src);

    }

    //convertToColorImage(dest2,src);
    vc_image_free(temp);
    vc_image_free(tempRed);
    vc_image_free(tempBlue);
	return 1;
}

IVC * Processes::process2(IVC *src,int color){
    
    IVC *dest = vc_image_new(src->width, src->height, 1, 255);
	IVC *dest2 = vc_image_new(src->width, src->height, 1, 255);

	//vc_show_red_objects(src);
    vc_rgb_to_gray(src,dest);               //image enhancement
	vc_gray_negative(dest);
	vc_gray_to_binary(dest, dest2, 150);    //segmentation techniques
	vc_binary_open(dest2, dest, 5);         //image enhancement
    blobs = vc_binary_blob_labelling(dest, dest2, &labelsnum, color);

    vc_image_free(dest);
	return dest2;
}

string Processes::getSign(int imageColor){
    string text="";
    switch(imageColor){
        case STOP: text="STOP";
        break;
        case CAR: text="CAR";
        break;
        case HIGHWAY: text="HIGHWAY";
        break;
        case ARROWLEFT: text="ARROWLEFT";
        break;
        case ARROWRIGHT: text="ARROWRIGHT";
        break;
        case FORBIDDEN: text="FORBIDDEN";
        break;
        default: text="NOT FOUND";
        break;
    }
    return text;
}

void Processes::drawText()
{
    if (blobs != NULL){

        for (int i = 0; i < labelsnum; i++)
	    {
            if (blobs[i].area>8000&& blobs[i].area<50000) {

                int x = blobs[i].x +blobs[i].width -70;
                int y = blobs[i].y + blobs[i].height + 50;

                if(x>0){
                    putText(*screen ,getSign(blobs[i].sign),Point(x, y), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 128, 0), 1, LINE_AA);
                }
            }
        }
     }
}
