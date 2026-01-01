#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <queue>

struct Participant {
    std::string name;
    std::string pID;
};

class Event {
public:
    std::string eventID;
    std::string name;
    std::string date;
    std::string resourceID;

    int startMinutes; 
    int endMinutes;   
    int priority;      
    int maxParticipants;

    std::queue<Participant> participantQueue;
    std::queue<Participant> waitlistQueue;

    Event() : startMinutes(0), endMinutes(0), priority(1), maxParticipants(0) {}
};

#endif