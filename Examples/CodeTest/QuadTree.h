//
// Created by wmh on 18-1-10.
//

#ifndef CODETEST_QUADTREE_H
#define CODETEST_QUADTREE_H



#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;


namespace WMH
{

typedef struct Point3f{
    float x;
    float y;
    float z;
}Point3f;

typedef struct QTContent{
    int MapPtN;
    std::vector<Point3f*> MapPts;
}QTContent;

class QTNode{
public:
    QTNode();
    bool isPointIn(Point3f* Pt);
private:
    float minX,maxX,minY,maxY;
    int NodeId;
    Point3f* center;
    float size;
    int depth;
    QTNode Children[4];
    QTContent content;
    QTNode* LeftNb,RightNb,UpperNb,LowerNb;
    QTNode* NextChild;

}; // class QTNode

//the Corrdinate is same with cozmo's cam
//  z
//  ^
//  +----+----+
//  | 10 | 11 |
//  +----+----+
//  | 00 | 01 |
//  +----+----|  --> x
// x+ : right; y+ : down; z+ : front;

    class QuadTree {
public:
   QTNode* RootNode;


public:
    QuadTree();
    QTNode* SearchNeighbor();
    int QueryPt();

    void SayHello(const string &someting);

private:
    string mpWord;

}; // class QuadTree


} //namespace WMH

#endif //CODETEST_QUADTREE_H
