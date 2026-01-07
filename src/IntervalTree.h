#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include "IntervalTreeNode.h"
#include <vector>

class IntervalTree
{
public:
    IntervalTreeNode *root;

    IntervalTree();
    ~IntervalTree();

    void insert(Event *e);
    void remove(Event *e);
    std::vector<Event *> findOverlaps(Event *query);
    std::vector<Event *> getAllEventsSorted() const;

private:
    void updateMaxEnd(IntervalTreeNode *node);
    void deleteTree(IntervalTreeNode *node);
};

#endif