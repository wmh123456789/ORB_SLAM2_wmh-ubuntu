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


#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>

#include<opencv2/core/core.hpp>
// #include<stdlib.h>
#include<System.h>

using namespace std;

void LoadImages(const string &strFile, vector<string> &vstrImageFilenames,
                vector<double> &vTimestamps);
double LoadTimestamp(const string &strFile);

int main(int argc, char **argv)
{
    if (!(argc== 4 || argc == 6))
    {
        cerr << endl << "Usage 1: ./mono_wmh path_to_vocabulary path_to_settings path_to_sequence" << endl;
        cerr << endl << "Usage 2: ./mono_wmh path_to_vocabulary path_to_settings path_to_sequence From_N To_N" << endl;
        return 1;
    }

    // Retrieve paths to images
    // vector<string> vstrImageFilenames;
    vector<double> vTimestamps;
    // string strFile = string(argv[3])+"/rgb.txt";
    // LoadImages(strFile, vstrImageFilenames, vTimestamps);

    // int nImages = vstrImageFilenames.size();

    int nImages = 10000; // Define a buffer length


    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR,true);

    // debug  by wmh
//    SLAM.TryKalmanFilter("aa.a");
    // - debug

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    // cout << "Images in the sequence: " << nImages << endl << endl;

    // Main loop
    cv::Mat im;

    // for(int ni=0; ni<nImages; ni++)
    int ni = 1;

    // Jump-back-and-loop strategy to avoid tracking lost
    int LostCount = 0;
    int TrackStartN = 0;
    int MAXLOST_N  = 10;
    int JUMPBACK_N = 50;
    bool isJumpBack = true;


    // partially-looping function
    int GoToFrom = nImages+2, GoToTerm = nImages+2;

    if(argc == 6){
        GoToFrom = atoi(argv[4]);
        GoToTerm = atoi(argv[5]);
    } else{

    }


    // Fake time stamp:
    double t0 = 1512724685.0989;
    unsigned long ti = 0;

#ifdef COMPILEDWITHC11
    std::chrono::steady_clock::time_point ta = std::chrono::steady_clock::now();
#else
    std::chrono::monotonic_clock::time_point ta = std::chrono::monotonic_clock::now();
#endif
 
//    while(ni>0 && ni < nImages+1)
    bool isInverseLoop = false;
    while(abs(ni) < nImages+1)
    {
        // Read image from file
        // im = cv::imread(string(argv[3])+"/"+vstrImageFilenames[ni],CV_LOAD_IMAGE_UNCHANGED);
        stringstream ni_ss;  // TODO need to optimize
        string ni_s;
        ni_ss << abs(ni);
        ni_ss >> ni_s;
        im = cv::imread(string(argv[3])+"/"+ni_s+".jpg",CV_LOAD_IMAGE_UNCHANGED); // Read image from file buffer.  By wmh
        if(im.empty())
        {
            im = cv::imread(string(argv[3])+"/"+ni_s+".png",CV_LOAD_IMAGE_UNCHANGED);
            if(im.empty())
            {
                cerr << endl << "Failed to load image at: "
                     << string(argv[3]) << "/" << ni_s +".jpg/png"<< endl;
                // return 1;
                break;
            }
        }

        cout << "Image: "<< ni_s << ".jpg/png is loaded.  " << endl;
        // double tframe = vTimestamps[ni];
        double tframe = LoadTimestamp(string(argv[3])+"/"+ ni_s + ".txt");
        // cout << "@Timestamp: " << tframe << endl;

        // use faked time stamp for debug
        tframe = t0+ti*0.2;
        vTimestamps.push_back(tframe);
       

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the image to the SLAM system
        cv::Mat Tcw = SLAM.TrackMonocular(im,tframe);

        // output the Tcw of current frame   by wmh
//        if (Tcw.rows > 0)
//        {
//            cout<< "Tcw:"
//                << Tcw.at<float>(0)
//                << Tcw.at<float>(1)
//                << Tcw.at<float>(2) << endl;
//        }

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
        // double T=0;
        // if(ni<nImages-1)
        //     T = vTimestamps[ni+1]-tframe;
        // else if(ni>0)
        //     T = tframe-vTimestamps[ni-1];

        double T = 0.001;  // T set to 0.1 (10Hz)when debug, 0.2 (5Hz)when Cozmo realtime test, 0 when full speed
        if(ttrack<T)
            usleep((T-ttrack)*1e6);

        // Save save results for coodinate study.  by wmh
        // SLAM.SaveKeyFrameInfo("./MapInfo/KFInfo_"+ni_s+".csv");
//        SLAM.SaveCurrentFrameInfo(string(argv[3])+"/CurFrmInfo.csv");
//        SLAM.SaveMapPointInfo("./MapInfo/MPInfo_"+ni_s+".csv");
        ni++;
        ti++;


        // To speed up SLAM "Training", sequence loop is introduced.

        // Loop over seq (End >= From > Term >= 0)
        // No Loop: 1 > 2 > 3 >...> End-1 > End
        // From-Term Loop: 1 > ...From -1 > From > Term > Term+1 > ... > From > ...
        // InverseLoop enabled:  1 > ...> From > -From > ... > -Term > Term > ... > From > ...

        if (isInverseLoop){
            if (ni == GoToFrom)
                ni = -GoToFrom;
            if (ni == -GoToTerm)
                ni = GoToTerm;
        }
        else{
            // Loop over seq
            if(ni == GoToFrom)
                ni = GoToTerm;
        }


        // Jump-back-and-loop strategy to avoid tracking lost
        cout << "Tracking State: " <<SLAM.GetTrackingState();
//        if (Tcw.rows == 0)
//            cout << " : Tracking lost~~~~~~~~~~~~~~~~~~~~~" << endl;
        if (isJumpBack) {
            switch (SLAM.GetTrackingState()) {
                case 1: // Not initialize yet
                    break;
                case 2:  // Tracking
                    LostCount = 0;
                    if (TrackStartN == 0)
                        TrackStartN = ni;
                case 3: // Tracking lost
                    LostCount++;
                    break;
                default:
                    break;
            }
            if (LostCount == MAXLOST_N) {
                ni = max(ni - JUMPBACK_N, TrackStartN);
                cout << "~~~~~~~~~~~~~~~~~~~~~~~  Jump back to: " << ni << endl;
                LostCount = 0;
            }
        }

    }

#ifdef COMPILEDWITHC11
    std::chrono::steady_clock::time_point tb = std::chrono::steady_clock::now();
#else
    std::chrono::monotonic_clock::time_point tb = std::chrono::monotonic_clock::now();
#endif

    // Wait keyboard input before end.   by wmh
    // TODO: timer should be paused here
    char input;
    cout << "All images are finished, end by input any key." << endl;
    cin >> input;

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    cout << "Total time to finish:";
    cout << std::chrono::duration_cast<std::chrono::duration<double> >(tb- ta).count() << endl;
    cout << "Max tracking-lost:" << MAXLOST_N << "; Jump-Back:" << JUMPBACK_N << endl;

    return 0;
}

double LoadTimestamp(const string &strFile)
{
    ifstream f;
    string s;
    stringstream ss;
    double atimestamp;
    f.open(strFile.c_str());
    getline(f,s);
    ss << s;
    ss >> atimestamp;
    f.close();
    // cout << "@TIme of:"<< strFile << " is "<< atimestamp << endl;
    return atimestamp;

}

void LoadImages(const string &strFile, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream f;
    f.open(strFile.c_str());

    // skip first three lines
    string s0;
    getline(f,s0);
    getline(f,s0);
    getline(f,s0);

    while(!f.eof())
    {
        string s;
        getline(f,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            string sRGB;
            ss >> t;
            vTimestamps.push_back(t);
            ss >> sRGB;
            vstrImageFilenames.push_back(sRGB);
        }
    }
}
