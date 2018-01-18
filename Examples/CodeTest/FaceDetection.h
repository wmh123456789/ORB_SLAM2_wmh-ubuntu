//
// Created by wmh on 18-1-17.
//

#ifndef CODETEST_FACEDETECTION_H
#define CODETEST_FACEDETECTION_H
#include <opencv2/core/core.hpp>
#include <opencv/cv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <opencv2/highgui/highgui_c.h>

#include <string>

using namespace std;
//using namespace cv;


class FaceDetection {
public:
    string mImagePath = "/media/wmh/TEMP/SlamTestSeq/MySeq/CarMove2/";
    int mStartN = 1;
    int mEndN = 2400;


//    string cascadeName = "/home/wmh/work/opencv-3.2.0/data/haarcascades/
//  haarcascade_frontalface_alt.xml";
//    haarcascade_frontalface_default.xml

    string cascadePath = "/home/wmh/work/opencv-3.2.0/data/haarcascades/";
    string cascadeName =  cascadePath + "haarcascade_frontalface_default.xml";

    string nestedCascadeName = "/home/wmh/work/opencv-3.2.0/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";


    void Detect(string ImgPath, int startN, int endN);
    void Detect();
    void detectAndDraw(cv::Mat& img, cv::CascadeClassifier& cascade,
                       cv::CascadeClassifier& nestedCascade,
                       double scale, bool tryflip);
    void DetectCam();

};

#endif //CODETEST_FACEDETECTION_H
