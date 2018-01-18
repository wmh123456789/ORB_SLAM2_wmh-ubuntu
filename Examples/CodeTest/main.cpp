#include <iostream>
#include "QuadTree.h"
#include <string>
#include "FaceDetection.h"

using namespace std;
using namespace WMH;

int main() {
    int i=0;
//    int a = 03020;
//    int x = 0;
//    cout<< oct <<a <<endl;
//    while(a>0){
//        cout<<a%8<<endl;
//        a = a/8;
//    }

//    std::cout << "Hello, World!" << std::endl;
//
//    WMH::QuadTree QT;
//    QT.SayHello("You are handsome boy");

    vector<WMH::Point3f*> Points;

//    float xz[10][2] = {{1.0,1.0},       // UR
//                       {2.0,2.0},       // UR
//                       {-2.0,-2.0},     // DL
//                       {1.1,1.3},       // UR
//                       {1.5,1.6},       // UR
//                       {3.2,1.0},       // UR
//                       {10.0,20.3},     // out
//                       {9.0,30.1},      // out
//                       {-15.3,2.3},     // UL
//                       {25.1,-4.4}};    // out
    float xz[11][2] = {{1.0,1.0},       // UR
                       {6.0,5.0},       // UR
                       {4.0,11.0},      // UR
                       {12.1,4.3},      // UR
                       {-4.5,4.6},      // UL
                       {-14.2,12.0},    // UL
                       {-2.0,-2.3},     // DL
                       {-12.0,-4.1},    // DL
                       {4.3,-4.3},      // DR
                       {12.1,-5.4},
                       {5.1,2.3} };    // DR

    for( i = 0; i<11; i++){
        WMH::Point3f* pt = new Point3f;
        pt->x = xz[i][0];
        pt->y = 0.0;
        pt->z = xz[i][1];
        Points.insert(Points.end(),pt);
    }

    WMH::QuadTree QT2 = QuadTree(32.0,6,Points);

//    QT2.PrintRootNode();
    QT2.PrintTree();

    int id = 0123456;
    cout<< id <<endl;
    vector<int> nums = ID2Vector(id);
    for(i=0;i<nums.size();i++)
        cout << nums[i]<< endl;

    cout << "Inverse: " << Vector2ID(nums)<< endl;

    cout << "Calc neighbor ID:" << IDCalculater(013,nRIGHT)<<endl;

    FaceDetection face;
//    face.Detect();
    face.DetectCam();
    return 0;
};

