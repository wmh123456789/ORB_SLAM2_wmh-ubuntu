//
// Created by wmh on 18-1-10.
//

#include "QuadTree.h"
#include <iostream>
#include <list>

namespace WMH
{


QTNode::QTNode(int depth, float size, Point3f center, QTNode* parent, QTContent* content)
{



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

