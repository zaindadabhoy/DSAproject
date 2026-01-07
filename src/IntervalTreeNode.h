#ifndef INTERVALTREENODE_H
#define INTERVALTREENODE_H

#include "Event.h"

class IntervalTreeNode
{
public:
    Event *event;
    int maxEnd;
    IntervalTreeNode *left, *right, *parent;

    IntervalTreeNode(Event *e)
    {
        event = e;
        maxEnd = e->endMinutes;
        left = right = parent = nullptr;
    }
};

#endif