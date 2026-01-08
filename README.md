# 🎓 Campus Event Scheduler System

## 📌 Overview
The Campus Event Scheduler System is a C++ console-based application designed to manage and schedule events across multiple campus rooms efficiently. The system uses core Data Structures and Algorithms to handle event conflicts, priority-based scheduling, participant registration, undo/redo operations, and room utilization reporting.

The application supports three roles:
- Admin
- Organizer
- Participant

---

## 🚀 Features

### Admin
- Add new rooms with capacity
- Delete rooms (only if no events exist)
- View all rooms
- Generate room utilization reports (date-wise or overall)

### Organizer
- Schedule events with date, time, room, and priority
- Automatic conflict detection using interval trees
- Priority-based event preemption
- Alternative time-slot suggestions
- Delete scheduled events
- Undo and redo scheduling actions
- View full event schedule

### Participant
- Register for events using FIFO order
- Automatic waitlist handling when events are full
- Withdraw from events
- Automatic promotion from waitlist

### Data Persistence
- Save all rooms, events, and participants to file
- Load saved data on application startup

---

## 🧠 Data Structures Used

| Data Structure | Purpose |
|---------------|--------|
| Interval Tree (BST-based) | Event conflict detection |
| Binary Search Tree | Ordered event storage |
| Queue (FIFO) | Participant registration and waitlist |
| Priority Queue (Heap) | Priority-based event preemption |
| Stack | Undo and redo operations |
| Vector | Event traversal and reporting |
| Unordered Map (Hash Table) | Fast room lookup |
| Set | Unique date tracking for utilization |
| String parsing (CSV) | File save/load |

---

## 🌳 Interval Tree (Core Algorithm)
Each room maintains an Interval Tree where:
- Each node represents an event time interval
- Nodes are ordered by start time
- Each node stores a `maxEnd` value for efficient overlap checking

### Overlap Condition
An event overlaps another event if:

startA < endB AND startB < endA


This enables conflict detection in **O(log n + k)** time.

---

## ⚙️ System Architecture
User Interface (CLI)

↓

Scheduler

↓

Room (Resource)

↓

Interval Tree

↓

Event Nodes


---

## 📂 File Structure

.

├── main.cpp

├── Scheduler.h

├── Scheduler.cpp

├── Event.h

├── IntervalTree.h

├── IntervalTree.cpp

├── IntervalTreeNode.h

├── IntervalTreeNode.cpp

├── campus_data.txt

└── README.md


---

## ▶️ How to Compile and Run

### Compile

g++ main.cpp Scheduler.cpp IntervalTree.cpp -o scheduler


### Run
./scheduler

---

## 💾 Data Persistence
- All data is stored in `campus_data.txt`
- Data is automatically loaded at startup and saved on exit

---

## 📊 Utilization Report
The utilization report calculates:
- Total minutes used per room
- Number of unique dates
- Utilization percentage using:
(total minutes used / (number of days × 1440)) × 100

---

## 🔄 Undo / Redo Mechanism
- Implemented using stacks
- Undo removes the most recently scheduled event
- Redo restores the last undone event
- Ensures reversible scheduling operations

---

## 🧪 Edge Case Handling
- Prevents events from crossing midnight
- Prevents deleting rooms with scheduled events
- Handles full events using waitlists
- Validates all user inputs

---

## 📈 Time Complexity Summary

| Operation | Complexity |
|---------|------------|
| Insert Event | O(log n) |
| Find Overlaps | O(log n + k) |
| Delete Event | O(log n) |
| Register Participant | O(1) |
| Undo / Redo | O(log n) |
| Save / Load | O(n) |


---

## 🧾 Project Summary
A campus event scheduling system implemented in C++ using interval trees for conflict detection, priority queues for event preemption, stacks for undo/redo operations, queues for participant management, and hash maps for fast room access.
