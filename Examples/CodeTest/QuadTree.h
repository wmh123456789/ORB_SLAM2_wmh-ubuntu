//
// Created by wmh on 18-1-10.
//

#ifndef CODETEST_QUADTREE_H
#define CODETEST_QUADTREE_H



#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

//TODO: change this path when merge the project
//#include "../../include/MapPoint.h"

using namespace std;
namespace WMH
{

typedef struct Point3f{
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
}Point3f;

enum NodePos{
    DL = 0,
    DR = 1,
    UL = 2,
    UR = 3
};

enum NeighborOrientation{
    nLEFT  = 0,
    nRIGHT = 1,
    nUPPER = 2,
    nLOWER = 3
};

typedef struct QTContent{
    unsigned long MapPtN = 0;
    std::vector<Point3f*> MapPts;
}QTContent;

class QuadTree;
class QTNode{
public:
    QTNode(int depth, float size, Point3f center, QTNode* parent);
    void setNodeId(int id);
    int  getNodeId();

    void setContent(const QTContent &mContent);
    const QTContent &getContent() const;

    void setCenter(const Point3f &mCenter);
    const Point3f &getCenter() const;


    bool isPointIn(Point3f Pt);
    void QueryNeighborNode(NeighborOrientation Orientation, QTNode* Node);
    int InitChildren(QTContent* content);
    void PrintNodeInfo();



private:
    float mMinX,mMaxX,mMinY,mMaxY,mMinZ,mMaxZ;
    int mNodeId;  // For each layer DL:00 -> 0x0 DR:01->0x1 UL:10->0x2 UR:11->0x3
    Point3f mCenter;
    float mSize;
    int mDepth,mMaxDepth;  // RootNode with MaxDepth, depth -1 for child
    QuadTree* mTree;
    QTNode* mParent;
//    QTNode  mChildren[4]; // 00 -> 0x0 01->0x1 10->0x2 11->0x3
    vector<QTNode> mChildren;
    QTContent mContent;
    QTNode* mLeftNb;
    QTNode* mRightNb;
    QTNode* mUpperNb;
    QTNode* mLowerNb;
    QTNode* mNextChild;

}; // class QTNode

//the Corrdinate is same with cozmo's cam
//  z                       z
//  ^                       ^
//  +----+----+             +----+----+
//  | 10 | 11 |             |2:UL|3:UR|
//  +----+----+             +----+----+
//  | 00 | 01 |             |0:DL|1:DR|
//  +----+----+  --> x      +----+----+  --> x
// x+ : right; y+ : down; z+ : front;
// 00 -> 0x0 01->0x1 10->0x2 11->0x3

class QuadTree {
public:


public:
    QuadTree();
    QuadTree(int MaxDepth,vector<Point3f*> MapPoints);

    QTNode* QueryNodeById(int NodeId);
    QTNode* QueryPt();

    void SayHello(const string &someting);
    QTContent FillContentWithMapPoints(vector<Point3f*> MapPoints);

private:
    QTNode* RootNode;
    string mpWord;

}; // class QuadTree


} //namespace WMH

#endif //CODETEST_QUADTREE_H
