#include "IntervalTreeNode.h"
#include <algorithm>
#include <stack>

IntervalTreeNode::IntervalTreeNode(Event* e) {
    event = e;
    maxEnd = e->endMinutes;
    left = right = parent = NULL;
}

void IntervalTreeNode::updateMaxEndIterative(IntervalTreeNode* node) {
    while (node != NULL) {
        int currentMax = node->event->endMinutes;
        if (node->left) currentMax = std::max(currentMax, node->left->maxEnd);
        if (node->right) currentMax = std::max(currentMax, node->right->maxEnd);
        node->maxEnd = currentMax;
        node = node->parent;
    }
}

IntervalTreeNode* IntervalTreeNode::insert(IntervalTreeNode* root, Event* e) {
    IntervalTreeNode* newNode = new IntervalTreeNode(e);
    if (root == NULL) return newNode;
    IntervalTreeNode* curr = root;
    IntervalTreeNode* prev = NULL;
    while (curr != NULL) {
        prev = curr;
        if (e->startMinutes < curr->event->startMinutes) curr = curr->left;
        else curr = curr->right;
    }
    newNode->parent = prev;
    if (e->startMinutes < prev->event->startMinutes) prev->left = newNode;
    else prev->right = newNode;
    updateMaxEndIterative(newNode);
    return root;
}

IntervalTreeNode* IntervalTreeNode::remove(IntervalTreeNode* root, Event* e) {
    IntervalTreeNode *curr = root, *target = NULL;
    while (curr != NULL) {
        if (curr->event == e) { target = curr; break; }
        if (e->startMinutes < curr->event->startMinutes) curr = curr->left;
        else curr = curr->right;
    }
    if (!target) return root;
    IntervalTreeNode *toDelete = target;
    IntervalTreeNode *p = NULL;
    if (target->left && target->right) {
        IntervalTreeNode* succ = target->right;
        while (succ->left) succ = succ->left;
        target->event = succ->event;
        toDelete = succ;              
    }
    p = toDelete->parent;
    IntervalTreeNode* child = (toDelete->left) ? toDelete->left : toDelete->right;
    if (child) child->parent = p;
    if (!p) root = child;
    else {
        if (p->left == toDelete) p->left = child;
        else p->right = child;
    }
    if (p) updateMaxEndIterative(p);
    delete toDelete;
    return root;
}

void IntervalTreeNode::findOverlaps(IntervalTreeNode* root, Event* query, std::vector<Event*>& out) {
    if (!root) return;
    std::stack<IntervalTreeNode*> s;
    s.push(root);
    while (!s.empty()) {
        IntervalTreeNode* curr = s.top();
        s.pop();
        if (curr->event->startMinutes < query->endMinutes && query->startMinutes < curr->event->endMinutes) {
            out.push_back(curr->event);
        }
        if (curr->left && curr->left->maxEnd > query->startMinutes) s.push(curr->left);
        if (curr->right && curr->event->startMinutes < query->endMinutes) s.push(curr->right);
    }
}

void IntervalTreeNode::inorderCollect(IntervalTreeNode* root, std::vector<Event*>& out) {
    std::stack<IntervalTreeNode*> s;
    IntervalTreeNode* curr = root;
    while (curr || !s.empty()) {
        while (curr) { s.push(curr); curr = curr->left; }
        curr = s.top(); s.pop();
        out.push_back(curr->event);
        curr = curr->right;
    }
}