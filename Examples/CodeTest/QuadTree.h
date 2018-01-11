//
// Created by wmh on 18-1-10.
//

#ifndef CODETEST_QUADTREE_H
#define CODETEST_QUADTREE_H



#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;


namespace WMH
{

typedef struct Point3f{
    float x;
    float y;
    float z;
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
    int MapPtN;
    std::vector<Point3f*> MapPts;
}QTContent;

class QuadTree;
class QTNode{
public:
    QTNode(int depth, float size, Point3f* center, QTNode* parent, QTContent* content);
    bool isPointIn(Point3f* Pt);
    vector<QTNode*> QueryNeighborNode(NeighborOrientation Orientation);
    int InitChildren(QTContent* content);


private:
    float mMinX,mMaxX,mMinY,mMaxY,mMinZ,mMaxZ;
    int mNodeId;  // For each layer DL:00 -> 0x0 DR:01->0x1 UL:10->0x2 UR:11->0x3
    Point3f* mCenter;
    float mSize;
    int mDepth,mMaxDepth;
    QuadTree* mTree;
    QTNode* mParent;
    QTNode* mChildren[4]; // 00 -> 0x0 01->0x1 10->0x2 11->0x3
    QTContent* mContent;
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
//  | 10 | 11 |             | 2  | 3  |
//  +----+----+             +----+----+
//  | 00 | 01 |             | 0  | 1  |
//  +----+----|  --> x      +----+----|  --> x
// x+ : right; y+ : down; z+ : front;
// 00 -> 0x0 01->0x1 10->0x2 11->0x3

class QuadTree {
public:
   QTNode* RootNode;


public:
    QuadTree();
    QTNode* SearchNeighbor();
    QTNode* QueryNodeById(int NodeId);
    QTNode* QueryPt();

    void SayHello(const string &someting);

private:
    string mpWord;

}; // class QuadTree


} //namespace WMH

#endif //CODETEST_QUADTREE_H
