//
// Created by wmh on 18-1-10.
//

#include "QuadTree.h"
#include <iostream>


namespace WMH {
    QTNode::QTNode(int depth, float size, Point3f center, QTNode *parent) {
        mDepth = depth;
        mSize = size;
        mCenter = center;
        mParent = parent;
//        mChildren = NULL;
//        mChildren[DL] = NULL;
//        mChildren[DR] = NULL;
//        mChildren[UR] = NULL;
//        mChildren[UL] = NULL;

        mMaxX = center.x + size / 2;
        mMinX = center.x - size / 2;
        mMaxY = center.y;         // Don't use Y in cozmo
        mMinY = center.y;         // Don't use Y in cozmo
        mMaxZ = center.z + size / 2;
        mMinZ = center.z - size / 2;

    }

    bool QTNode::isPointIn(Point3f Pt) {
        return 1;
    }

    void QTNode::QueryNeighborNode(NeighborOrientation Orientation, QTNode *Node) {
        return;
    }

    int QTNode::InitChildren(QTContent *content) {
        Point3f Pt;
        Point3f center;
        QTContent ContDL, ContDR, ContUL, ContUR;

        // Loop over all points
        for (int i = 0; i < content->MapPts.size(); i++) {
            Pt = *content->MapPts[i];
            // Allocate points into Quad-Area
            if (isPointIn(Pt)) {
                if (Pt.x < mCenter.x) {         // Left
                    if (Pt.z < mCenter.z)       // Down
                        ContDL.MapPts.insert(ContDL.MapPts.begin(), &Pt);
                    else                       // Up
                        ContUL.MapPts.insert(ContUL.MapPts.begin(), &Pt);
                } else {                          // Right
                    if (Pt.z > mCenter.z)       // Down
                        ContDR.MapPts.insert(ContDR.MapPts.begin(), &Pt);
                    else                       // Up
                        ContUR.MapPts.insert(ContUR.MapPts.begin(), &Pt);
                }
            }
        }
        ContDL.MapPtN = ContDL.MapPts.size();
        ContDR.MapPtN = ContDR.MapPts.size();
        ContUL.MapPtN = ContUL.MapPts.size();
        ContUR.MapPtN = ContUR.MapPts.size();

        // Calculate the sub-center and set content for children
        // TODO: Check depth before new children
        mChildren.insert(mChildren.begin(), 4, QTNode(mDepth - 1, mSize / 2, mCenter, this));

        center.y = mCenter.y;  // y is not changed in Cozmo.
        center.x = mCenter.x - mSize / 4;
        center.z = mCenter.z - mSize / 4;
        mChildren[DL].setCenter(center); //TODO
        mChildren[DL].setContent(&ContDL);
//
//        center.x = mCenter.x + mSize/4;
//        center.z = mCenter.z - mSize/4;
//        mChildren[DR] = QTNode(mDepth-1,mSize/2,center,this);
//        mChildren[DR].setContent(ContDR);
//
//        center.x = mCenter.x - mSize/4;
//        center.z = mCenter.z + mSize/4;
//        mChildren[UL] = QTNode(mDepth-1,mSize/2,center,this);
//        mChildren[UL].setContent(ContUL);
//
//        center.x = mCenter.x + mSize/4;
//        center.z = mCenter.z + mSize/4;
//        mChildren[UR] = QTNode(mDepth-1,mSize/2,center,this);
//        mChildren[UR].setContent(ContUR);

        return 0;
    }

    void QTNode::PrintNodeInfo(QTNode* node) {
        int id = node->getNodeId();
        cout << "NodeID:" << id << endl;
        cout << "Center X: " << node->mCenter.x << endl;
        cout << "Center Z: " << node->mCenter.z << endl;
        cout << "Depth:" << node->mDepth << "; Size:" << mSize << endl;
        cout << "X: From " << node->mMinX << " to " << mMaxX << endl;
        cout << "Z: From " << node->mMinZ << " to " << mMaxZ << endl;
        cout << "NodeID:" << node->getNodeId() << endl;
    }

    void QTNode::PrintNodeContent() {
        if (mContent.MapPtN == 0){
            cout << "Noting is found." << endl;
        }
        else{
            cout << mContent.MapPtN << " points are found:" << endl;
            for (int pi=0;pi < mContent.MapPtN;pi++){
                cout << "No." << pi << ": ";
                cout << "x=" << mContent.MapPts[pi]->x << ", ";
                cout << "z=" << mContent.MapPts[pi]->z << ";" << endl;
            }
        }
    }




    void QTNode::setContent(const QTContent* content) {
        QTNode::mContent = *content;
    }

    const QTContent &QTNode::getContent() const {
        return mContent;
    }

    void QTNode::setCenter(const Point3f &mCenter) {
        QTNode::mCenter = mCenter;
    }

    const Point3f &QTNode::getCenter() const {
        return mCenter;
    }

    void QTNode::setNodeId(int mNodeId) {
        QTNode::mNodeId = mNodeId;
    }

    int QTNode::getNodeId() const {
        return mNodeId;
    }

    QTNode::~QTNode() {
        //TODO  delete content with all points
    }


    QuadTree::QuadTree(float size, int MaxDepth, vector<Point3f *> MapPoints) {

        Point3f center;
        center.x = 0.0;
        center.y = 0.0;
        center.z = 0.0;

        QTContent* content = FillContentWithMapPoints(MapPoints);
        RootNode = new QTNode(MaxDepth, size, center, NULL);
        RootNode->setNodeId(-1);
        RootNode->setContent(content);
        delete content; //TODO: need to confirm
    }

    QTContent* QuadTree::FillContentWithMapPoints(vector<Point3f *> MapPoints) {

        QTContent* content = new QTContent;
        content->MapPts = MapPoints;
        content->MapPtN = MapPoints.size();

        return content;
    }


    void QuadTree::PrintRootNode() {
        RootNode->PrintNodeInfo(RootNode);
        RootNode->PrintNodeContent();
    }

    void QuadTree::SayHello(const string &something) {
        cout << mpWord << something << endl;
    }

    QuadTree::~QuadTree() {
        delete RootNode;

    }

}

