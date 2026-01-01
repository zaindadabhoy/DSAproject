#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include "IntervalTreeNode.h"

class IntervalTree {
public:
    IntervalTreeNode* root;
    IntervalTree();
    void insert(Event* e);
    void remove(Event* e);
    std::vector<Event*> findOverlaps(Event* query);
    std::vector<Event*> getAllEventsSorted() const;
};

#endif