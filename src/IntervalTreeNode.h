#ifndef INTERVALTREENODE_H
#define INTERVALTREENODE_H

#include "Event.h"
#include <vector>

class IntervalTreeNode {
public:
    Event* event;
    int maxEnd;
    IntervalTreeNode *left, *right, *parent;

    IntervalTreeNode(Event* e);
    
    static IntervalTreeNode* insert(IntervalTreeNode* root, Event* e);
    static IntervalTreeNode* remove(IntervalTreeNode* root, Event* e);
    static void findOverlaps(IntervalTreeNode* root, Event* query, std::vector<Event*>& out);
    static void inorderCollect(IntervalTreeNode* root, std::vector<Event*>& out);
    static void updateMaxEndIterative(IntervalTreeNode* node);
};

#endif