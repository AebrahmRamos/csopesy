#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

using namespace std;



class Screen {
private:
    string processName;
    int currentLine;
    int totalLines;
    string creationDate;
    bool isActive;

public:
    Screen(const string& name) : processName(name), currentLine(1), totalLines(100), isActive(true) {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        char timeBuffer[100];
        strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", now);
        creationDate = string(timeBuffer);
    }

    void display() {
        cout << "\n==============================" << endl;
        cout << "Process Name: " << processName << endl;
        cout << "Current line of instruction: " << currentLine << " / " << totalLines << endl;
        cout << "Created on: " << creationDate << endl;
        cout << "==============================" << endl;
        cout << "\033[33m(Type 'exit' to return to main menu)\033[0m" << endl;
    }

    void simulateProgress() {
        if (currentLine < totalLines) {
            currentLine++;
        }
    }

    // Getters
    string getName() const { return processName; }
    int getCurrentLine() const { return currentLine; }
    int getTotalLines() const { return totalLines; }
    string getCreationDate() const { return creationDate; }
    bool getIsActive() const { return isActive; }

    // Setters
    void setActive(bool active) { isActive = active; }
};

class ConsoleManager {
private:
    map<string, shared_ptr<Screen>> screens;
    shared_ptr<Screen> currentScreen;
    bool inMainMenu;
    bool initialized = false;

    string extractName(const string& command) {
        regex pattern(R"(screen\s+-[rs]\s+(\S+))");
        smatch match;
        if (regex_search(command, match, pattern) && match.size() > 1) {
            return match[1];
        }
        return "";
    }

    bool findCommand(const string& text, const string& command) {
        return text.find(command) != string::npos;
    }

public:
    ConsoleManager() : currentScreen(nullptr), inMainMenu(true) {}

    void clearScreen() {
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
    }

    void printHeader() {
        cout << "   ___________ ____  ____  _____________  __\n";
        cout << "  / ____/ ___// __ \\/ __ \\/ ____/ ___/\\ \\/ /\n";
        cout << " / /    \\__ \\/ / / / /_/ / __/  \\__ \\  \\  /\n";
        cout << "/ /___ ___/ / /_/ / ____/ /___ ___/ /  / /\n";
        cout << "\\____//____/\\____/_/   /_____//____/  /_/\n";
        cout << "\033[32m      Welcome to CSOPESY Command Line\033[0m\n\n";
    }

    void commandHelp() {
        if (inMainMenu) {
            cout << "\n\033[32m=== CSOPESY Command Help ===\033[0m\n";
            cout << "  initialize     - Initialize the system\n";
            cout << "  screen -s <n>  - Create a new screen session\n";
            cout << "  screen -r <n>  - Resume an existing screen session\n";
            cout << "  screen -ls     - List all screen sessions\n";
            cout << "  scheduler-test - Run scheduler test\n";
            cout << "  scheduler-stop - Stop scheduler\n";
            cout << "  report-util    - Generate report\n";
            cout << "  clear          - Clear the screen\n";
            cout << "  help           - Show this help menu\n";
            cout << "  exit           - Exit the application\n";
        } else {
            cout << "\n\033[32m=== Screen Session Help ===\033[0m\n";
            cout << "  exit - Return to main menu\n";
            cout << "  help - Show this help menu\n";
            cout << "  Any other command will simulate process execution\n";
        }
    }

    void commandInitialize() {
        cout << "initialize command recognized. System initialized." << endl;
    }

    void commandSchedulerTest() {
        cout << "scheduler-test command recognized. Running scheduler test." << endl;
    }

    void commandSchedulerStop() {
        cout << "scheduler-stop command recognized. Scheduler stopped." << endl;
    }

    void commandReportUtil() {
        cout << "report-util command recognized. Generating report." << endl;
    }

    void commandClear() {
        cout << "clear command recognized. Clearing screen." << endl;
        clearScreen();
        if (!inMainMenu && currentScreen) {
            currentScreen->display();
        } 
        else{
            printHeader();  
        }
    }

    void commandExit() {
        if (inMainMenu) {
            cout << "exit command recognized. Closing application." << endl;
            exit(0);
        } else {
            cout << "Returning to main menu..." << endl;
            currentScreen = nullptr;
            inMainMenu = true;
            clearScreen();
            printHeader();
        }
    }

    void createScreen(const string& name) {
        auto screen = make_shared<Screen>(name);
        screens[name] = screen;
        currentScreen = screen;
        inMainMenu = false;

        clearScreen();
        cout << "Screen session '" << name << "' created successfully." << endl;
        screen->display();
    }

    void resumeScreen(const string& name) {
        auto it = screens.find(name);
        if (it != screens.end()) {
            currentScreen = it->second;
            inMainMenu = false;

            clearScreen();
            cout << "Resuming screen session '" << name << "'..." << endl;
            currentScreen->display();
        } else {
            cout << "Screen '" << name << "' not found." << endl;
        }
    }

    void listScreens() {
        if (screens.empty()) {
            cout << "No screen sessions found." << endl;
            return;
        }

        cout << "\n\033[32m=== Active Screen Sessions ===\033[0m\n";
        for (const auto& pair : screens) {
            cout << "  • " << pair.first << " (Created: " << pair.second->getCreationDate() << ")" << endl;
        }
        cout << endl;
    }

    void handleScreenCommand(const string& command) {
        string processName = extractName(command);
        
        if (processName.empty()) {
            cout << "Invalid screen command format." << endl;
            cout << "Usage: screen -s <name> or screen -r <name>" << endl;
            return;
        }

        auto it = screens.find(processName);
        
        if (findCommand(command, "screen -s")) {
            if (it != screens.end()) {
                cout << "Screen '" << processName << "' already exists. Use 'screen -r " << processName << "' to resume." << endl;
            } else {
                createScreen(processName);
            }
        }
        else if (findCommand(command, "screen -r")) {
            if (it != screens.end()) {
                resumeScreen(processName);
            } else {
                cout << "Screen '" << processName << "' not found. Use 'screen -s " << processName << "' to create." << endl;
            }
        }
    }

    void processMainMenuCommand(const string& command) {
        if (command == "initialize") {
            commandInitialize();
            initialized = true;
        }
        else if ((findCommand(command, "screen -r") || findCommand(command, "screen -s")) && initialized == true) {
            handleScreenCommand(command);
        }
        else if (command == "screen -ls" && initialized == true) {
            listScreens();
        }
        else if (command == "scheduler-test" && initialized == true) {
            commandSchedulerTest();
        }
        else if (command == "scheduler-stop" && initialized == true) {
            commandSchedulerStop();
        }
        else if (command == "report-util" && initialized == true) {
            commandReportUtil();
        }
        else if (command == "clear" && initialized == true) {
            commandClear();
        }
        else if (command == "exit") {
            commandExit();
        }
        else if (command == "help" && initialized == true) {
            commandHelp();
        }
        else if (initialized == false && command != "initialize" && command != "exit") {
            cout << "Please initialize the OS first." << endl;
        }
        else {
            cout << "Unknown command: " << command << endl;
            cout << "Type 'help' for available commands." << endl;
        }
    }

    

    string extractCommandValue(const std::string& command, const std::string type) {
        string prefix = type + "(";
        int pos = command.find(prefix);
        int start = pos + prefix.size();
        int end = command.find(')', start);
        if (start == end){
            return "";
        }

        string value = command.substr(start, end - start);
        return value;
    }

    void processScreenCommand(const string& command) {
        string printPrefix = "PRINT(";
        string declarePrefix = "DECLARE(";
        char commandSuffix = ')';

        int printPos = command.find(printPrefix);
        int printStart = printPos + printPrefix.size();
        int printEnd = command.find(')', printStart);

        int declarePos = command.find(declarePrefix);
        int declareStart = declarePos + declarePrefix.size();
        int declareEnd = command.find(')', declareStart);


        if (command == "exit") {
            commandExit();
        }
        else if (command == "help") {
            commandHelp();
        }
        else if (command == "clear") {
            commandClear();
        }
        else if (printPos != string::npos && printEnd != string::npos) {
            string printMsg = extractCommandValue(command, "PRINT");
            if(printMsg != ""){
                cout << printMsg << endl;
                currentScreen->simulateProgress();
                cout << "Command completed. Progress updated." << endl;
                currentScreen->display();
            }
            else{
                cout << "PRINT arg cannot be empty."<< endl;
            }
            
        }
        else if (declarePos != string::npos && declareEnd != string::npos) {
            string declareValues = extractCommandValue(command, "DECLARE");
            if(declareValues!= ""){
                int commaPos = declareValues.find(',');
                bool correctArgs = true;
                if (commaPos != string::npos) {
                    string var = declareValues.substr(0, commaPos);
                    int spacePos = declareValues.find(' ');
                    string temp;
                    if(spacePos != string::npos){
                        temp = declareValues.substr(commaPos + 2);
                    } else {
                        temp = declareValues.substr(commaPos + 1);
                    }
                    
                    bool isNumber = all_of(temp.begin(), temp.end(), ::isdigit);

                    uint16_t value;
                    if(isNumber){
                        int conTemp = stoi(temp);
                        if (conTemp < 0 || conTemp > 65535) {
                            cout << "Value out of range for uint16_t." << endl;
                            correctArgs = false;
                        } else {
                            value = static_cast<uint16_t>(conTemp);

                        }
                    }
                    else {
                        correctArgs = false;
                    }

                    if (correctArgs == true){
                        cout << "var: " << var << endl;
                        cout << "value: " << value << endl;
                    }
                    else {
                        cout << "Wrong args for DECLARE. Must be DECLARE(var, value), where value must be a uint16 number (0 - 65535)." << endl;
                    }
                } 
                else {
                    cout << "Wrong args for DECLARE. Must be DECLARE(var, value), where value must be a uint16 number (0 - 65535)." << endl;
                }
            }
            else{
                cout << "DECLARE arg cannot be empty."<< endl;
            }
        }
        else {
            cout << "Executing command in screen '" << currentScreen->getName() << "': " << command << endl;
            currentScreen->simulateProgress();
            cout << "Command completed. Progress updated." << endl;
            currentScreen->display();
        }
    }

    void processCommand(const string& command) {
        if (inMainMenu) {
            processMainMenuCommand(command);
        } else {
            processScreenCommand(command);
        }
    }

    void showPrompt() {
        if (inMainMenu) {
            cout << "\033[33mType 'exit' to quit, 'clear' to clear screen, 'help' for commands\033[0m\n";
            cout << "Enter a command: ";
        } else {
            cout << "\n\033[33m[Screen: " << currentScreen->getName() << "] Enter command (or 'exit' to return): \033[0m";
        }
    }

    void run() {
        string command;
        clearScreen();
        printHeader();
        
        while (true) {
            showPrompt();
            getline(cin, command);
            processCommand(command);
        }
    }
};

int main() {
    ConsoleManager manager;
    manager.run();
    return 0;
}