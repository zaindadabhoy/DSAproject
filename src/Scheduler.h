#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <unordered_map>
#include <stack>
#include <string>
#include <vector>
#include <queue>
#include "IntervalTree.h"
#include "Event.h"

enum ActionType
{
    ACTION_ADD,
    ACTION_DELETE
};

struct ActionRecord
{
    ActionType type;
    Event *event;
};

class Scheduler
{
private:
    struct ResourceInfo
    {
        std::string resourceID;
        std::string name;
        int capacity;
        IntervalTree tree;
    };
    int nextEventID;
    int nextParticipantID;

    std::unordered_map<std::string, ResourceInfo> resources;
    std::vector<Event *> ownedEvents;

    std::stack<ActionRecord> undoStack;
    std::stack<ActionRecord> redoStack;

    int findGap(ResourceInfo &room, std::string date, int duration, int preferredStart = 0) const;
    void clearAllEvents();
    std::vector<std::string> split(const std::string &s, char delim) const;
    std::string joinQueue(std::queue<Participant> q) const;
    void loadQueue(std::queue<Participant> &q, std::string csv);
    int timeToMinutes(std::string t) const;
    std::string minutesToTime(int m) const;
    Event *findEventByID(std::string id);
    void syncCounters();

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
    bool saveToFile(const std::string &filename) const;
    bool loadFromFile(const std::string &filename);
    void printUtilizationReportUI() const;
};
#endif