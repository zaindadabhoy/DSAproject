#include "IntervalTree.h"
#include <algorithm>
#include <stack>

IntervalTree::IntervalTree() : root(nullptr) {}

IntervalTree::~IntervalTree() { deleteTree(root); }
void IntervalTree::deleteTree(IntervalTreeNode *node)
{
    if (node)
    {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

void IntervalTree::updateMaxEnd(IntervalTreeNode *node)
{
    while (node != nullptr)
    {
        int currentMax = node->event->endMinutes;
        if (node->left)
            currentMax = std::max(currentMax, node->left->maxEnd);
        if (node->right)
            currentMax = std::max(currentMax, node->right->maxEnd);

        node->maxEnd = currentMax;
        node = node->parent;
    }
}

void IntervalTree::insert(Event *e)
{
    IntervalTreeNode *newNode = new IntervalTreeNode(e);
    if (root == nullptr)
    {
        root = newNode;
        return;
    }

    IntervalTreeNode *curr = root;
    IntervalTreeNode *prev = nullptr;

    while (curr != nullptr)
    {
        prev = curr;
        if (e->startMinutes < curr->event->startMinutes)
            curr = curr->left;
        else
            curr = curr->right;
    }

    newNode->parent = prev;
    if (e->startMinutes < prev->event->startMinutes)
        prev->left = newNode;
    else
        prev->right = newNode;

    updateMaxEnd(newNode);
}

void IntervalTree::remove(Event *e)
{
    IntervalTreeNode *curr = root, *target = nullptr;

    while (curr != nullptr)
    {
        if (curr->event == e)
        {
            target = curr;
            break;
        }
        if (e->startMinutes < curr->event->startMinutes)
            curr = curr->left;
        else
            curr = curr->right;
    }
    if (!target)
        return;

    IntervalTreeNode *toDelete = target;
    IntervalTreeNode *p = nullptr;

    if (target->left && target->right)
    {
        IntervalTreeNode *succ = target->right;
        while (succ->left)
            succ = succ->left;
        target->event = succ->event;
        toDelete = succ;
    }

    p = toDelete->parent;
    IntervalTreeNode *child = (toDelete->left) ? toDelete->left : toDelete->right;

    if (child)
        child->parent = p;

    if (!p)
    {
        root = child;
    }
    else
    {
        if (p->left == toDelete)
            p->left = child;
        else
            p->right = child;
    }

    if (p)
        updateMaxEnd(p);
    delete toDelete;
}

std::vector<Event *> IntervalTree::findOverlaps(Event *query)
{
    std::vector<Event *> out;
    if (!root)
        return out;

    std::stack<IntervalTreeNode *> s;
    s.push(root);

    while (!s.empty())
    {
        IntervalTreeNode *curr = s.top();
        s.pop();

        if (curr->event->startMinutes < query->endMinutes &&
            query->startMinutes < curr->event->endMinutes)
        {
            out.push_back(curr->event);
        }

        if (curr->left && curr->left->maxEnd > query->startMinutes)
        {
            s.push(curr->left);
        }

        if (curr->right && curr->event->startMinutes < query->endMinutes)
        {
            s.push(curr->right);
        }
    }
    return out;
}

std::vector<Event *> IntervalTree::getAllEventsSorted() const
{
    std::vector<Event *> out;
    std::stack<IntervalTreeNode *> s;
    IntervalTreeNode *curr = root;
    while (curr || !s.empty())
    {
        while (curr)
        {
            s.push(curr);
            curr = curr->left;
        }
        curr = s.top();
        s.pop();
        out.push_back(curr->event);
        curr = curr->right;
    }
    return out;
}