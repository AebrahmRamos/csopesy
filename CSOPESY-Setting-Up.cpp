#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

void clearScreen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

void commandHelp() {
    cout << "\n\033[32m=== CSOPESY Command Help ===\033[0m\n";
    cout << "  initialize     - Do Something\n";
    cout << "  screen         - Do Something\n";
    cout << "  scheduler-test - Do Something\n";
    cout << "  scheduler-stop - Do Something\n";
    cout << "  report-util    - Do Something\n";
    cout << "  clear          - Do Something\n";
    cout << "  help           - Do Something\n";
    cout << "  exit           - Do Something\n";
}

void printHeader() {
    cout << "   ___________ ____  ____  _____________  __\n";
    cout << "  / ____/ ___// __ \\/ __ \\/ ____/ ___/\\ \\/ /\n";
    cout << " / /    \\__ \\/ / / / /_/ / __/  \\__ \\  \\  /\n";
    cout << "/ /___ ___/ / /_/ / ____/ /___ ___/ /  / /\n";
    cout << "\\____//____/\\____/_/   /_____//____/  /_/\n";
    cout << "\033[32m      Welcome to CSOPESY Command Line\033[0m\n\n";
}

void commandInitialize() {
    cout << "initialize command recognized. Doing something." << std::endl;
}

void commandScreen() {
    cout << "screen command recognized. Doing something." << std::endl;
}

void commandSchedulerTest() {
    cout << "scheduler-test command recognized. Doing something." << std::endl;
}

void commandSchedulerStop() {
    cout << "scheduler-stop command recognized. Doing something." << std::endl;
}

void commandReportUtil() {
    cout << "report-util command recognized. Doing something." << std::endl;
}

void commandClear() {
    cout << "clear command recognized. Clearing screen." << std::endl;
    clearScreen();
    printHeader();
}

void commandExit() {
    cout << "exit command recognized. Closing application." << std::endl;
    exit(0);
}

void processCommand(const std::string& command) {
    if (command == "initialize") {
        commandInitialize();
    }
    else if (command == "screen") {
        commandScreen();
    }
    else if (command == "scheduler-test") {
        commandSchedulerTest();
    }
    else if (command == "scheduler-stop") {
        commandSchedulerStop();
    }
    else if (command == "report-util") {
        commandReportUtil();
    }
    else if (command == "clear") {
        commandClear();
    }
    else if (command == "exit") {
        commandExit();
    }
    else if (command == "help") {
        commandHelp();
    }
    else {
        cout << "Unknown command: " << command << std::endl;
    }
}

int main() {
    std::string command;

    clearScreen();
    printHeader();

    while (true) {
        cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen, or 'help' to view available commands\033[0m\n";
        cout << "Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command);
    }

    return 0;
}
