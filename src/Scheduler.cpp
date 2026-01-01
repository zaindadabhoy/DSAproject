#include "Scheduler.h"
#include <iostream>
#include <algorithm>
#include <queue>

Scheduler::Scheduler() {
    ResourceInfo r1; r1.resourceID = "R1"; r1.name = "Room 1"; r1.capacity = 200;
    resources[r1.resourceID] = r1;
}

Scheduler::~Scheduler() { for (auto e : ownedEvents) delete e; }

int Scheduler::timeToMinutes(std::string t) const {
    if (t.length() < 5) return 0;
    return std::stoi(t.substr(0, 2)) * 60 + std::stoi(t.substr(3, 2));
}

std::string Scheduler::minutesToTime(int m) const {
    char buf[6];
    sprintf(buf, "%02d:%02d", (m / 60) % 24, m % 60);
    return std::string(buf);
}

int Scheduler::findGap(ResourceInfo& room, std::string date, int duration, int pref) const {
    for (int s = pref; s <= (1440 - duration); s += 15) {
        Event tmp; tmp.date = date; tmp.startMinutes = s; tmp.endMinutes = s + duration;
        auto ov = room.tree.findOverlaps(&tmp);
        bool conflict = false;
        for (auto o : ov) if (o->date == date) { conflict = true; break; }
        if (!conflict) return s;
    }
    return -1;
}

void Scheduler::scheduleEventUI() {
    Event* e = new Event();
    std::cout << "ID: "; std::cin >> e->eventID;
    std::cout << "Date: "; std::cin >> e->date;
    std::string sT, eT; std::cin >> sT >> eT;
    e->startMinutes = timeToMinutes(sT); e->endMinutes = timeToMinutes(eT);
    std::cout << "Priority (1-3): "; std::cin >> e->priority;
    std::cout << "Room: "; std::cin >> e->resourceID;

    ResourceInfo& room = resources[e->resourceID];
    auto overlaps = room.tree.findOverlaps(e);
    std::vector<Event*> sameDay;
    for (auto o : overlaps) if (o->date == e->date) sameDay.push_back(o);

    if (sameDay.empty()) {
        room.tree.insert(e); ownedEvents.push_back(e);
        std::cout << "Scheduled.\n";
    } else {
        bool higher = true;
        for (auto o : sameDay) if (o->priority >= e->priority) higher = false;
        if (higher) {
            for (auto o : sameDay) room.tree.remove(o);
            room.tree.insert(e); ownedEvents.push_back(e);
            std::cout << "Preempted others.\n";
        } else {
            std::cout << "Conflict! Try another time.\n";
            delete e;
        }
    }
}