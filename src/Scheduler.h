#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <unordered_map>
#include <stack>
#include <string>
#include <vector>
#include <queue>
#include "IntervalTree.h"
#include "Event.h"

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
    std::stack<Event*> undoStack;
    std::stack<Event*> redoStack;

    int findGap(ResourceInfo& room, std::string date, int duration, int preferredStart = 0) const;
    void clearAllEvents();
    std::vector<std::string> split(const std::string& s, char delim) const;
    std::string joinQueue(std::queue<Participant> q) const;
    void loadQueue(std::queue<Participant>& q, std::string csv);
    int timeToMinutes(std::string t) const;
    std::string minutesToTime(int m) const;
    Event* findEventByID(std::string id);

public:
    Scheduler();
    ~Scheduler();
    void addRoomUI();
    void deleteRoomUI();
    void printRooms() const;
    void scheduleEventUI();
    void deleteEventUI();
    void printSchedule() const;
    void undo();
    void redo();
    void registerParticipantUI();
    void withdrawParticipantUI();
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    void printUtilizationReportUI() const;
};
#endif