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
    nLEFT  = -1,
    nRIGHT = 1,
    nUPPER = 2,
    nLOWER = -2
};

typedef struct QTContent{
    unsigned long MapPtN = 0;
    std::vector<Point3f*> MapPts;  //TODO: Need to initialize.
}QTContent;

// ID operation tools
// Only for Quadtree with oct-id
vector<int> ID2Vector(int id, bool isStartWithMSB);
vector<int> ID2Vector(int id);
int Vector2ID(vector<int> numbers);      //ONLY for MSB-first vector
int IDCalculater(int id, NeighborOrientation orientatio);

class QuadTree;
class QTNode{
public:
    QTNode(int depth, float size, Point3f center, QTNode* parent, QuadTree* tree);

    virtual ~QTNode();

    void setContent(const QTContent *content);
    const QTContent &getContent() const;

    void setCenter(const Point3f &mCenter);
    const Point3f &getCenter() const;

    void setBoundray();


    void setNodeId(int NodeId);
    int  getNodeId();


    bool isPointIn(Point3f Pt);
    QTNode* QueryNodeById(int NodeId);
    QTNode* QueryNeighborNode(NeighborOrientation Orientation);
    int InitChildren();
    void PrintNodeInfo(QTNode* node);
    void PrintNodeContent();
    void PrintChildren(bool isRecursion);



private:
    float mMinX,mMaxX,mMinY,mMaxY,mMinZ,mMaxZ;
    int mNodeId = 06;  // For each layer DL:00 -> 0x0 DR:01->0x1 UL:10->0x2 UR:11->0x3
    Point3f mCenter;
    float mSize;
    int mDepth;  // RootNode with MaxDepth, depth -1 for child
    const int MAPPOINTS_MAX = 5; // Maximum capacity of map points
    QuadTree* mTree;
    QTNode* mParent;
//    QTNode  mChildren[4]; // 00 -> 0x0 01->0x1 10->0x2 11->0x3
    vector<QTNode*> mChildren;
    bool isHasChild;
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
// 00 -> 00 01->01 10->02 11->03   id is oct

class QuadTree {
public:


public:
//    QuadTree();
    QuadTree(float size, int MaxDepth,vector<Point3f*> MapPoints);

    virtual ~QuadTree();

    int getMaxDepth();
    QTNode* getRootNode();
    QTNode* QueryNodeById(int NodeId);
    QTNode* QueryPt();
    void PrintRootNode();
    void PrintTree();

    void SayHello(const string &someting);
    QTContent* FillContentWithMapPoints(vector<Point3f*> MapPoints);

private:
    QTNode* mRootNode;
    string mpWord;
    int mMaxDepth;

}; // class QuadTree


} //namespace WMH

#endif //CODETEST_QUADTREE_H
