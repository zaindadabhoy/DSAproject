#include <iostream>
#include "Scheduler.h"

int main() {
    Scheduler scheduler;
    int choice;
    while (true) {
        std::cout << "\n1) Schedule\n2) View\n3) Exit\nChoice: ";
        std::cin >> choice;
        if (choice == 1) scheduler.scheduleEventUI();
        else if (choice == 2) scheduler.printSchedule();
        else break;
    }
    return 0;
}