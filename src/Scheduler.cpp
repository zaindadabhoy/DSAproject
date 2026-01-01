#include "Scheduler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

Scheduler::Scheduler() {
    ResourceInfo r1; r1.resourceID = "R1"; r1.name = "Room 1"; r1.capacity = 200;
    ResourceInfo r2; r2.resourceID = "R2"; r2.name = "Room 2"; r2.capacity = 800;
    ResourceInfo r3; r3.resourceID = "R3"; r3.name = "Room 3"; r3.capacity = 1500;
    resources[r1.resourceID] = r1;
    resources[r2.resourceID] = r2;
    resources[r3.resourceID] = r3;
}

Scheduler::~Scheduler() { clearAllEvents(); }

int Scheduler::timeToMinutes(std::string t) const {
    if (t.length() < 5) return 0;
    int h = std::stoi(t.substr(0, 2));
    int m = std::stoi(t.substr(3, 2));
    return (h * 60) + m;
}

std::string Scheduler::minutesToTime(int m) const {
	if (m == 1440) return "24:00";
    int hours = (m / 60) % 24;
    int mins = m % 60;
    char buf[6];
    sprintf(buf, "%02d:%02d", hours, mins);
    return std::string(buf);
}

int Scheduler::findGap(ResourceInfo& room, std::string date, int duration, int preferredStart) const {
    for (int start = preferredStart; start <= (1440 - duration); start += 15) {
        Event tmp; tmp.date = date; tmp.startMinutes = start; tmp.endMinutes = start + duration;
        std::vector<Event*> overlaps = room.tree.findOverlaps(&tmp);
        bool conflict = false;
        for (auto ov : overlaps) if (ov->date == date) { conflict = true; break; }
        if (!conflict) return start;
    }
    for (int start = 0; start < preferredStart; start += 15) {
        Event tmp; tmp.date = date; tmp.startMinutes = start; tmp.endMinutes = start + duration;
        std::vector<Event*> overlaps = room.tree.findOverlaps(&tmp);
        bool conflict = false;
        for (auto ov : overlaps) if (ov->date == date) { conflict = true; break; }
        if (!conflict) return start;
    }
    return -1;
}

void Scheduler::scheduleEventUI() {
    printRooms();
    Event* e = new Event();
    std::cout << "Event ID: "; std::cin >> e->eventID; std::cin.ignore();
    std::cout << "Name: "; std::getline(std::cin, e->name);
    std::cout << "Date (YYYY-MM-DD): "; std::getline(std::cin, e->date);
    std::string sT, eT;
    std::cout << "Start (HH:MM): "; std::cin >> sT;
    std::cout << "End (HH:MM): "; std::cin >> eT;
    e->startMinutes = timeToMinutes(sT);
    e->endMinutes = timeToMinutes(eT);

    if (e->startMinutes >= e->endMinutes) {
        std::cout << "Error: Start time must be before end time. (Events cannot cross midnight).\n";
        delete e; return;
    }

    std::cout << "Priority (1-3): "; std::cin >> e->priority;
    std::cout << "Participants: "; std::cin >> e->maxParticipants;
    std::cout << "Room ID: "; std::cin >> e->resourceID;

    if (resources.find(e->resourceID) == resources.end()) {
        std::cout << "Room not found.\n";
        delete e; return;
    }
    ResourceInfo& room = resources[e->resourceID];

    std::vector<Event*> rawOverlaps = room.tree.findOverlaps(e);
    std::vector<Event*> sameDayConflicts;
    for (size_t i = 0; i < rawOverlaps.size(); i++) {
        if (rawOverlaps[i]->date == e->date) {
            sameDayConflicts.push_back(rawOverlaps[i]);
        }
    }

    if (sameDayConflicts.empty()) {
        room.tree.insert(e);
        ownedEvents.push_back(e);
        undoStack.push(e);
        std::cout << "Scheduled successfully.\n";
    } else {
        bool canPreemptAll = true;
        for (size_t i = 0; i < sameDayConflicts.size(); i++) {
            if (sameDayConflicts[i]->priority >= e->priority) {
                canPreemptAll = false;
                break;
            }
        }

        if (canPreemptAll) {
            std::cout << "Conflict detected! Your priority is higher. Preempting existing events...\n";
            std::priority_queue<Event*, std::vector<Event*>, ComparePriority> preemptionHeap;
            
            for (size_t i = 0; i < sameDayConflicts.size(); i++) {
                room.tree.remove(sameDayConflicts[i]);
                preemptionHeap.push(sameDayConflicts[i]);
            }

            room.tree.insert(e);
            ownedEvents.push_back(e);
            undoStack.push(e);

            while (!preemptionHeap.empty()) {
                Event* bumped = preemptionHeap.top();
                preemptionHeap.pop();
                int dur = bumped->endMinutes - bumped->startMinutes;
                int gap = findGap(room, bumped->date, dur, e->endMinutes);
                if (gap != -1) {
                    bumped->startMinutes = gap;
                    bumped->endMinutes = gap + dur;
                    room.tree.insert(bumped);
                    std::cout << ">> Event [" << bumped->eventID << "] bumped to " << minutesToTime(gap) << "\n";
                } else {
                    std::cout << ">> Event [" << bumped->eventID << "] could not be rescheduled today.\n";
                }
            }
        } else {
            std::cout << "Conflict with higher/equal priority event!\n";
            int dur = e->endMinutes - e->startMinutes;
            int alt = findGap(room, e->date, dur, e->endMinutes);
            if (alt != -1) {
                std::cout << "Suggested alternative: " << minutesToTime(alt) << ". Use? (y/n): ";
                char choice; std::cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    e->startMinutes = alt;
                    e->endMinutes = alt + dur;
                    room.tree.insert(e);
                    ownedEvents.push_back(e);
                    undoStack.push(e);
                    std::cout << "Scheduled at alternative time.\n";
                } else {
                    delete e;
                }
            } else {
                std::cout << "No slots left today.\n";
                delete e;
            }
        }
    }
}

void Scheduler::clearAllEvents() {
    std::unordered_map<std::string, ResourceInfo>::iterator it;
    for (it = resources.begin(); it != resources.end(); ++it) {
        it->second.tree.root = NULL;
    }
    for (size_t i = 0; i < ownedEvents.size(); ++i) delete ownedEvents[i];
    ownedEvents.clear();
    while(!undoStack.empty()) undoStack.pop();
    while(!redoStack.empty()) redoStack.pop();
}

std::vector<std::string> Scheduler::split(const std::string& s, char delim) const {
    std::vector<std::string> parts; std::string cur;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == delim) { parts.push_back(cur); cur = ""; }
        else cur += s[i];
    }
    parts.push_back(cur); return parts;
}

std::string Scheduler::joinQueue(std::queue<Participant> q) const {
    std::string res = "";
    bool f = true;
    while(!q.empty()) {
        if(!f) res += ",";
        res += q.front().name + ":" + q.front().pID;
        q.pop();
        f = false;
    }
    return res;
}

void Scheduler::loadQueue(std::queue<Participant>& q, std::string csv) {
    if (csv.empty()) return;
    std::stringstream ss(csv);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        size_t colonPos = pair.find(':');
        if (colonPos != std::string::npos) {
            Participant p;
            p.name = pair.substr(0, colonPos);
            p.pID = pair.substr(colonPos + 1);
            q.push(p);
        }
    }
}

bool Scheduler::saveToFile(const std::string& filename) const {
    std::ofstream out(filename.c_str()); if (!out) return false;
    out << "ROOMS\n";
    std::unordered_map<std::string, ResourceInfo>::const_iterator it;
    for (it = resources.begin(); it != resources.end(); ++it)
        out << it->second.resourceID << "|" << it->second.name << "|" << it->second.capacity << "\n";
    out << "EVENTS\n";
    for (it = resources.begin(); it != resources.end(); ++it) {
        std::vector<Event*> evs = it->second.tree.getAllEventsSorted();
        for (size_t i = 0; i < evs.size(); i++)
            out << evs[i]->eventID << "|" << evs[i]->name << "|" << evs[i]->date << "|" << evs[i]->resourceID << "|"
                << minutesToTime(evs[i]->startMinutes) << "|" << minutesToTime(evs[i]->endMinutes) << "|"
                << evs[i]->priority << "|" << evs[i]->maxParticipants << "|" 
                << joinQueue(evs[i]->participantQueue) << "|" << joinQueue(evs[i]->waitlistQueue) << "\n";
    }
    return true;
}

bool Scheduler::loadFromFile(const std::string& filename) {
    std::ifstream in(filename.c_str()); if (!in) return false;
    clearAllEvents(); std::string line; std::getline(in, line);
    while(std::getline(in, line) && line != "EVENTS") {
        std::vector<std::string> p = split(line, '|'); if (p.size() < 3) continue;
        ResourceInfo r; r.resourceID = p[0]; r.name = p[1]; r.capacity = std::atoi(p[2].c_str());
        resources[r.resourceID] = r;
    }
    while(std::getline(in, line)) {
        std::vector<std::string> p = split(line, '|'); if (p.size() < 8) continue;
        Event* e = new Event();
        e->eventID = p[0]; e->name = p[1]; e->date = p[2]; e->resourceID = p[3];
        e->startMinutes = timeToMinutes(p[4]); e->endMinutes = timeToMinutes(p[5]);
        e->priority = std::atoi(p[6].c_str()); e->maxParticipants = std::atoi(p[7].c_str());
        if (p.size() >= 9) loadQueue(e->participantQueue, p[8]);
        if (p.size() >= 10) loadQueue(e->waitlistQueue, p[9]);
        ownedEvents.push_back(e);
        if (resources.count(e->resourceID)) resources[e->resourceID].tree.insert(e);
    }
    return true;
}

void Scheduler::printSchedule() const {
    std::unordered_map<std::string, ResourceInfo>::const_iterator it;
    for (it = resources.begin(); it != resources.end(); ++it) {
        std::cout << "\n[" << it->second.resourceID << " " << it->second.name << "]\n";
        std::vector<Event*> evs = it->second.tree.getAllEventsSorted();
        for (size_t i = 0; i < evs.size(); i++) {
            std::cout << evs[i]->eventID << " | " << evs[i]->date << " | " << minutesToTime(evs[i]->startMinutes) 
                      << "-" << minutesToTime(evs[i]->endMinutes) << " | P:" << evs[i]->priority << "\n";
        }
    }
}

void Scheduler::printRooms() const {
    std::unordered_map<std::string, ResourceInfo>::const_iterator it;
    for (it = resources.begin(); it != resources.end(); ++it)
        std::cout << it->second.resourceID << " | cap:" << it->second.capacity << "\n";
}

Event* Scheduler::findEventByID(std::string id) {
    for (size_t i = 0; i < ownedEvents.size(); i++) {
        if (ownedEvents[i]->eventID == id) return ownedEvents[i];
    }
    return NULL;
}

void Scheduler::deleteEventUI() {
    std::string id;
    std::cout << "Enter Event ID to delete: ";
    std::cin >> id;

    bool found = false;
    for (size_t i = 0; i < ownedEvents.size(); i++) {
        if (ownedEvents[i]->eventID == id) {
            Event* e = ownedEvents[i];
            if (resources.count(e->resourceID)) {
                resources[e->resourceID].tree.remove(e);
            }
            ownedEvents.erase(ownedEvents.begin() + i);
            delete e;
            found = true;
            std::cout << "Event deleted successfully.\n";
            break;
        }
    }
    if (!found) std::cout << "Event ID not found.\n";
}

void Scheduler::registerParticipantUI() {
    std::string eventId, userName, userId;
    std::cout << "Enter Event ID: ";
    std::cin >> eventId;
    std::cout << "Enter Name: ";
    std::cin.ignore();
    std::getline(std::cin, userName);
    std::cout << "Enter Unique ID (e.g., Email/Seat#): ";
    std::getline(std::cin, userId);

    Event* target = findEventByID(eventId);
    if (!target) return;

    Participant p;
    p.name = userName;
    p.pID = userId;

    if (target->participantQueue.size() < (size_t)target->maxParticipants) {
        target->participantQueue.push(p);
        std::cout << "Registered successfully.\n";
    } else {
        target->waitlistQueue.push(p);
        std::cout << "Event full. Added to waitlist.\n";
    }
}

void Scheduler::withdrawParticipantUI() {
    std::string eventId, userId;
    std::cout << "Enter Event ID: ";
    std::cin >> eventId;
    std::cout << "Enter your Unique ID to verify identity: ";
    std::cin >> userId;

    Event* target = findEventByID(eventId);
    if (!target) return;

    std::queue<Participant> tempQ;
    bool found = false;

    while (!target->participantQueue.empty()) {
        Participant current = target->participantQueue.front();
        target->participantQueue.pop();

        if (current.pID == userId && !found) {
            found = true;
            std::cout << "Withdrew " << current.name << " (ID: " << userId << ")\n";
        } else {
            tempQ.push(current);
        }
    }
    target->participantQueue = tempQ;

    if (!found) {
        std::queue<Participant> tempW;
        while (!target->waitlistQueue.empty()) {
            Participant current = target->waitlistQueue.front();
            target->waitlistQueue.pop();

            if (current.pID == userId && !found) {
                found = true;
                std::cout << "Removed from waitlist.\n";
            } else {
                tempW.push(current);
            }
        }
        target->waitlistQueue = tempW;
    } else if (!target->waitlistQueue.empty()) {
        target->participantQueue.push(target->waitlistQueue.front());
        std::cout << target->waitlistQueue.front().name << " moved from waitlist to registered.\n";
        target->waitlistQueue.pop();
    }

    if (!found) std::cout << "Error: ID not found in this event.\n";
}

void Scheduler::undo() {
    if (undoStack.empty()) {
        std::cout << "Nothing to undo.\n";
        return;
    }
    Event* e = undoStack.top();
    undoStack.pop();
    redoStack.push(e);

    if (resources.count(e->resourceID)) {
        resources[e->resourceID].tree.remove(e);
    }
    std::cout << "Undo successful: Removed " << e->name << ".\n";
}

void Scheduler::redo() {
    if (redoStack.empty()) {
        std::cout << "Nothing to redo.\n";
        return;
    }
    Event* e = redoStack.top();
    redoStack.pop();
    undoStack.push(e);

    if (resources.count(e->resourceID)) {
        resources[e->resourceID].tree.insert(e);
    }
    std::cout << "Redo successful: Restored " << e->name << ".\n";
}

void Scheduler::addRoomUI() {
    std::string id, name;
    int cap;

    std::cout << "\n--- ADD NEW ROOM ---\n";
    std::cout << "Enter Room ID (e.g., R4): ";
    std::cin >> id;
    
    if (resources.find(id) != resources.end()) {
        std::cout << "Error: Room ID " << id << " already exists.\n";
        return;
    }

    std::cout << "Enter Room Name: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cout << "Enter Capacity: ";
    std::cin >> cap;

    if (cap <= 0) {
        std::cout << "Error: Capacity must be a positive number.\n";
        return;
    }

    ResourceInfo newRoom;
    newRoom.resourceID = id;
    newRoom.name = name;
    newRoom.capacity = cap;

    resources[id] = newRoom;
    std::cout << "Room " << id << " added successfully.\n";
}

void Scheduler::deleteRoomUI() {
    std::string id;
    std::cout << "Enter Room ID to delete: ";
    std::cin >> id;

    if (resources.find(id) == resources.end()) {
        std::cout << "Error: Room ID " << id << " not found.\n";
        return;
    }

    bool hasEvents = false;
    for (size_t i = 0; i < ownedEvents.size(); i++) {
        if (ownedEvents[i]->resourceID == id) {
            hasEvents = true;
            break;
        }
    }

    if (hasEvents) {
        std::cout << "Error: Cannot delete room. There are events scheduled in this room.\n";
        std::cout << "Please delete or reschedule those events first.\n";
        return;
    }

    resources.erase(id);
    std::cout << "Room " << id << " deleted successfully.\n";
}

#include <set>

void Scheduler::printUtilizationReportUI() const {
    std::string targetDate;
    std::cout << "Enter Date (YYYY-MM-DD) or type 'all': ";
    std::cin >> targetDate;

    std::cout << "\nUTILIZATION REPORT (" << targetDate << ")\n";
    std::cout << "===============================================================\n";
    std::cout << "Room ID    | Room Name       | Mins Used | Dates | Utilization%\n";
    std::cout << "-----------|-----------------|-----------|-------|-------------\n";

    for (auto const& pair : resources) {
        const ResourceInfo& room = pair.second;
        std::vector<Event*> events = room.tree.getAllEventsSorted();
        
        int totalMins = 0;
        std::set<std::string> distinctDates;

        for (size_t i = 0; i < events.size(); i++) {
            if (targetDate == "all" || events[i]->date == targetDate) {
                totalMins += (events[i]->endMinutes - events[i]->startMinutes);
                distinctDates.insert(events[i]->date);
            }
        }

        int datesCount = distinctDates.size();
        double percentage = 0.0;
        
        if (datesCount > 0) {
            percentage = (totalMins / (static_cast<double>(datesCount) * 1440.0)) * 100.0;
        }

        std::cout << room.resourceID;
        for(int s=0; s < (11 - (int)room.resourceID.length()); s++) std::cout << " ";
        std::cout << "| ";

        std::cout << room.name;
        for(int s=0; s < (16 - (int)room.name.length()); s++) std::cout << " ";
        std::cout << "| ";

        std::string mStr = std::to_string(totalMins);
        std::cout << mStr;
        for(int s=0; s < (10 - (int)mStr.length()); s++) std::cout << " ";
        std::cout << "| ";

        std::string dStr = std::to_string(datesCount);
        std::cout << dStr;
        for(int s=0; s < (6 - (int)dStr.length()); s++) std::cout << " ";
        std::cout << "| ";

        std::cout << (int)percentage << "%\n";
    }
    std::cout << "===============================================================\n";
}