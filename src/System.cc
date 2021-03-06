/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/



#include "System.h"
#include "Converter.h"
#include <thread>
#include <pangolin/pangolin.h>
#include <iomanip>

namespace ORB_SLAM2
{

System::System(const string &strVocFile, const string &strSettingsFile, const eSensor sensor,
               const bool bUseViewer):mSensor(sensor), mpViewer(static_cast<Viewer*>(NULL)), mbReset(false),mbActivateLocalizationMode(false),
        mbDeactivateLocalizationMode(false)
{
    // Output welcome message
    cout << endl <<
    "ORB-SLAM2 Copyright (C) 2014-2016 Raul Mur-Artal, University of Zaragoza." << endl <<
    "This program comes with ABSOLUTELY NO WARRANTY;" << endl  <<
    "This is free software, and you are welcome to redistribute it" << endl <<
    "under certain conditions. See LICENSE.txt." << endl << endl;

    cout << "Input sensor was set to: ";

    if(mSensor==MONOCULAR)
        cout << "Monocular" << endl;
    else if(mSensor==STEREO)
        cout << "Stereo" << endl;
    else if(mSensor==RGBD)
        cout << "RGB-D" << endl;

    //Check settings file
    cv::FileStorage fsSettings(strSettingsFile.c_str(), cv::FileStorage::READ);
    if(!fsSettings.isOpened())
    {
       cerr << "Failed to open settings file at: " << strSettingsFile << endl;
       exit(-1);
    }

    // For debug    by wmh
    string a;
    TryKalmanFilter("aa.a");
    // -- debug


    //Load ORB Vocabulary
    cout << endl << "Loading ORB Vocabulary. This could take a while..." << endl;

    mpVocabulary = new ORBVocabulary();
    bool bVocLoad = mpVocabulary->loadFromTextFile(strVocFile);
    if(!bVocLoad)
    {
        cerr << "Wrong path to vocabulary. " << endl;
        cerr << "Falied to open at: " << strVocFile << endl;
        exit(-1);
    }
    cout << "Vocabulary loaded!" << endl << endl;

    //Create KeyFrame Database
    mpKeyFrameDatabase = new KeyFrameDatabase(*mpVocabulary);

    //Create the Map
    mpMap = new Map();

    //Create Drawers. These are used by the Viewer
    mpFrameDrawer = new FrameDrawer(mpMap);
    mpMapDrawer = new MapDrawer(mpMap, strSettingsFile);

    //Initialize the Tracking thread
    //(it will live in the main thread of execution, the one that called this constructor)
    mpTracker = new Tracking(this, mpVocabulary, mpFrameDrawer, mpMapDrawer,
                             mpMap, mpKeyFrameDatabase, strSettingsFile, mSensor);

    //Initialize the Local Mapping thread and launch
    mpLocalMapper = new LocalMapping(mpMap, mSensor==MONOCULAR);
    mptLocalMapping = new thread(&ORB_SLAM2::LocalMapping::Run,mpLocalMapper);

    //Initialize the Loop Closing thread and launch
    mpLoopCloser = new LoopClosing(mpMap, mpKeyFrameDatabase, mpVocabulary, mSensor!=MONOCULAR);
    mptLoopClosing = new thread(&ORB_SLAM2::LoopClosing::Run, mpLoopCloser);

    //Initialize the Viewer thread and launch
    if(bUseViewer)
    {
        mpViewer = new Viewer(this, mpFrameDrawer,mpMapDrawer,mpTracker,strSettingsFile);
        mptViewer = new thread(&Viewer::Run, mpViewer);
        mpTracker->SetViewer(mpViewer);
    }

    //Set pointers between threads
    mpTracker->SetLocalMapper(mpLocalMapper);
    mpTracker->SetLoopClosing(mpLoopCloser);

    mpLocalMapper->SetTracker(mpTracker);
    mpLocalMapper->SetLoopCloser(mpLoopCloser);

    mpLoopCloser->SetTracker(mpTracker);
    mpLoopCloser->SetLocalMapper(mpLocalMapper);
}

cv::Mat System::TrackStereo(const cv::Mat &imLeft, const cv::Mat &imRight, const double &timestamp)
{
    if(mSensor!=STEREO)
    {
        cerr << "ERROR: you called TrackStereo but input sensor was not set to STEREO." << endl;
        exit(-1);
    }   

    // Check mode change
    {
        unique_lock<mutex> lock(mMutexMode);
        if(mbActivateLocalizationMode)
        {
            mpLocalMapper->RequestStop();

            // Wait until Local Mapping has effectively stopped
            while(!mpLocalMapper->isStopped())
            {
                usleep(1000);
            }

            mpTracker->InformOnlyTracking(true);
            mbActivateLocalizationMode = false;
        }
        if(mbDeactivateLocalizationMode)
        {
            mpTracker->InformOnlyTracking(false);
            mpLocalMapper->Release();
            mbDeactivateLocalizationMode = false;
        }
    }

    // Check reset
    {
    unique_lock<mutex> lock(mMutexReset);
    if(mbReset)
    {
        mpTracker->Reset();
        mbReset = false;
    }
    }

    cv::Mat Tcw = mpTracker->GrabImageStereo(imLeft,imRight,timestamp);

    unique_lock<mutex> lock2(mMutexState);
    mTrackingState = mpTracker->mState;
    mTrackedMapPoints = mpTracker->mCurrentFrame.mvpMapPoints;
    mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;
    return Tcw;
}

cv::Mat System::TrackRGBD(const cv::Mat &im, const cv::Mat &depthmap, const double &timestamp)
{
    if(mSensor!=RGBD)
    {
        cerr << "ERROR: you called TrackRGBD but input sensor was not set to RGBD." << endl;
        exit(-1);
    }    

    // Check mode change
    {
        unique_lock<mutex> lock(mMutexMode);
        if(mbActivateLocalizationMode)
        {
            mpLocalMapper->RequestStop();

            // Wait until Local Mapping has effectively stopped
            while(!mpLocalMapper->isStopped())
            {
                usleep(1000);
            }

            mpTracker->InformOnlyTracking(true);
            mbActivateLocalizationMode = false;
        }
        if(mbDeactivateLocalizationMode)
        {
            mpTracker->InformOnlyTracking(false);
            mpLocalMapper->Release();
            mbDeactivateLocalizationMode = false;
        }
    }

    // Check reset
    {
    unique_lock<mutex> lock(mMutexReset);
    if(mbReset)
    {
        mpTracker->Reset();
        mbReset = false;
    }
    }

    cv::Mat Tcw = mpTracker->GrabImageRGBD(im,depthmap,timestamp);

    unique_lock<mutex> lock2(mMutexState);
    mTrackingState = mpTracker->mState;
    mTrackedMapPoints = mpTracker->mCurrentFrame.mvpMapPoints;
    mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;
    return Tcw;
}

cv::Mat System::TrackMonocular(const cv::Mat &im, const double &timestamp)
{
    if(mSensor!=MONOCULAR)
    {
        cerr << "ERROR: you called TrackMonocular but input sensor was not set to Monocular." << endl;
        exit(-1);
    }

    // Check mode change
    {
        unique_lock<mutex> lock(mMutexMode);
        if(mbActivateLocalizationMode)
        {
            mpLocalMapper->RequestStop();

            // Wait until Local Mapping has effectively stopped
            while(!mpLocalMapper->isStopped())
            {
                usleep(1000);
            }

            mpTracker->InformOnlyTracking(true);
            mbActivateLocalizationMode = false;
        }
        if(mbDeactivateLocalizationMode)
        {
            mpTracker->InformOnlyTracking(false);
            mpLocalMapper->Release();
            mbDeactivateLocalizationMode = false;
        }
    }

    // Check reset
    {
    unique_lock<mutex> lock(mMutexReset);
    if(mbReset)
    {
        mpTracker->Reset();
        mbReset = false;
    }
    }

    cv::Mat Tcw = mpTracker->GrabImageMonocular(im,timestamp);

    unique_lock<mutex> lock2(mMutexState);
    mTrackingState = mpTracker->mState;
    mTrackedMapPoints = mpTracker->mCurrentFrame.mvpMapPoints;
    mTrackedKeyPointsUn = mpTracker->mCurrentFrame.mvKeysUn;

    // update QuadTree by map points
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    if (vpMPs.size() > 0) {
        WMH::QuadTree *QT = new QuadTree(3.2, 7, MapPoint2Point3f(vpMPs));
        QT->UpdateBlockNodeSet(10);   // set Block with a Passable threshold
        mpMapDrawer->setQuadTree(QT);

//        // for test
//        std::set<int> blocks = QT->getBlockedNodeId();
//        std::set<int>::iterator it;
//        for (it = blocks.begin(); it != blocks.end(); it++) {
//            cout << " # " << *it;
//        }// -- for test

    } else {
        mpMapDrawer->setQuadTree(nullptr);
    }

    return Tcw;
}

void System::ActivateLocalizationMode()
{
    unique_lock<mutex> lock(mMutexMode);
    mbActivateLocalizationMode = true;
}

void System::DeactivateLocalizationMode()
{
    unique_lock<mutex> lock(mMutexMode);
    mbDeactivateLocalizationMode = true;
}

bool System::MapChanged()
{
    static int n=0;
    int curn = mpMap->GetLastBigChangeIdx();
    if(n<curn)
    {
        n=curn;
        return true;
    }
    else
        return false;
}

void System::Reset()
{
    unique_lock<mutex> lock(mMutexReset);
    mbReset = true;
}

void System::Shutdown()
{
    mpLocalMapper->RequestFinish();
    mpLoopCloser->RequestFinish();
    if(mpViewer)
    {
        mpViewer->RequestFinish();
        while(!mpViewer->isFinished())
            usleep(5000);
    }

    // Wait until all thread have effectively stopped
    while(!mpLocalMapper->isFinished() || !mpLoopCloser->isFinished() || mpLoopCloser->isRunningGBA())
    {
        usleep(5000);
    }

    if(mpViewer)
        pangolin::BindToContext("ORB-SLAM2: Map Viewer");
}

void System::SaveTrajectoryTUM(const string &filename)
{
    cout << endl << "Saving camera trajectory to " << filename << " ..." << endl;
    if(mSensor==MONOCULAR)
    {
        cerr << "ERROR: SaveTrajectoryTUM cannot be used for monocular." << endl;
        return;
    }

    vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();
    sort(vpKFs.begin(),vpKFs.end(),KeyFrame::lId);

    // Transform all keyframes so that the first keyframe is at the origin.
    // After a loop closure the first keyframe might not be at the origin.
    cv::Mat Two = vpKFs[0]->GetPoseInverse();

    ofstream f;
    f.open(filename.c_str());
    f << fixed;

    // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
    // We need to get first the keyframe pose and then concatenate the relative transformation.
    // Frames not localized (tracking failure) are not saved.

    // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
    // which is true when tracking failed (lbL).
    list<ORB_SLAM2::KeyFrame*>::iterator lRit = mpTracker->mlpReferences.begin();
    list<double>::iterator lT = mpTracker->mlFrameTimes.begin();
    list<bool>::iterator lbL = mpTracker->mlbLost.begin();
    for(list<cv::Mat>::iterator lit=mpTracker->mlRelativeFramePoses.begin(),
        lend=mpTracker->mlRelativeFramePoses.end();lit!=lend;lit++, lRit++, lT++, lbL++)
    {
        if(*lbL)
            continue;

        KeyFrame* pKF = *lRit;

        cv::Mat Trw = cv::Mat::eye(4,4,CV_32F);

        // If the reference keyframe was culled, traverse the spanning tree to get a suitable keyframe.
        while(pKF->isBad())
        {
            Trw = Trw*pKF->mTcp;
            pKF = pKF->GetParent();
        }

        Trw = Trw*pKF->GetPose()*Two;

        cv::Mat Tcw = (*lit)*Trw;
        cv::Mat Rwc = Tcw.rowRange(0,3).colRange(0,3).t();
        cv::Mat twc = -Rwc*Tcw.rowRange(0,3).col(3);

        vector<float> q = Converter::toQuaternion(Rwc);

        f << setprecision(6) << *lT << " " <<  setprecision(9) << twc.at<float>(0) << " " << twc.at<float>(1) << " " << twc.at<float>(2) << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;
    }
    f.close();
    cout << endl << "trajectory saved!" << endl;
}


void System::SaveKeyFrameTrajectoryTUM(const string &filename)
{
    cout << endl << "Saving keyframe trajectory to " << filename << " ..." << endl;

    vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();
    sort(vpKFs.begin(),vpKFs.end(),KeyFrame::lId);

    // Transform all keyframes so that the first keyframe is at the origin.
    // After a loop closure the first keyframe might not be at the origin.
    //cv::Mat Two = vpKFs[0]->GetPoseInverse();

    ofstream f;
    f.open(filename.c_str());
    f << fixed;

    for(size_t i=0; i<vpKFs.size(); i++)
    {
        KeyFrame* pKF = vpKFs[i];

       // pKF->SetPose(pKF->GetPose()*Two);

        if(pKF->isBad())
            continue;

        cv::Mat R = pKF->GetRotation().t();
        vector<float> q = Converter::toQuaternion(R);
        cv::Mat t = pKF->GetCameraCenter();
        f << setprecision(6) << pKF->mTimeStamp << setprecision(7) << " " << t.at<float>(0) << " " << t.at<float>(1) << " " << t.at<float>(2)
          << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << endl;

    }

    f.close();
    cout << endl << "trajectory saved!" << endl;
}

void System::SaveTrajectoryKITTI(const string &filename)
{
    cout << endl << "Saving camera trajectory to " << filename << " ..." << endl;
    if(mSensor==MONOCULAR)
    {
        cerr << "ERROR: SaveTrajectoryKITTI cannot be used for monocular." << endl;
        return;
    }

    vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();
    sort(vpKFs.begin(),vpKFs.end(),KeyFrame::lId);

    // Transform all keyframes so that the first keyframe is at the origin.
    // After a loop closure the first keyframe might not be at the origin.
    cv::Mat Two = vpKFs[0]->GetPoseInverse();

    ofstream f;
    f.open(filename.c_str());
    f << fixed;

    // Frame pose is stored relative to its reference keyframe (which is optimized by BA and pose graph).
    // We need to get first the keyframe pose and then concatenate the relative transformation.
    // Frames not localized (tracking failure) are not saved.

    // For each frame we have a reference keyframe (lRit), the timestamp (lT) and a flag
    // which is true when tracking failed (lbL).
    list<ORB_SLAM2::KeyFrame*>::iterator lRit = mpTracker->mlpReferences.begin();
    list<double>::iterator lT = mpTracker->mlFrameTimes.begin();
    for(list<cv::Mat>::iterator lit=mpTracker->mlRelativeFramePoses.begin(), lend=mpTracker->mlRelativeFramePoses.end();lit!=lend;lit++, lRit++, lT++)
    {
        ORB_SLAM2::KeyFrame* pKF = *lRit;

        cv::Mat Trw = cv::Mat::eye(4,4,CV_32F);

        while(pKF->isBad())
        {
          //  cout << "bad parent" << endl;
            Trw = Trw*pKF->mTcp;
            pKF = pKF->GetParent();
        }

        Trw = Trw*pKF->GetPose()*Two;

        cv::Mat Tcw = (*lit)*Trw;
        cv::Mat Rwc = Tcw.rowRange(0,3).colRange(0,3).t();
        cv::Mat twc = -Rwc*Tcw.rowRange(0,3).col(3);

        f << setprecision(9) << Rwc.at<float>(0,0) << " " << Rwc.at<float>(0,1)  << " " << Rwc.at<float>(0,2) << " "  << twc.at<float>(0) << " " <<
             Rwc.at<float>(1,0) << " " << Rwc.at<float>(1,1)  << " " << Rwc.at<float>(1,2) << " "  << twc.at<float>(1) << " " <<
             Rwc.at<float>(2,0) << " " << Rwc.at<float>(2,1)  << " " << Rwc.at<float>(2,2) << " "  << twc.at<float>(2) << endl;
    }
    f.close();
    cout << endl << "trajectory saved!" << endl;
}

// by wmh
void System::SaveKeyFrameInfo(const string &filename)
{
    // cout << "Saving map info for wmh, to " << filename << " ..." << endl;

    // Get the data to print.
    vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();
    // vector<MapPoint*> vpMPs = mpMap->GetAllMapPoints();
    cout << "Got " << vpKFs.size() << " Key Frames;" << endl;
    // cout << " and " << vpMPs.size() << " Map Points." << endl;
    
    // Write file
    cout << "Saving a log to " << filename << endl;
    ofstream f;
    f.open(filename.c_str());
    f << fixed;

    // -- Write info of KeyFrame
    // f << "Got " << vpKFs.size() << " Key Frames:"<< endl;
    
    // Head line of a CSV file
    f << "mnID,";
    f <<"nmFrameID,";
    f <<"TimeStamp,";
    f <<"mnGridCols,";
    f <<"mnGridRows,";
    f <<"KeyPoints,";
    f <<"PoseX,";
    f <<"PoseY,";
    f <<"PoseZ"<< endl;

    // Loop over key frames
    for (size_t i=0; i < vpKFs.size(); i++)
    {
        // f << "mnID:" << vpKFs[i]->mnId << endl;
        // f << "nmFrameID:" << vpKFs[i]->mnFrameId << endl;
        // f << "TimeStamp:" << vpKFs[i]->mTimeStamp << endl;
        // f << "mnGridCols:" << vpKFs[i]->mnGridCols << ", Rows:" <<  vpKFs[i]->mnGridRows << endl;
        // f << "KeyPoints:" << vpKFs[i]->N << endl;
        // cv::Mat Tcw = vpKFs[i]->GetPose();
        // f << "Pose:"<< Tcw.at<float>(0,3)<< ", " << Tcw.at<float>(1,3)<< ", " << Tcw.at<float>(2,3) << endl;
        // f << endl;

        // For CSV Format output
        cv::Mat T = vpKFs[i]->GetCameraCenter();
        f << vpKFs[i]->mnId << ",";
        f << vpKFs[i]->mnFrameId << ",";
        f << vpKFs[i]->mTimeStamp << ",";
        f << vpKFs[i]->mnGridCols << ",";
        f << vpKFs[i]->mnGridRows << ",";
        f << vpKFs[i]->N << ",";
        f << T.at<float>(0,3) << ",";
        f << T.at<float>(1,3) << ",";
        f << T.at<float>(2,3) << endl;
    }

    f.close();
}

void System::SaveMapPointInfo(const string &filename)
{
//    vector<MapPoint*> vpMPs = mpMap->GetAllMapPoints();
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    cout << "Got " << vpMPs.size() << " Map Points;" << endl;

    // Write file
    cout << "Saving a log to " << filename << endl;
    ofstream f;
    f.open(filename.c_str());
    f << fixed;

    // -- Write info of MapPoint
    // Head line of a CSV file
    f << "mnId,";
    f << "RefKF.Id,";
    f << "RefKF.FrameId,";

    f << "NextId,";
    f << "FirstKF,";
    f << "FirstFrame,";

    f <<"PoseX,";
    f <<"PoseY,";
    f <<"PoseZ"<< endl;

    // TODO: Add output field according to MapPoint.h and .cc
    // Loop over map points
    for (size_t i=0; i < vpMPs.size(); i++)
    {
        if(vpMPs[i]->isBad())
            continue;
        // For CSV Format output
        cv::Mat pos = vpMPs[i]->GetWorldPos();
        f << vpMPs[i]->mnId << ",";
        f << vpMPs[i]->GetReferenceKeyFrame()->mnId << ",";
        f << vpMPs[i]->GetReferenceKeyFrame()->mnFrameId << ",";

        f << vpMPs[i]->nNextId << ",";
        f << vpMPs[i]->mnFirstKFid << ",";
        f << vpMPs[i]->mnFirstFrame << ",";

        f << pos.at<float>(0) << ",";
        f << pos.at<float>(1) << ",";
        f << pos.at<float>(2) << endl;
    }

    f.close();
}

// Out put the camera position of Current frame  by wmh
void System::SaveCurrentFrameInfo(const string &filename)
{
    cv::Mat CamPos = mpTracker->mCurrentFrame.GetCameraCenter();

//    // Print the camera position
//    if (CamPos.rows > 0)
//    {
//        cout<< "CurrentFrame No."<<  mpTracker->mCurrentFrame.mnId <<" at(mOw): "
//            << CamPos.at<float>(0) << "; "
//            << CamPos.at<float>(1) << "; "
//            << CamPos.at<float>(2)<< endl;
//    }

    // Write file
    cout << "Saving camera position to " << filename << endl;
    ofstream f;
    if (mpTracker->mCurrentFrame.mnId == 0)
        f.open(filename.c_str());
    else
        f.open(filename.c_str(),ios::app);

    f << fixed;
    f << mpTracker->mCurrentFrame.mnId << ", ";
    f << mpTracker->mCurrentFrame.mTimeStamp << ", ";
    if (CamPos.rows > 0)
    {

        f << CamPos.at<float>(0)<< ", ";
        f << CamPos.at<float>(1)<< ", ";
        f << CamPos.at<float>(2)<< endl;
    }
    else
    {
        f << "xx, yy, zz" << endl;
    }

    f.close();
}

vector<WMH::Point3f *> System::MapPoint2Point3f(vector<MapPoint *> MPs){
    vector<WMH::Point3f *> MapPoints;
    int it;
    for (it = 0; it < MPs.size();it++) {
        if(MPs[it]->isBad())
            continue;
        WMH::Point3f *point = new WMH::Point3f;
        cv::Mat pos = MPs[it]->GetWorldPos();

        // TODO: Need calculate the Projction
        point->x = pos.at<float>(0);
//        point->y = pos.at<float>(1);
        point->y = 0.0;
        point->z = pos.at<float>(2);
        MapPoints.insert(MapPoints.end(),point);
    }
    return MapPoints;
}

void System::TryKalmanFilter(const string &filename)
{

    float X[10][3] =
            {
                    10,    50,     15.6,
                    12,    49,     16,
                    11,    52,     15.8,
                    13,    52.2,   15.8,
                    12.9,  50,     17,
                    14,    48,     16.6,
                    13.7,  49,     16.5,
                    13.6,  47.8,   16.4,
                    12.3,  46,     15.9,
                    13.1,  45,     16.2
            };

    float A[10][3] =
            {
                    10,    50,     15.6,
                    12,    49,     16,
                    11,    52,     15.8,
                    13,    52.2,   15.8,
                    12.9,  50,     17,
                    14,    48,     16.6,
                    13.7,  49,     16.5,
                    13.6,  47.8,   16.4,
                    12.3,  46,     15.9,
                    13.1,  45,     16.2
            };



    const int stateNum=3;
    const int measureNum=3;
    const float dt = 0.2;

    cv::KalmanFilter KF(stateNum, measureNum, 0);

    KF.transitionMatrix = (cv::Mat_<float>(3, 3) <<1,0,0,0,1,0,0,0,1);  //转移矩阵A
    setIdentity(KF.measurementMatrix);                                             //测量矩阵H
    setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));                            //系统噪声方差矩阵Q
    setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));                        //测量噪声方差矩阵R
    setIdentity(KF.errorCovPost, cv::Scalar::all(1));
    cv::Mat measurement = cv::Mat::zeros(measureNum, 1, CV_32F);

    //初始状态值
    KF.statePost = (cv::Mat_<float>(3, 1) << A[0][0], A[0][0], A[0][0]);
    cout<<"state0="<<KF.statePost<<endl;

    for(int i=1;i<=9;i++)
    {
        //预测
        cv::Mat prediction = KF.predict();
        //计算测量值
        measurement.at<float>(0) = (float)A[i][0];
        measurement.at<float>(1) = (float)A[i][1];
        measurement.at<float>(2) = (float)A[i][2];
        //更新
        KF.correct(measurement);
        //输出结果
        cout<<"predict ="<<"\t"<<prediction.at<float>(0)<<"\t"<<prediction.at<float>(1)<<"\t"<<prediction.at<float>(2)<<endl;
        cout<<"measurement="<<"\t"<<measurement.at<float>(0)<<"\t"<<measurement.at<float>(1)<<"\t"<<measurement.at<float>(2)<<endl;
        cout<<"correct ="<<"\t"<<KF.statePost.at<float>(0)<<"\t"<<KF.statePost.at<float>(1)<<"\t"<<KF.statePost.at<float>(2)<<endl;
    }
    system("pause");

}


    int System::GetTrackingState()
{
    unique_lock<mutex> lock(mMutexState);
    // 1: not init yet 2:Tracking 3:lost
    return mTrackingState;
}

vector<MapPoint*> System::GetTrackedMapPoints()
{
    unique_lock<mutex> lock(mMutexState);
    return mTrackedMapPoints;
}

vector<cv::KeyPoint> System::GetTrackedKeyPointsUn()
{
    unique_lock<mutex> lock(mMutexState);
    return mTrackedKeyPointsUn;
}


MapDrawer* System::getMapDrawer() {
    return mpMapDrawer;
}


} //namespace ORB_SLAM


