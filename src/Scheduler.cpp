#include "Scheduler.h"
#include "PriorityQueue.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

Scheduler::Scheduler()
{
    ResourceInfo r1;
    r1.resourceID = "R1";
    r1.name = "Room 1";
    r1.capacity = 200;
    ResourceInfo r2;
    r2.resourceID = "R2";
    r2.name = "Room 2";
    r2.capacity = 800;
    ResourceInfo r3;
    r3.resourceID = "R3";
    r3.name = "Room 3";
    r3.capacity = 1500;
    resources[r1.resourceID] = r1;
    resources[r2.resourceID] = r2;
    resources[r3.resourceID] = r3;

    nextEventID = 1000;
    nextParticipantID = 5000;
}

Scheduler::~Scheduler() { clearAllEvents(); }

int Scheduler::timeToMinutes(std::string t) const
{
    if (t.length() < 5)
        return 0;
    int h = std::stoi(t.substr(0, 2));
    int m = std::stoi(t.substr(3, 2));
    return (h * 60) + m;
}

std::string Scheduler::minutesToTime(int m) const
{
    if (m >= 1440)
        return "24:00";
    int hours = (m / 60) % 24;
    int mins = m % 60;
    std::string res = "";
    if (hours < 10)
        res += "0";
    res += std::to_string(hours) + ":";
    if (mins < 10)
        res += "0";
    res += std::to_string(mins);
    return res;
}

int Scheduler::findGap(ResourceInfo &room, std::string date, int duration, int preferredStart) const
{
    for (int start = preferredStart; start <= (1440 - duration); start += 15)
    {
        Event tmp;
        tmp.date = date;
        tmp.startMinutes = start;
        tmp.endMinutes = start + duration;
        std::vector<Event *> overlaps = room.tree.findOverlaps(&tmp);
        bool conflict = false;
        for (auto ov : overlaps)
            if (ov->date == date)
            {
                conflict = true;
                break;
            }
        if (!conflict)
            return start;
    }
    for (int start = 0; start < preferredStart; start += 15)
    {
        Event tmp;
        tmp.date = date;
        tmp.startMinutes = start;
        tmp.endMinutes = start + duration;
        std::vector<Event *> overlaps = room.tree.findOverlaps(&tmp);
        bool conflict = false;
        for (auto ov : overlaps)
            if (ov->date == date)
            {
                conflict = true;
                break;
            }
        if (!conflict)
            return start;
    }
    return -1;
}

void Scheduler::scheduleEventUI()
{
    printRooms();

    std::string newID = "EVT-" + std::to_string(nextEventID);
    nextEventID++;

    Event *e = new Event();
    e->eventID = newID;

    std::cin.ignore();
    std::cout << "Name: ";
    std::getline(std::cin, e->name);
    std::cout << "Date (YYYY-MM-DD): ";
    std::getline(std::cin, e->date);
    std::string sT, eT;
    std::cout << "Start (HH:MM): ";
    std::cin >> sT;
    std::cout << "End (HH:MM): ";
    std::cin >> eT;
    e->startMinutes = timeToMinutes(sT);
    e->endMinutes = timeToMinutes(eT);

    if (e->startMinutes >= e->endMinutes)
    {
        std::cout << "Error: Invalid time range.\n";
        delete e;
        return;
    }

    std::cout << "Priority (1-3): ";
    std::cin >> e->priority;
    std::cout << "Capacity: ";
    std::cin >> e->maxParticipants;
    std::cout << "Room ID: ";
    std::cin >> e->resourceID;

    if (resources.find(e->resourceID) == resources.end())
    {
        std::cout << "Error: Room ID '" << e->resourceID << "' does not exist.\n";
        delete e;
        return;
    }

    ResourceInfo &room = resources[e->resourceID];
    std::vector<Event *> rawOverlaps = room.tree.findOverlaps(e);
    std::vector<Event *> sameDayConflicts;
    for (size_t i = 0; i < rawOverlaps.size(); i++)
    {
        if (rawOverlaps[i]->date == e->date)
            sameDayConflicts.push_back(rawOverlaps[i]);
    }

    if (sameDayConflicts.empty())
    {
        room.tree.insert(e);
        ownedEvents.push_back(e);
        undoStack.push({ACTION_ADD, e});
        std::cout << "Event scheduled successfully.\n";
    }
    else
    {
        bool canPreemptAll = true;
        for (size_t i = 0; i < sameDayConflicts.size(); i++)
        {
            if (sameDayConflicts[i]->priority >= e->priority)
            {
                canPreemptAll = false;
                break;
            }
        }

        if (canPreemptAll)
        {
            std::cout << "Higher priority: Bumping existing events...\n";
            for (size_t i = 0; i < sameDayConflicts.size(); i++)
                room.tree.remove(sameDayConflicts[i]);

            room.tree.insert(e);
            ownedEvents.push_back(e);
            undoStack.push({ACTION_ADD, e});

            EventMaxHeap bumpHeap;
            for (size_t i = 0; i < sameDayConflicts.size(); i++)
                bumpHeap.push(sameDayConflicts[i]);

            while (!bumpHeap.empty())
            {
                Event *bumped = bumpHeap.top();
                bumpHeap.pop();
                int dur = bumped->endMinutes - bumped->startMinutes;
                int gap = findGap(room, bumped->date, dur, e->endMinutes);
                if (gap != -1)
                {
                    bumped->startMinutes = gap;
                    bumped->endMinutes = gap + dur;
                    room.tree.insert(bumped);
                    std::cout << ">> " << bumped->eventID << " moved to " << minutesToTime(gap) << "\n";
                }
                else
                {
                    std::cout << ">> " << bumped->eventID << " could not be rescheduled (no gaps).\n";
                }
            }
        }
        else
        {
            std::cout << "Conflict with higher priority event. Seeking alternative...\n";
            int dur = e->endMinutes - e->startMinutes;
            int alt = findGap(room, e->date, dur, e->endMinutes);
            if (alt != -1)
            {
                std::cout << "Suggested alternative: " << minutesToTime(alt) << ". Accept? (y/n): ";
                char c;
                std::cin >> c;
                if (c == 'y' || c == 'Y')
                {
                    e->startMinutes = alt;
                    e->endMinutes = alt + dur;
                    room.tree.insert(e);
                    ownedEvents.push_back(e);
                    undoStack.push({ACTION_ADD, e});
                }
                else
                    delete e;
            }
            else
            {
                std::cout << "No slots available for this date.\n";
                delete e;
            }
        }
    }
}

void Scheduler::deleteEventUI()
{
    std::string id;
    std::cout << "Enter Event ID to delete: ";
    std::cin >> id;
    Event *e = findEventByID(id);
    if (!e)
    {
        std::cout << "Event ID not found.\n";
        return;
    }

    if (resources.count(e->resourceID))
        resources[e->resourceID].tree.remove(e);
    undoStack.push({ACTION_DELETE, e});
    while (!redoStack.empty())
        redoStack.pop();
    std::cout << "Event deleted successfully.\n";
}

void Scheduler::registerParticipantUI()
{
    std::string eid, name;
    std::cout << "Enter Event ID to register for: ";
    std::cin >> eid;

    Event *t = findEventByID(eid);
    if (t == NULL)
    {
        std::cout << "Error: Event '" << eid << "' does not exist.\n";
        return;
    }

    std::cout << "Participant Name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    std::string newPID = "P-" + std::to_string(nextParticipantID);
    nextParticipantID++;

    Participant p;
    p.name = name;
    p.pID = newPID;

    if (t->participantQueue.size() < (size_t)t->maxParticipants)
    {
        t->participantQueue.push(p);
        std::cout << "Registered! Your System ID is: " << p.pID << "\n";
    }
    else
    {
        t->waitlistQueue.push(p);
        std::cout << "Waitlisted. Your System ID is: " << p.pID << "\n";
    }
}

void Scheduler::withdrawParticipantUI()
{
    std::string eid, uid;
    std::cout << "Enter Event ID: ";
    std::cin >> eid;

    Event *t = findEventByID(eid);
    if (t == NULL)
    {
        std::cout << "Error: Event ID '" << eid << "' not found.\n";
        return;
    }

    std::cout << "Enter Participant ID to withdraw: ";
    std::cin >> uid;

    std::queue<Participant> temp;
    bool found = false;

    while (!t->participantQueue.empty())
    {
        Participant p = t->participantQueue.front();
        t->participantQueue.pop();
        if (p.pID == uid && !found)
            found = true;
        else
            temp.push(p);
    }
    t->participantQueue = temp;

    if (!found)
    {
        std::queue<Participant> wTemp;
        while (!t->waitlistQueue.empty())
        {
            Participant p = t->waitlistQueue.front();
            t->waitlistQueue.pop();
            if (p.pID == uid && !found)
                found = true;
            else
                wTemp.push(p);
        }
        t->waitlistQueue = wTemp;
    }
    else
    {
        if (!t->waitlistQueue.empty())
        {
            t->participantQueue.push(t->waitlistQueue.front());
            t->waitlistQueue.pop();
        }
    }

    if (found)
    {
        std::cout << "ID " << uid << " has been successfully withdrawn.\n";
    }
    else
    {
        std::cout << "Error: Participant ID '" << uid << "' does not exist for this event.\n";
    }
}

void Scheduler::addRoomUI()
{
    std::string id, name;
    int cap;
    std::cout << "Room ID: ";
    std::cin >> id;
    if (resources.count(id))
    {
        std::cout << "Exists.\n";
        return;
    }
    std::cout << "Name: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cout << "Capacity: ";
    std::cin >> cap;
    resources[id] = {id, name, cap, IntervalTree()};
}

void Scheduler::deleteRoomUI()
{
    std::string id;
    std::cout << "Room ID: ";
    std::cin >> id;
    if (!resources.count(id))
        return;
    for (auto e : ownedEvents)
        if (e->resourceID == id)
        {
            std::cout << "Has events.\n";
            return;
        }
    resources.erase(id);
}

void Scheduler::printUtilizationReportUI() const
{
    std::string targetDate;
    std::cout << "Enter Date (YYYY-MM-DD) or 'all': ";
    std::cin >> targetDate;

    std::cout << "\n============================================================\n";
    std::cout << "                ROOM UTILIZATION REPORT (" << targetDate << ")\n";
    std::cout << "============================================================\n";
    std::cout << std::left << std::setw(10) << "Room ID"
              << "| " << std::setw(12) << "Time Util"
              << "| " << std::setw(12) << "Avg People"
              << "| " << std::setw(10) << "Cap Util" << "\n";
    std::cout << "----------|-------------|-------------|-------------\n";

    double totalCampusMins = 0;
    int totalCampusEvents = 0;

    for (std::unordered_map<std::string, ResourceInfo>::const_iterator it = resources.begin(); it != resources.end(); ++it)
    {
        const ResourceInfo &room = it->second;

        double roomBusyMins = 0;
        double totalAttendees = 0;
        int eventCount = 0;

        std::vector<std::string> datesSeen;

        std::vector<Event *> evs = room.tree.getAllEventsSorted();
        for (size_t i = 0; i < evs.size(); ++i)
        {
            Event *e = evs[i];

            if (targetDate == "all" || e->date == targetDate)
            {
                roomBusyMins += (e->endMinutes - e->startMinutes);
                totalAttendees += (double)e->participantQueue.size();
                eventCount++;

                bool alreadySeen = false;
                for (size_t j = 0; j < datesSeen.size(); ++j)
                {
                    if (datesSeen[j] == e->date)
                    {
                        alreadySeen = true;
                        break;
                    }
                }
                if (!alreadySeen)
                    datesSeen.push_back(e->date);
            }
        }

        double timeUtil = 0.0, capUtil = 0.0, avgOcc = 0.0;

        double availableMins = 1440.0;
        if (targetDate == "all")
        {
            availableMins = (datesSeen.size() > 0) ? (datesSeen.size() * 1440.0) : 1440.0;
        }

        if (eventCount > 0)
        {
            timeUtil = (roomBusyMins / availableMins) * 100.0;
            avgOcc = totalAttendees / (double)eventCount;
            capUtil = (avgOcc / (double)room.capacity) * 100.0;

            if (timeUtil > 100.0)
                timeUtil = 100.0;
        }

        std::cout << std::left << std::setw(10) << room.resourceID << "| "
                  << std::fixed << std::setprecision(1) << std::setw(10) << timeUtil << "% | "
                  << std::setw(11) << avgOcc << " | "
                  << std::setw(9) << capUtil << "%\n";

        totalCampusMins += roomBusyMins;
        totalCampusEvents += eventCount;
    }
    std::cout << "============================================================\n";
    std::cout << " Total Events Managed: " << totalCampusEvents << "\n";
    std::cout << " Total Active Minutes: " << totalCampusMins << "\n";
    std::cout << "============================================================\n";
}

void Scheduler::undo()
{
    if (undoStack.empty())
    {
        std::cout << "Nothing to undo.\n";
        return;
    }
    ActionRecord record = undoStack.top();
    undoStack.pop();
    redoStack.push(record);

    if (record.type == ACTION_ADD)
    {
        if (resources.count(record.event->resourceID))
            resources[record.event->resourceID].tree.remove(record.event);
        std::cout << "Undo: Removed " << record.event->name << ".\n";
    }
    else
    {
        if (resources.count(record.event->resourceID))
            resources[record.event->resourceID].tree.insert(record.event);
        std::cout << "Undo: Restored " << record.event->name << ".\n";
    }
}

void Scheduler::redo()
{
    if (redoStack.empty())
    {
        std::cout << "Nothing to redo.\n";
        return;
    }
    ActionRecord record = redoStack.top();
    redoStack.pop();
    undoStack.push(record);

    if (record.type == ACTION_ADD)
    {
        if (resources.count(record.event->resourceID))
            resources[record.event->resourceID].tree.insert(record.event);
        std::cout << "Redo: Restored " << record.event->name << ".\n";
    }
    else
    {
        if (resources.count(record.event->resourceID))
            resources[record.event->resourceID].tree.remove(record.event);
        std::cout << "Redo: Deleted " << record.event->name << " again.\n";
    }
}

std::vector<std::string> Scheduler::split(const std::string &s, char delim) const
{
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == delim)
        {
            parts.push_back(cur);
            cur = "";
        }
        else
        {
            cur += s[i];
        }
    }
    parts.push_back(cur);
    return parts;
}

std::string Scheduler::joinQueue(std::queue<Participant> q) const
{
    std::string res = "";
    bool f = true;
    while (!q.empty())
    {
        if (!f)
            res += ",";
        res += q.front().name + ":" + q.front().pID;
        q.pop();
        f = false;
    }
    return res;
}

void Scheduler::loadQueue(std::queue<Participant> &q, std::string csv)
{
    if (csv.empty())
        return;
    std::stringstream ss(csv);
    std::string pair;
    while (std::getline(ss, pair, ','))
    {
        size_t colonPos = pair.find(':');
        if (colonPos != std::string::npos)
        {
            Participant p;
            p.name = pair.substr(0, colonPos);
            p.pID = pair.substr(colonPos + 1);
            q.push(p);
        }
    }
}

bool Scheduler::saveToFile(const std::string &filename) const
{
    std::ofstream out(filename.c_str());
    if (!out)
        return false;

    out << "ROOMS_COUNT|" << resources.size() << "\n";
    for (std::unordered_map<std::string, ResourceInfo>::const_iterator it = resources.begin(); it != resources.end(); ++it)
    {
        out << it->second.resourceID << "|" << it->second.name << "|" << it->second.capacity << "\n";
    }

    out << "EVENTS_COUNT|" << ownedEvents.size() << "\n";
    for (size_t i = 0; i < ownedEvents.size(); ++i)
    {
        Event *e = ownedEvents[i];
        out << e->eventID << "|" << e->name << "|" << e->date << "|" << e->resourceID << "|"
            << minutesToTime(e->startMinutes) << "|" << minutesToTime(e->endMinutes) << "|"
            << e->priority << "|" << e->maxParticipants << "\n";

        out << "PARTICIPANTS|";
        std::queue<Participant> tempP = e->participantQueue;
        while (!tempP.empty())
        {
            out << tempP.front().pID << ":" << tempP.front().name;
            tempP.pop();
            if (!tempP.empty())
                out << ",";
        }
        out << "\n";

        out << "WAITLIST|";
        std::queue<Participant> tempW = e->waitlistQueue;
        while (!tempW.empty())
        {
            out << tempW.front().pID << ":" << tempW.front().name;
            tempW.pop();
            if (!tempW.empty())
                out << ",";
        }
        out << "\n";
    }
    return true;
}

bool Scheduler::loadFromFile(const std::string &filename)
{
    std::ifstream in(filename.c_str());
    if (!in)
        return false;

    clearAllEvents();
    std::string line;

    if (!std::getline(in, line))
        return false;
    std::vector<std::string> header = split(line, '|');
    if (header.size() < 2)
        return false;

    int roomCount = std::stoi(header[1]);
    for (int i = 0; i < roomCount; ++i)
    {
        if (!std::getline(in, line))
            break;
        std::vector<std::string> p = split(line, '|');
        if (p.size() >= 3)
        {
            resources[p[0]] = {p[0], p[1], std::stoi(p[2]), IntervalTree()};
        }
    }

    if (!std::getline(in, line))
        return true;
    std::vector<std::string> eHeader = split(line, '|');
    if (eHeader.size() < 2)
        return true;

    int eventCount = std::stoi(eHeader[1]);
    int maxE = 1000, maxP = 5000;

    for (int i = 0; i < eventCount; ++i)
    {
        if (!std::getline(in, line))
            break;
        std::vector<std::string> p = split(line, '|');
        if (p.size() < 8)
            continue;

        Event *e = new Event();
        e->eventID = p[0];
        e->name = p[1];
        e->date = p[2];
        e->resourceID = p[3];
        e->startMinutes = timeToMinutes(p[4]);
        e->endMinutes = timeToMinutes(p[5]);
        e->priority = std::stoi(p[6]);
        e->maxParticipants = std::stoi(p[7]);

        if (e->eventID.length() > 4)
        {
            int eNum = std::stoi(e->eventID.substr(4));
            if (eNum >= maxE)
                maxE = eNum + 1;
        }

        if (std::getline(in, line))
        {
            std::vector<std::string> parts = split(line, '|');
            if (parts.size() >= 2 && !parts[1].empty())
            {
                std::vector<std::string> pList = split(parts[1], ',');
                for (size_t j = 0; j < pList.size(); ++j)
                {
                    std::vector<std::string> pair = split(pList[j], ':');
                    if (pair.size() >= 2)
                    {
                        e->participantQueue.push({pair[1], pair[0]});
                        int pNum = std::stoi(pair[0].substr(2));
                        if (pNum >= maxP)
                            maxP = pNum + 1;
                    }
                }
            }
        }

        if (std::getline(in, line))
        {
            std::vector<std::string> parts = split(line, '|');
            if (parts.size() >= 2 && !parts[1].empty())
            {
                std::vector<std::string> wList = split(parts[1], ',');
                for (size_t j = 0; j < wList.size(); ++j)
                {
                    std::vector<std::string> pair = split(wList[j], ':');
                    if (pair.size() >= 2)
                    {
                        e->waitlistQueue.push({pair[1], pair[0]});
                        int pNum = std::stoi(pair[0].substr(2));
                        if (pNum >= maxP)
                            maxP = pNum + 1;
                    }
                }
            }
        }

        ownedEvents.push_back(e);
        if (resources.count(e->resourceID))
        {
            resources[e->resourceID].tree.insert(e);
        }
    }

    nextEventID = maxE;
    nextParticipantID = maxP;
    return true;
}

void Scheduler::printSchedule() const
{
    for (std::unordered_map<std::string, ResourceInfo>::const_iterator it = resources.begin(); it != resources.end(); ++it)
    {
        std::cout << "\n[" << it->second.resourceID << " " << it->second.name << "]\n";
        std::vector<Event *> evs = it->second.tree.getAllEventsSorted();
        for (size_t i = 0; i < evs.size(); ++i)
        {
            Event *e = evs[i];
            std::cout << e->eventID << " | " << e->date << " | "
                      << minutesToTime(e->startMinutes) << "-" << minutesToTime(e->endMinutes)
                      << " | P:" << e->priority << "\n";
        }
    }
}

void Scheduler::printRooms() const
{
    for (std::unordered_map<std::string, ResourceInfo>::const_iterator it = resources.begin(); it != resources.end(); ++it)
    {
        std::cout << it->second.resourceID << " | cap:" << it->second.capacity << "\n";
    }
}

Event *Scheduler::findEventByID(std::string id)
{
    for (size_t i = 0; i < ownedEvents.size(); ++i)
    {
        if (ownedEvents[i]->eventID == id)
            return ownedEvents[i];
    }
    return NULL;
}

void Scheduler::syncCounters()
{
    int maxE = 1000;
    int maxP = 5000;

    for (size_t i = 0; i < ownedEvents.size(); ++i)
    {
        std::string eID = ownedEvents[i]->eventID;
        int eNum = std::stoi(eID.substr(4));
        if (eNum >= maxE)
            maxE = eNum + 1;

        std::queue<Participant> q = ownedEvents[i]->participantQueue;
        while (!q.empty())
        {
            int pNum = std::stoi(q.front().pID.substr(2));
            if (pNum >= maxP)
                maxP = pNum + 1;
            q.pop();
        }
    }
    nextEventID = maxE;
    nextParticipantID = maxP;
}

void Scheduler::clearAllEvents()
{
    for (auto &pair : resources)
        pair.second.tree.root = NULL;
    for (size_t i = 0; i < ownedEvents.size(); ++i)
        delete ownedEvents[i];
    ownedEvents.clear();
    while (!undoStack.empty())
        undoStack.pop();
    while (!redoStack.empty())
        redoStack.pop();
}