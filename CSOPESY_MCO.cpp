#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <vector>

using namespace std;

struct screenConsole{
    std::string processName;
    int totalLines;
    std::string creationDate;
};

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

std::string extractName(const std::string& command) {
    std::regex pattern(R"(screen\s+-[rs]\s+(\S+))");
    std::smatch match;
    if (std::regex_search(command, match, pattern) && match.size() > 1) {
        return match[1];
    }
    return "";
}

void printScreenConsole(const std::string& processName, std::vector<screenConsole>& vec2){
    for(const auto& process : vec2) {
        if (process.processName == processName) {
            cout << std::endl;
            cout << "==============================" << std::endl;
            cout << "Process Name: "<< process.processName << std::endl;
            cout << "Total lines of instruction: " << process.totalLines <<std::endl;
            cout << "Created on: "<< process.creationDate << std::endl;
            cout << "==============================" << std::endl;
            cout << std::endl;
        }
    }
}

void commandScreen(const std::string& command, const std::string& processName, std::vector<screenConsole>& vec2) {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    char time[100];
    std::strftime(time, sizeof(time), "%m/%d/%Y, %I:%M:%S %p", now);

    vec2.push_back({processName, 0, time});
    printScreenConsole(processName, vec2);
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

bool findCommand(const std::string& text, const std::string& command) {
    return text.find(command) != std::string::npos;
}

// void printVector(const std::vector<std::string>& vec) {
//     for (const auto& str : vec) {
//         std::cout << str << std::endl;
//     }
// }

void processCommand(const std::string& command, std::vector<std::string>& vec, std::vector<screenConsole>& vec2) {
    if (findCommand(command, "initialize")) {
    }
    else if (findCommand(command, "screen -r") || findCommand(command, "screen -s")) {
        std::string processName = extractName(command);
        if(std::find(vec.begin(), vec.end(), processName) != vec.end()){
            printScreenConsole(processName, vec2);
        } 
        else {
            vec.push_back(processName);
            commandScreen(command, processName, vec2);
        }
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
    // else if (command == "checkVec"){ //! Debug only
    //     printVector(vec);
    // }
    else {
        cout << "Unknown command: " << command << std::endl;
    }
}

int main() {
    std::string command;
    std::vector<std::string> processes;
    std::vector<screenConsole> consoles;
    clearScreen();
    printHeader();

    while (true) {
        cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen, or 'help' to view available commands\033[0m\n";
        cout << "Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command, processes, consoles);
    }

    return 0;
}
