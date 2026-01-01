#include "IntervalTree.h"

IntervalTree::IntervalTree() { root = NULL; }
void IntervalTree::insert(Event* e) { root = IntervalTreeNode::insert(root, e); }
void IntervalTree::remove(Event* e) { root = IntervalTreeNode::remove(root, e); }
std::vector<Event*> IntervalTree::findOverlaps(Event* query) {
    std::vector<Event*> res;
    IntervalTreeNode::findOverlaps(root, query, res);
    return res;
}
std::vector<Event*> IntervalTree::getAllEventsSorted() const {
    std::vector<Event*> res;
    IntervalTreeNode::inorderCollect(root, res);
    return res;
}