//
// Created by wmh on 18-1-10.
//

#include "QuadTree.h"
#include <iostream>


namespace WMH
{


QTNode::QTNode(int depth, float size, Point3f* center, QTNode* parent, QTContent* content)
{
    this->mDepth = depth;
    this->mSize = size;
    this->mCenter = center;
    this->mParent= parent;
    this->mContent = content;

    mChildren[DL] = NULL;
    mChildren[DR] = NULL;
    mChildren[UR] = NULL;
    mChildren[UL] = NULL;

    mMaxX = center->x + size/2;
    mMinX = center->x - size/2;
    mMaxY = center->y ;         // Don't use Y in cozmo
    mMinY = center->y ;         // Don't use Y in cozmo
    mMaxZ = center->z + size/2;
    mMinZ = center->z - size/2;




    return ;


}

bool QTNode::isPointIn(Point3f* Pt){
    return 0;
}

vector<QTNode*> QTNode::QueryNeighborNode(NeighborOrientation Orientation){
    return NULL;
}

int QTNode::InitChildren(QTContent* content){
    return 0;
}


QuadTree::QuadTree()
{
    mpWord = "Hello WMH.";
}



void QuadTree::SayHello(const string &something)
{
    cout << mpWord << something << endl;
}

}

