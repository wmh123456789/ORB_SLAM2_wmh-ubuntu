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

typedef struct QTContent{
    int MapPtN;
    std::vector<Point3f*> MapPts;
}QTContent;

class QTNode{
public:
    QTNode(int depth, float size, Point3f center, QTNode* parent, QTContent* content);
    bool isPointIn(Point3f* Pt);
private:
    float minX,maxX,minY,maxY;
    int NodeId;  // DL:00 -> 0x0 DR:01->0x1 UL:10->0x2 UR:11->0x3
    Point3f* center;
    float size;
    int depth;
    QTNode* Children[4];
    QTContent content;
    QTNode* LeftNb;
    QTNode* RightNb;
    QTNode* UpperNb;
    QTNode* LowerNb;
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
