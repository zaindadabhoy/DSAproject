#ifndef INTERVALTREE_H
#define INTERVALTREE_H
#include "IntervalTreeNode.h"
class IntervalTree {
public:
    IntervalTreeNode* root;
    IntervalTree() { root = NULL; }
    void insert(Event* e) { root = IntervalTreeNode::insert(root, e); }
    void remove(Event* e) { root = IntervalTreeNode::remove(root, e); }
    std::vector<Event*> findOverlaps(Event* query) {
        std::vector<Event*> res;
        IntervalTreeNode::findOverlaps(root, query, res);
        return res;
    }
    std::vector<Event*> getAllEventsSorted() const {
        std::vector<Event*> res;
        IntervalTreeNode::inorderCollect(root, res);
        return res;
    }
};
#endif