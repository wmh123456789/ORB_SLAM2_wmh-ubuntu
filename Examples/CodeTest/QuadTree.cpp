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
        isHasChild = false;
        setBoundray();
    }

    bool QTNode::isPointIn(Point3f Pt) {
        int flag = 0;
        if (Pt.x <= mMaxX && Pt.x >= mMinX)
            flag++;
        if (Pt.y <= mMaxY && Pt.y >= mMinY)
            flag++;
        if (Pt.z <= mMaxZ && Pt.z >= mMinZ)
            flag++;

        if (flag == 3)
            return true;
        else
            return false;
    }

    void QTNode::QueryNeighborNode(NeighborOrientation Orientation, QTNode *Node) {
        return;
    }

    int QTNode::InitChildren() {
        Point3f* Pt;
        Point3f center;
        int iCh = 0;
        QTContent* ContDL = new QTContent;
        QTContent* ContDR = new QTContent;
        QTContent* ContUL = new QTContent;
        QTContent* ContUR = new QTContent;


        if (mContent.MapPtN <= MAPPOINTS_MAX){
            cout<< "No enough points in node, initialization is stopped.";
            return 0;
        }

        // Loop over all points
        for (int i = 0; i < mContent.MapPts.size(); i++) {
            Pt =  mContent.MapPts[i];
            // Allocate points into Quad-Area
            if (isPointIn(*Pt)) {
                if (Pt->x < mCenter.x) {         // Left
                    if (Pt->z < mCenter.z)       // Down
                        ContDL->MapPts.insert(ContDL->MapPts.begin(), Pt);
                    else                        // Up
                        ContUL->MapPts.insert(ContUL->MapPts.begin(), Pt);
                } else {                          // Right
                    if (Pt->z < mCenter.z)       // DOWN
                        ContDR->MapPts.insert(ContDR->MapPts.begin(), Pt);
                    else                       // Up
                        ContUR->MapPts.insert(ContUR->MapPts.begin(), Pt);
                }
            }
        }
        ContDL->MapPtN = ContDL->MapPts.size();
        ContDR->MapPtN = ContDR->MapPts.size();
        ContUL->MapPtN = ContUL->MapPts.size();
        ContUR->MapPtN = ContUR->MapPts.size();

        // Calculate the sub-center and set content for children
        // TODO: Check depth before new children
        for (iCh = 0; iCh < 4; iCh++){
            QTNode* child = new QTNode(mDepth - 1, mSize / 2, mCenter, this);
            mChildren.insert(mChildren.begin(),child);
        }

        //TODO: Allocate: NodeId, Tree pointer, Neighbor for children
        center.y = mCenter.y;  // y is not changed in Cozmo.
        center.x = mCenter.x - mSize / 4;
        center.z = mCenter.z - mSize / 4;
        mChildren[DL]->setCenter(center);
//        mChildren[DL]->setBoundray();
        mChildren[DL]->setContent(ContDL);


        center.x = mCenter.x + mSize/4;
        center.z = mCenter.z - mSize/4;
        mChildren[DR]->setCenter(center);
//        mChildren[DR]->setBoundray();
        mChildren[DR]->setContent(ContDR);

        center.x = mCenter.x - mSize/4;
        center.z = mCenter.z + mSize/4;
        mChildren[UL]->setCenter(center);
//        mChildren[UL]->setBoundray();
        mChildren[UL]->setContent(ContUL);

        center.x = mCenter.x + mSize/4;
        center.z = mCenter.z + mSize/4;
        mChildren[UR]->setCenter(center);
//        mChildren[UR]->setBoundray();
        mChildren[UR]->setContent(ContUR);

        isHasChild = true;

        for(iCh = 0; iCh < 4; iCh++){
            mChildren[iCh]->setBoundray();
            mChildren[iCh]->InitChildren();
        }

        return 1;
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

    void QTNode::setBoundray() {
        mMaxX = mCenter.x + mSize / 2;
        mMinX = mCenter.x - mSize / 2;
        mMaxY = mCenter.y;         // Don't use Y in cozmo
        mMinY = mCenter.y;         // Don't use Y in cozmo
        mMaxZ = mCenter.z + mSize / 2;
        mMinZ = mCenter.z - mSize / 2;
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

    void QTNode::PrintChilden(bool isRecursion) {
        if (isHasChild)
            cout <<
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
        RootNode->InitChildren();
        delete content; //TODO: need to confirm
    }

    QuadTree::~QuadTree() {
        delete RootNode;

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


    void QuadTree::PrintTree() {
        RootNode->PrintChilden(true); // Print children with recursion
    }

}

