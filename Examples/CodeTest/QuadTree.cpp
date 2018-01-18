//
// Created by wmh on 18-1-10.
//

#include "QuadTree.h"
#include <iostream>


namespace WMH {
    // Turn the Oct id into vector
    // StartWithMSB: 0abcd -> {a,b,c,d}
    // StartWithLSB: 0abcd -> {d,c,b,a}
    vector<int> ID2Vector(int id, bool isStartWithMSB){
        vector<int> numbers;
        while(id > 0){
            if(isStartWithMSB)
                numbers.insert(numbers.begin(),id%8);
            else
                numbers.insert(numbers.end(),id%8);
            id = id/8;
        }
        return numbers;
    }

    vector<int> ID2Vector(int id) {
        return ID2Vector(id,true);
    }

    //ONLY for MSB-first vector
    int Vector2ID(vector<int> numbers){
        int id = 00;
        for(int i = 0; i < numbers.size(); i++){
            id = id*8 + numbers[i];
        }
        return id;
    }

    // toLeft: -1, toRight: +1, toLower: -2, toUpper: +2
    int IDCalculater(int id, NeighborOrientation orientation){
        int target = 06;
        int LSB = id%8;
        vector<int> HighBits;

        if (id + orientation < 0 || id + orientation > 3 || id*orientation == 1 || id*orientation == -2){
            cout << "Neighbor is out of the tree" << endl;
            return -999;
        }

        // TODO: simplify the if condition
        if (abs(orientation)==2){       // ToLower or ToUpper
            if (LSB + orientation < 0 || LSB + orientation > 3)
                target = IDCalculater(id/8,orientation)*8 + LSB - orientation;
            else
                target = id + orientation;
        }
        else if(abs(orientation)==1){    // ToLeft or ToRight
            // 1+1, 2-1,0-1,3+1
            if (LSB + orientation < 0 || LSB + orientation > 3 || LSB*orientation == 1 || LSB*orientation == -2){
                target = IDCalculater(id/8,orientation)*8 + LSB - orientation;
            } else{
                target = id + orientation;
            }
        }
        else{
            cout << "Invalid Operation for query neighbor. only +-1 or -+2 is acceptable."<< endl;
            return id;
        }
        return target;
    }

    QTNode::QTNode(int depth, float size, Point3f center, QTNode* parent, QuadTree* Tree) {
        mDepth = depth;
        mSize = size;
        mCenter = center;
        mParent = parent;
        mTree = Tree;
        isHasChild = false;
        setBoundray();
    }

    QTNode::~QTNode() {
        //TODO  delete content with all points
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

    QTNode* QTNode::QueryNeighborNode(NeighborOrientation Orientation) {
        if (mNodeId == 05){
            cout << "Cannot query neighbor for root node." << endl;
            return nullptr;
        }
        int PosInParent = mNodeId%8;
        QTNode* node;
        int FoundNeighbor = 0;
        int i_num = 0;  // bit pos of ID
        int NeighborId = 06;
        vector<int> num = ID2Vector(mNodeId,false); // start with LSB

        NeighborId = IDCalculater(mNodeId,Orientation);
//        while (FoundNeighbor == 0 ){
//            // toLeft: -1, toRight: +1, toLower: -2, toUpper: +2
//
//            switch (Orientation){
//                case nLEFT:{
//                    NeighborId = IDCalculater(mNodeId,-1);
//                    break;
//                }
//                case nRIGHT:{
//                    NeighborId = IDCalculater(mNodeId,-1);
//                    break;
//                }
//                case nUPPER:{
//                    break;
//                }
//                case nLOWER:{
//                    break;
//                }
//
//            }// switch
//        }



//        switch(Orientation){
//            case nLEFT:         // search left neighbor
//                switch(PosInParent){
//                    case 00:    // for left-child
//                    case 02:{
//                        node = mParent->QueryNeighborNode(nLEFT);
//                        if (node!= nullptr)
//                            node = node->mChildren[PosInParent+1];
//                        else
//                            return nullptr;
//                        break;
//                    }
//                    case 01:    // for right-child
//                    case 03:{
//                        node = mParent->mChildren[PosInParent-1];
//                    }
//                    default:
//                        return nullptr;
//
//
//                }
//
//
//        }
//        return nullptr;
    }

    int QTNode::InitChildren() {
        if (mContent.MapPtN <= MAPPOINTS_MAX){
//            cout<< "No enough points in node, initialization is stopped.";
            return 0;
        }

        Point3f* Pt;
        Point3f center;
        int iCh = 00;  // oct index
        QTContent* ContDL = new QTContent;
        QTContent* ContDR = new QTContent;
        QTContent* ContUL = new QTContent;
        QTContent* ContUR = new QTContent;

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
        QTNode* child;
        for (iCh = 0; iCh < 4; iCh++){
            child = new QTNode(mDepth - 1, mSize / 2, mCenter, this, mTree);
            child->setNodeId(07);
            mChildren.insert(mChildren.begin(),child);
        }

        //TODO: Allocate: NodeId, Tree pointer, Neighbor for children
        center.y = mCenter.y;  // y is not changed in Cozmo.
        center.x = mCenter.x - mSize / 4;
        center.z = mCenter.z - mSize / 4;
        mChildren[DL]->setCenter(center);
        mChildren[DL]->setContent(ContDL);


        center.x = mCenter.x + mSize/4;
        center.z = mCenter.z - mSize/4;
        mChildren[DR]->setCenter(center);
        mChildren[DR]->setContent(ContDR);

        center.x = mCenter.x - mSize/4;
        center.z = mCenter.z + mSize/4;
        mChildren[UL]->setCenter(center);
        mChildren[UL]->setContent(ContUL);

        center.x = mCenter.x + mSize/4;
        center.z = mCenter.z + mSize/4;
        mChildren[UR]->setCenter(center);
        mChildren[UR]->setContent(ContUR);

        isHasChild = true;

        for(iCh = 0; iCh < 4; iCh++){
            mChildren[iCh]->setBoundray();
            // Set OCT id
            // in each level: 00~03: DL,DR,UL,UR
            // 05: Root of the tree, 06: default
            if (mNodeId == 05)    // if the root node
                mChildren[iCh]->setNodeId(iCh);
            else
                mChildren[iCh]->setNodeId(iCh + mNodeId*8);
            // Init the Children in next level.
            if (mChildren[iCh]-> mDepth > 0)
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



    QTNode* QTNode::QueryNodeById(int NodeId) {
        vector<int> IDs;
        QTNode* ptr = mTree->getRootNode();
//        cout<< "Access:" <<oct <<NodeId << endl;

//        while (NodeId > 0){
//            IDs.insert(IDs.begin(),NodeId%8);
//            NodeId = NodeId/8;
//        }
        IDs = ID2Vector(NodeId);

        for(int i=0; i < IDs.size();i++){
            if (!ptr->isHasChild){
                cout << "Node" << NodeId << "is not exit."<< endl;
                return nullptr;
            }
            ptr = ptr->mChildren[IDs[i]];
        }
        return ptr;
    }


    void QTNode::PrintNodeContent() {
        if (mContent.MapPtN == 0){
            cout << "Noting is found." << endl;
        }
        else{
            cout << mContent.MapPtN << " points are found:" << endl;
            for (int pi = 0;pi < mContent.MapPtN;pi++){
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

    void QTNode::setNodeId(int id) {
        mNodeId = id;
    }

    int QTNode::getNodeId()  {
        return mNodeId;
    }



    void QTNode::PrintChildren(bool isRecursion) {
        cout << "Find " << dec << mContent.MapPtN  << " points;"<<endl;
        if (isHasChild){
            for(int iCh = 0; iCh < 4; iCh++) {
                for (int iLv = mDepth; iLv < mTree -> getMaxDepth(); iLv++)
                    cout<< "   ";
                cout << "|-in node "<< oct << mChildren[iCh]->mNodeId << ": ";
                if (isRecursion){
                    mChildren[iCh]-> PrintChildren(isRecursion);
                }
                else{
                    cout << dec << mChildren[iCh]->mContent.MapPtN << " points." << endl;
                }

            }
        }
    }


    QuadTree::QuadTree(float size, int MaxDepth, vector<Point3f *> MapPoints) {
        mMaxDepth = MaxDepth;

        Point3f center;
        center.x = 0.0;
        center.y = 0.0;
        center.z = 0.0;

        QTContent* content = FillContentWithMapPoints(MapPoints);
        mRootNode = new QTNode(mMaxDepth, size, center, NULL, this);
        mRootNode->setNodeId(05);
        mRootNode->setContent(content);
        mRootNode->InitChildren();
        delete content; //TODO: need to confirm


    }

    QuadTree::~QuadTree() {
        delete mRootNode;

    }

    QTNode *QuadTree::getRootNode() {
        return mRootNode;
    }

    QTContent* QuadTree::FillContentWithMapPoints(vector<Point3f *> MapPoints) {

        QTContent* content = new QTContent;
        content->MapPts = MapPoints;
        content->MapPtN = MapPoints.size();

        return content;
    }

    QTNode* QuadTree::QueryNodeById(int NodeId) {
        return mRootNode->QueryNodeById(NodeId);
    }


    void QuadTree::PrintRootNode() {
        mRootNode->PrintNodeInfo(mRootNode);
        mRootNode->PrintNodeContent();

    }

    void QuadTree::SayHello(const string &something) {
        cout << mpWord << something << endl;
    }


    void QuadTree::PrintTree() {
        mRootNode->PrintChildren(true); // Print children with recursion

//        QTNode* node = mRootNode->QueryNodeById(0303);
//        cout << "Get node:" << node->getNodeId() << endl;
    }

    int QuadTree::getMaxDepth() {
        return mMaxDepth;
    }

}

