#include <iostream>
#include <string>
#include "Scheduler.h"

int main() {
    Scheduler scheduler;
    int choice = 0;
    
    scheduler.loadFromFile("campus_data.txt");

    while (true) {
        std::cout << "\n===== Campus Event Scheduler =====\n";
        std::cout << "1) Admin\n2) Organizer\n3) Participant\n4) Save\n5) Load\n6) Exit\nChoice: ";
        if (!(std::cin >> choice)) break;

        if (choice == 1) {
            bool adminLoop = true;
            while (adminLoop) {
                std::cout << "\n--- ADMIN MENU ---\n";
                std::cout << "1) Add Room\n";
                std::cout << "2) Delete Room\n";
                std::cout << "3) View Rooms\n";
                std::cout << "4) Utilization Report\n";
                std::cout << "5) Back\n";
                std::cout << "Choice: ";
                int adminChoice; 
                std::cin >> adminChoice;

                if (adminChoice == 1) {
                    scheduler.addRoomUI();
                } else if (adminChoice == 2) {
                    scheduler.deleteRoomUI();
                } else if (adminChoice == 3) {
                    scheduler.printRooms();
                } else if (adminChoice == 4) {
    				scheduler.printUtilizationReportUI();
				}  else if (adminChoice == 5) {
                    adminLoop = false;
                } else {
                    std::cout << "Invalid choice.\n";
                }
            }
        }
        else if (choice == 2) {
            bool orgLoop = true;
            while (orgLoop) {
                std::cout << "\n--- ORGANIZER MENU ---\n1) Schedule Event\n2) Delete Event\n3) View Schedule\n4) Undo\n5) Redo\n6) Back\nChoice: ";
                int orgChoice; std::cin >> orgChoice;
                switch (orgChoice) {
                    case 1: scheduler.scheduleEventUI(); break;
                    case 2: scheduler.deleteEventUI(); break;
                    case 3: scheduler.printSchedule(); break;
                    case 4: scheduler.undo(); break;
                    case 5: scheduler.redo(); break;
                    case 6: orgLoop = false; break;
                    default: std::cout << "Invalid.\n";
                }
            }
        } 
        else if (choice == 3) {
            bool partLoop = true;
            while (partLoop) {
                std::cout << "\n--- PARTICIPANT MENU ---\n1) Register for Event\n2) Withdraw from Event\n3) View Schedule\n4) Back\nChoice: ";
                int partChoice; std::cin >> partChoice;
                switch (partChoice) {
                    case 1: scheduler.registerParticipantUI(); break;
                    case 2: scheduler.withdrawParticipantUI(); break;
                    case 3: scheduler.printSchedule(); break;
                    case 4: partLoop = false; break;
                    default: std::cout << "Invalid.\n";
                }
            }
        } 
        else if (choice == 4) {
            scheduler.saveToFile("campus_data.txt");
            std::cout << "Data saved.\n";
        } 
        else if (choice == 5) {
            scheduler.loadFromFile("campus_data.txt");
            std::cout << "Data loaded.\n";
        } 
        else if (choice == 6) {
            scheduler.saveToFile("campus_data.txt");
            std::cout << "Saved. Exiting.\n";
            break;
        }
    }
    return 0;
}