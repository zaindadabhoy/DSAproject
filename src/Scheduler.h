#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <unordered_map>
#include <stack>
#include <string>
#include <vector>
#include "IntervalTree.h"

struct ComparePriority {
    bool operator()(Event* a, Event* b) { return a->priority < b->priority; }
};

class Scheduler {
private:
    struct ResourceInfo {
        std::string resourceID;
        std::string name;
        int capacity;
        IntervalTree tree;
    };
    std::unordered_map<std::string, ResourceInfo> resources;
    std::vector<Event*> ownedEvents;
    std::stack<Event*> undoStack, redoStack;

    int timeToMinutes(std::string t) const;
    std::string minutesToTime(int m) const;
    int findGap(ResourceInfo& room, std::string date, int duration, int preferredStart = 0) const;

public:
    Scheduler();
    ~Scheduler();
    void scheduleEventUI();
    void printSchedule() const;
    void printRooms() const;
};
#endif