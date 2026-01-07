#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "Event.h"
#include <vector>
#include <algorithm>

class EventMaxHeap
{
private:
    std::vector<Event *> heap;

    void heapifyUp(int index)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;
            if (heap[index]->priority > heap[parent]->priority)
            {
                std::swap(heap[index], heap[parent]);
                index = parent;
            }
            else
                break;
        }
    }

    void heapifyDown(int index)
    {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < (int)heap.size() && heap[left]->priority > heap[largest]->priority)
            largest = left;

        if (right < (int)heap.size() && heap[right]->priority > heap[largest]->priority)
        {
            largest = right;
        }

        if (largest != index)
        {
            std::swap(heap[index], heap[largest]);
            heapifyDown(largest);
        }
    }

public:
    void push(Event *e)
    {
        heap.push_back(e);
        heapifyUp((int)heap.size() - 1);
    }

    Event *top() { return heap.empty() ? NULL : heap[0]; }

    void pop()
    {
        if (heap.empty())
            return;
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
    }

    bool empty() { return heap.empty(); }
};
#endif