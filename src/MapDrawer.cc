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

#include "MapDrawer.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include <pangolin/pangolin.h>
#include <mutex>

namespace ORB_SLAM2
{


MapDrawer::MapDrawer(Map* pMap, const string &strSettingPath):mpMap(pMap)
{
    cv::FileStorage fSettings(strSettingPath, cv::FileStorage::READ);

    mKeyFrameSize = fSettings["Viewer.KeyFrameSize"];
    mKeyFrameLineWidth = fSettings["Viewer.KeyFrameLineWidth"];
    mGraphLineWidth = fSettings["Viewer.GraphLineWidth"];
    mPointSize = fSettings["Viewer.PointSize"];
    mCameraSize = fSettings["Viewer.CameraSize"];
    mCameraLineWidth = fSettings["Viewer.CameraLineWidth"];

//    mHeadAngle = -25.0 /180*3.14; // the head angle of cozmo
    mHeadAngle = fSettings["Camera.Angle"];  // Edit this in yaml file  by wmh
}

// Draw the base plane by a grid
void MapDrawer::DrawBaseGrid()
{
    bool isDrawBasePlane = false;
    float GridStep = 0.1;    // the size of each cell in grid
    float GridSize = 3.0;    // the radius of the whole grid
    int   GridLineNum;  // half-lines number in one direction
    GridLineNum =  GridSize/GridStep;

    // Mark X(r),Y(g), Z(b) axis with 3 unit vectors
    glBegin(GL_LINES);
    glLineWidth(5);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.05, 0, 0);
    glVertex3f( 0, 0, 0);

    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f( 0, 0.08, 0);
    glVertex3f( 0, 0, 0);

    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f( 0, 0, 0.11);
    glVertex3f( 0, 0, 0);
    glEnd();

    // Draw grid by wmh
    glBegin(GL_LINES);
    glLineWidth(0.5f);
    glColor3f(0.8f,0.8f,0.8f);


    float y_z = tan(mHeadAngle);
    for (int i = 0; i < GridLineNum; i++)
    {
        // Draw grid in plane: y = tan(head_angle)*z
        // X direction lines
        glVertex3f(    GridSize,    GridStep*i*y_z,   GridStep*i);
        glVertex3f( -1*GridSize,    GridStep*i*y_z,   GridStep*i);
        glVertex3f(    GridSize, -1*GridStep*i*y_z,-1*GridStep*i);
        glVertex3f( -1*GridSize, -1*GridStep*i*y_z,-1*GridStep*i);

        // Z direction lines
        glVertex3f(   GridStep*i,   GridSize*y_z,    GridSize);
        glVertex3f(   GridStep*i,-1*GridSize*y_z, -1*GridSize);
        glVertex3f(-1*GridStep*i,   GridSize*y_z,    GridSize);
        glVertex3f(-1*GridStep*i,-1*GridSize*y_z, -1*GridSize);
    }
    glEnd();

    // Draw a plane by a rectangle
    if(isDrawBasePlane) {
        glBegin(GL_POLYGON);
        glColor3f(0.9f, 0.5f, 0.5f);
        glVertex3f(GridSize, 0.0f, 0.0f);
        glVertex3f(GridSize, 0.0f, GridSize);
        glVertex3f(0.0f, 0.0f, GridSize);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();
    }
    return;
}

void MapDrawer::DrawMapPoints()
{
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    const vector<MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();

    set<MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

    if(vpMPs.empty())
        return;

    // For all Map Points, in BLACK
    glPointSize(mPointSize /1.5);
    glBegin(GL_POINTS);
    glColor3f(0.0,0.0,0.0);

    for(size_t i=0, iend=vpMPs.size(); i<iend;i++)
    {
        if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
            continue;
        cv::Mat pos = vpMPs[i]->GetWorldPos();

    //  Hide the points under the base plane    by wmh
    // Distance to base plane: cos(theta)*y - sin(theta)*z
        if (cos(mHeadAngle)*pos.at<float>(1) - sin(mHeadAngle)*pos.at<float>(2) > 0.05)
            glColor3f(1.0, 1.0, 1.0);
        else
            glColor3f(0.0, 0.0, 0.0);

        glVertex3f(pos.at<float>(0), pos.at<float>(1), pos.at<float>(2));



    }
    glEnd();


    // For current ref Map Points, in RED
    glPointSize(mPointSize /1.5);
    glBegin(GL_POINTS);
    glColor3f(1.0,0.0,0.0);

    for(set<MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++)
    {
        if((*sit)->isBad())
            continue;
        cv::Mat pos = (*sit)->GetWorldPos();



        //  Hide the points under the base plane    by wmh
        // Distance to base plane: cos(theta)*y - sin(theta)*z
        if (cos(mHeadAngle)*pos.at<float>(1) - sin(mHeadAngle)*pos.at<float>(2) > 0.05)
            glColor3f(1.0, 1.0, 1.0);
        else
            glColor3f(1.0, 0.0, 0.0);
        glVertex3f(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2));

    }

    glEnd();
}

    // By wmh
    // If the cozmo's head angle is theta, than
    // the rotation vactor is R = [0, cos(theta), -sin(theta)]'
    // the base plane is R'X = 0, where X = [x,y,z]'
    // and if the coordinate of a Map point is P = [xp,yp,zp]'
    // then the distance to base plane is RP
    // the projection to base plane is Y = P - R'PR
void MapDrawer::DrawMapPointsProj()
{
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    const vector<MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();

    set<MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

    if(vpMPs.empty())
        return;

    float dist;

    // For all Map Points, in BLACK
    glPointSize(mPointSize /1.5);
    glBegin(GL_POINTS);
    glColor3f(0.0,0.0,0.0);

    for(size_t i=0, iend=vpMPs.size(); i<iend;i++)
    {
        if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
            continue;
        cv::Mat pos = vpMPs[i]->GetWorldPos();
        cv::Mat proj = pos.clone();
//        cv::Mat rot = cv::Mat::zeros(3,1,CV_32F);
        cv::Mat rot = (cv::Mat_<float>(3,1) << 0.0, cos(mHeadAngle), -sin(mHeadAngle));

        dist = pos.dot(rot);
        proj = pos - dist*rot;

        //  Hide the points under the base plane    by wmh
        // Distance to base plane: cos(theta)*y - sin(theta)*z
        if (cos(mHeadAngle)*pos.at<float>(1) - sin(mHeadAngle)*pos.at<float>(2) > 0.05)
            glColor3f(1.0, 1.0, 1.0);
        else
            glColor3f(0.5, 0.0, 0.5);

        glVertex3f(proj.at<float>(0), proj.at<float>(1), proj.at<float>(2));

    }
    glEnd();


    // For current ref Map Points, in RED
    glPointSize(mPointSize /1.5);
    glBegin(GL_POINTS);
    glColor3f(1.0,0.0,0.0);

    for(set<MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++)
    {
        if((*sit)->isBad())
            continue;
        cv::Mat pos = (*sit)->GetWorldPos();

        cv::Mat proj = pos.clone();
//        cv::Mat rot = cv::Mat::zeros(3,1,CV_32F);
        cv::Mat rot = (cv::Mat_<float>(3,1) << 0.0, cos(mHeadAngle), -sin(mHeadAngle));

        dist = pos.dot(rot);
        proj = pos - dist*rot;


        //  Hide the points under the base plane    by wmh
        // Distance to base plane: cos(theta)*y - sin(theta)*z
        if (cos(mHeadAngle)*pos.at<float>(1) - sin(mHeadAngle)*pos.at<float>(2) > 0.05)
            glColor3f(1.0, 1.0, 1.0);
        else
            glColor3f(0.5, 0.5, 0.0);
        glVertex3f(proj.at<float>(0),proj.at<float>(1),proj.at<float>(2));

    }

    glEnd();

}

void MapDrawer::DrawKeyFrames(const bool bDrawKF, const bool bDrawGraph)
{
    const float &w = mKeyFrameSize *0.5; // smaller cam in the window -- by wmh
    const float h = w*0.75;
    const float z = w*0.6;

    const vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();

    if(bDrawKF)
    {
        for(size_t i=0; i<vpKFs.size(); i++)
        {
            KeyFrame* pKF = vpKFs[i];
            cv::Mat Twc = pKF->GetPoseInverse().t();

            glPushMatrix();

            glMultMatrixf(Twc.ptr<GLfloat>(0));

            glLineWidth(mKeyFrameLineWidth);
            glColor3f(0.0f,0.0f,1.0f);
            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(w,h,z);
            glVertex3f(0,0,0);
            glVertex3f(w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,h,z);

            glVertex3f(w,h,z);
            glVertex3f(w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(-w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(w,h,z);

            glVertex3f(-w,-h,z);
            glVertex3f(w,-h,z);
            glEnd();

            glPopMatrix();
        }
    }

    if(bDrawGraph)
    {
        glLineWidth(mGraphLineWidth);
        glColor4f(0.0f,1.0f,0.0f,0.6f);
        glBegin(GL_LINES);

        for(size_t i=0; i<vpKFs.size(); i++)
        {
            // Covisibility Graph
            const vector<KeyFrame*> vCovKFs = vpKFs[i]->GetCovisiblesByWeight(100);
            cv::Mat Ow = vpKFs[i]->GetCameraCenter();
            if(!vCovKFs.empty())
            {
                for(vector<KeyFrame*>::const_iterator vit=vCovKFs.begin(), vend=vCovKFs.end(); vit!=vend; vit++)
                {
                    if((*vit)->mnId<vpKFs[i]->mnId)
                        continue;
                    cv::Mat Ow2 = (*vit)->GetCameraCenter();
                    glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                    glVertex3f(Ow2.at<float>(0),Ow2.at<float>(1),Ow2.at<float>(2));
                }
            }

            // Spanning tree
            KeyFrame* pParent = vpKFs[i]->GetParent();
            if(pParent)
            {
                cv::Mat Owp = pParent->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owp.at<float>(0),Owp.at<float>(1),Owp.at<float>(2));
            }

            // Loops
            set<KeyFrame*> sLoopKFs = vpKFs[i]->GetLoopEdges();
            for(set<KeyFrame*>::iterator sit=sLoopKFs.begin(), send=sLoopKFs.end(); sit!=send; sit++)
            {
                if((*sit)->mnId<vpKFs[i]->mnId)
                    continue;
                cv::Mat Owl = (*sit)->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owl.at<float>(0),Owl.at<float>(1),Owl.at<float>(2));
            }
        }

        glEnd();
    }
}

void MapDrawer::DrawCurrentCamera(pangolin::OpenGlMatrix &Twc)
{
    const float &w = mCameraSize *0.5;  // smaller cam in the window -- by wmh
    const float h = w*0.75;
    const float z = w*0.6;

    glPushMatrix();

#ifdef HAVE_GLES
        glMultMatrixf(Twc.m);
#else
        glMultMatrixd(Twc.m);
#endif

    glLineWidth(mCameraLineWidth);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(w,h,z);
    glVertex3f(0,0,0);
    glVertex3f(w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,h,z);

    glVertex3f(w,h,z);
    glVertex3f(w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(-w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);

    glVertex3f(-w,-h,z);
    glVertex3f(w,-h,z);
    glEnd();

    glPopMatrix();
}


void MapDrawer::SetCurrentCameraPose(const cv::Mat &Tcw)
{
    unique_lock<mutex> lock(mMutexCamera);
    mCameraPose = Tcw.clone();
}

void MapDrawer::GetCurrentOpenGLCameraMatrix(pangolin::OpenGlMatrix &M)
{
    if(!mCameraPose.empty())
    {
        cv::Mat Rwc(3,3,CV_32F);
        cv::Mat twc(3,1,CV_32F);
        {
            unique_lock<mutex> lock(mMutexCamera);
            Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
            twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
        }

        M.m[0] = Rwc.at<float>(0,0);
        M.m[1] = Rwc.at<float>(1,0);
        M.m[2] = Rwc.at<float>(2,0);
        M.m[3]  = 0.0;

        M.m[4] = Rwc.at<float>(0,1);
        M.m[5] = Rwc.at<float>(1,1);
        M.m[6] = Rwc.at<float>(2,1);
        M.m[7]  = 0.0;

        M.m[8] = Rwc.at<float>(0,2);
        M.m[9] = Rwc.at<float>(1,2);
        M.m[10] = Rwc.at<float>(2,2);
        M.m[11]  = 0.0;

        M.m[12] = twc.at<float>(0);
        M.m[13] = twc.at<float>(1);
        M.m[14] = twc.at<float>(2);
        M.m[15]  = 1.0;
    }
    else
        M.SetIdentity();
}

} //namespace ORB_SLAM
