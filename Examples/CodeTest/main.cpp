#include <iostream>
#include "QuadTree.h"
#include <string>

using namespace std;
using namespace WMH;

int main() {
    int a = 0x00;
    std::cout << "Hello, World!" << std::endl;
//
//    WMH::QuadTree QT;
//    QT.SayHello("You are handsome boy");

    vector<Point3f*> Points;

    float xz[10][2] = {{1.0,1.0},
                       {2.0,2.0},
                       {-2.0,-2.0},
                       {1.1,1.3},
                       {1.5,1.6},
                       {3.2,1.0},
                       {10.0,20.3},
                       {9.0,30.1},
                       {-15.3,2.3},
                       {25.1,-4.4}};


    for( int i = 0; i<10; i++){
        Point3f* pt = new Point3f;
        pt->x = xz[i][0];
        pt->y = 0.0;
        pt->z = xz[i][1];
        Points.insert(Points.end(),pt);
    }

    WMH::QuadTree QT2 = QuadTree(32.0,6,Points);

    QT2.PrintRootNode();


    return 0;
};

