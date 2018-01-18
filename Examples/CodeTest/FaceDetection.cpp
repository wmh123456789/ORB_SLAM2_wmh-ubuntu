//
// Created by wmh on 18-1-17.
//




#include "FaceDetection.h"
#include <iostream>

//using namespace cv;


void FaceDetection::Detect(string ImgPath, int startN, int endN) {
    cv::Mat im;
    CvCapture* capture = 0;
    cv::Mat frame, frameCopy, image,gray;
    cv::CascadeClassifier cascade, nestedCascade;
    vector<cv::Rect> faces;
    cv::Point center;
    int radius;

    double scale = 1;
    if (!cascade.load(cascadeName))
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        return;
    }


    for(int i = startN; i < endN; i++){

        image = cv::imread(ImgPath+"/"+to_string(i)+".jpg",0);
//        cvtColor(image, gray, cv::COLOR_RGB2GRAY);
        equalizeHist(image, image);
        cascade.detectMultiScale(image, faces,
                                 1.3, 5, 0|CV_HAAR_SCALE_IMAGE,
                                 cv::Size(30, 30));
        if (faces.size() > 0){
            cout << "Image" << dec << i << ": " << faces.size() << " faces;" << endl;
            for(int i_face =0; i_face < faces.size(); i_face++){
                center.x = cvRound((faces[i_face].x + faces[i_face].width*0.5)*scale);
                center.y = cvRound((faces[i_face].y + faces[i_face].height*0.5)*scale);
                radius = cvRound(faces[i_face].width + (faces[i_face].height)*0.25*scale);
                circle(image, center, radius, CV_RGB(255, 128, 0), 3, 8, 0);
            }
            cv::imwrite(ImgPath+"/face/"+to_string(i)+"_face.jpg",image);
        }

//        cout << i << endl;
//        cv::imshow("CarCam", im);
//        cv::waitKey(30);
//        cvNamedWindow("result", 1);
//        cout << "In image read" << endl;
//        if (!image.empty())
//        {
//            cvtColor(image, gray, cv::COLOR_BGR2GRAY);
//            equalizeHist(gray, gray);
//            cascade.detectMultiScale(gray, faces,
//                                     1.1, 2, 0
//                                             //|CASCADE_FIND_BIGGEST_OBJECT
//                                             //|CASCADE_DO_ROUGH_SEARCH
//                                             | cv::CASCADE_SCALE_IMAGE
//                    ,
//                                     cv::Size(30, 30));
//            detectAndDraw(image, cascade, nestedCascade, scale, 0);//检测人脸
//            cv::waitKey(50);
//        }
//        cvDestroyWindow("result");
    }

    return;
}

void FaceDetection::Detect(){
    this->Detect(mImagePath,mStartN,mEndN);
}

void FaceDetection::DetectCam() {

    cv::VideoCapture capture;
    cv::Mat image,gray;
    vector<cv::Rect> faces;
    cv::Point center;
    int radius;
    cv::CascadeClassifier cascade;
    double scale = 1;
    if (!cascade.load(cascadeName))
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        return;
    }

    capture.open(0);
    while(1) {
        capture >> image;
        cv::resize(image,image,cv::Size(320,240),0.0,0.0,CV_INTER_LINEAR);


//        cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cvtColor(image, gray, cv::COLOR_RGB2GRAY);
        equalizeHist(gray, gray);
        cascade.detectMultiScale(gray, faces,
                                 1.3, 5, 0|CV_HAAR_SCALE_IMAGE,
                                 cv::Size(30, 30));

        if (faces.size() > 0) {
            for (int i_face = 0; i_face < faces.size(); i_face++) {
                center.x = cvRound((faces[i_face].x + faces[i_face].width * 0.5) * scale);
                center.y = cvRound((faces[i_face].y + faces[i_face].height * 0.5) * scale);
                radius = cvRound(faces[i_face].width + (faces[i_face].height) * 0.1 * scale);
                circle(image, center, radius, CV_RGB(255, 128, 0), 3, 8, 0);
            }
        }
        cv::imshow("win", image);
        char c = cvWaitKey(50);
        if(c=='q') break;
    }

    capture.release();
    return;
}

void FaceDetection::detectAndDraw(cv::Mat& img, cv::CascadeClassifier& cascade,
                   cv::CascadeClassifier& nestedCascade,
                   double scale, bool tryflip)
{
    int i = 0;
    double t = 0;
    vector<cv::Rect> faces, faces2;
    const static cv::Scalar colors[] = { CV_RGB(0, 0, 255),
                                     CV_RGB(0, 128, 255),
                                     CV_RGB(0, 255, 255),
                                     CV_RGB(0, 255, 0),
                                     CV_RGB(255, 128, 0),
                                     CV_RGB(255, 255, 0),
                                     CV_RGB(255, 0, 0),
                                     CV_RGB(255, 0, 255) };//用于画线

    cv::Mat gray, smallImg(cvRound(img.rows / scale), cvRound(img.cols / scale), CV_8UC1);

    cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
    equalizeHist(smallImg, smallImg);

    t = (double)cvGetTickCount();
    cascade.detectMultiScale(smallImg, faces,
                             1.1, 2, 0
                                     //|CASCADE_FIND_BIGGEST_OBJECT
                                     //|CASCADE_DO_ROUGH_SEARCH
                                     | cv::CASCADE_SCALE_IMAGE
            ,
                             cv::Size(30, 30));
    if (tryflip)
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale(smallImg, faces2,
                                 1.1, 2, 0
                                         //|CASCADE_FIND_BIGGEST_OBJECT
                                         //|CASCADE_DO_ROUGH_SEARCH
                                         | cv::CASCADE_SCALE_IMAGE
                ,
                                 cv::Size(30, 30));
        for (vector<cv::Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++)
        {
            faces.push_back(cv::Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }

    t = (double)cvGetTickCount() - t;
    printf("detection time = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));
    for (vector<cv::Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++)
    {
        cv::Mat smallImgROI;
        vector<cv::Rect> nestedObjects;
        cv::Point center;
        cv::Scalar color = colors[i % 8];
        int radius;

        double aspect_ratio = (double)r->width / r->height;
        if (0.75 < aspect_ratio && aspect_ratio < 1.3)
        {
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            circle(img, center, radius, color, 3, 8, 0);
        }
        else
            rectangle(img, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)),
                      cvPoint(cvRound((r->x + r->width - 1)*scale), cvRound((r->y + r->height - 1)*scale)),
                      color, 3, 8, 0);
        if (nestedCascade.empty())
            continue;
        smallImgROI = smallImg(*r);
        nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
                                       1.1, 2, 0
                                               //|CASCADE_FIND_BIGGEST_OBJECT
                                               //|CASCADE_DO_ROUGH_SEARCH
                                               //|CASCADE_DO_CANNY_PRUNING
                                               | cv::CASCADE_SCALE_IMAGE
                ,
                                       cv::Size(30, 30));
        for (vector<cv::Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++)
        {
            center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
            center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
            radius = cvRound((nr->width + nr->height)*0.25*scale);
            circle(img, center, radius, color, 3, 8, 0);
        }
    }
    cv::imshow("result", img);
}