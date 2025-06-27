#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <cstdint>

using namespace std;



class Screen {
private:
    string processName;
    int currentLine;
    int totalLines;
    string creationDate;
    bool isActive;
    map<string, uint16_t> variables;

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

    void declareVariable(const string& varName, uint16_t value = 0, bool randomize = false) {
    if (randomize) {
        srand(static_cast<unsigned int>(time(nullptr)));
        value = static_cast<uint16_t>(rand() % 65536);
    }
    variables[varName] = value;
}

    
    bool isVariableDeclared(const string& varName) const {
        return variables.find(varName) != variables.end();
    }

    
    void ensureVariableExists(const string& varName) {
    if (!isVariableDeclared(varName)) {
        declareVariable(varName, 0, true);
    }
}

    void setVariable(const string& varName, uint16_t value) {
        variables[varName] = value;
    }

    uint16_t getVariable(const string& varName) const {
        auto it = variables.find(varName);
        if (it != variables.end()) {
            return it->second;
        } else {
            
            return 0;
        }
    }

    
    void performAdd(const string& targetVar, const string& source1, const string& source2) {
        ensureVariableExists(targetVar);
    
        uint16_t val1;
        if (all_of(source1.begin(), source1.end(), ::isdigit)) {
            int num = stoi(source1);
            val1 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
        } else {
            ensureVariableExists(source1);
            val1 = getVariable(source1);
        }
        
        uint16_t val2;
        if (all_of(source2.begin(), source2.end(), ::isdigit)) {
            int num = stoi(source2);
            val2 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
        } else {
            ensureVariableExists(source2);
            val2 = getVariable(source2);
        }
        
        
        uint32_t result = static_cast<uint32_t>(val1) + static_cast<uint32_t>(val2);
        uint16_t finalValue = (result > 65535) ? 65535 : static_cast<uint16_t>(result);
        
        
        setVariable(targetVar, finalValue);
        
        cout << "ADD operation: " << targetVar << " = " << val1 << " + " << val2 << " = " << finalValue << endl;
        if (result > 65535) {
            cout << "Note: Result capped at 65535 due to uint16_t overflow." << endl;
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
        string addPrefix = "ADD(";
        string subtractPrefix = "SUBTRACT(";
        char commandSuffix = ')';

        int printPos = command.find(printPrefix);
        int printStart = printPos + printPrefix.size();
        int printEnd = command.find(')', printStart);

        int declarePos = command.find(declarePrefix);
        int declareStart = declarePos + declarePrefix.size();
        int declareEnd = command.find(')', declareStart);

        int addPos = command.find(addPrefix);
        int addStart = addPos + addPrefix.size();
        int addEnd = command.find(')', addStart);

        int subtractPos = command.find(subtractPrefix);
        int subtractStart = subtractPos + subtractPrefix.size();
        int subtractEnd = command.find(')', subtractStart);

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
        
        else if (addPos != string::npos && addEnd != string::npos) {
        string addValues = extractCommandValue(command, "ADD");
        if(addValues != ""){
        
        vector<string> args;
        size_t start = 0;
        size_t end = addValues.find(',');
        
        
        while (end != string::npos && args.size() < 2) {
            string arg = addValues.substr(start, end - start);
            arg.erase(0, arg.find_first_not_of(" \t\n\r\f\v"));
            arg.erase(arg.find_last_not_of(" \t\n\r\f\v") + 1);
            args.push_back(arg);
            start = end + 1;
            end = addValues.find(',', start);
        }
        
       
        if (start < addValues.length()) {
            string lastArg = addValues.substr(start);
            lastArg.erase(0, lastArg.find_first_not_of(" \t\n\r\f\v"));
            lastArg.erase(lastArg.find_last_not_of(" \t\n\r\f\v") + 1);
            args.push_back(lastArg);
        }

        if (args.size() == 3) {
            string targetVar = args[0];
            string source1 = args[1];
            string source2 = args[2];
            
    
            currentScreen->ensureVariableExists(targetVar);
            
            
            uint16_t val1;
            if (all_of(source1.begin(), source1.end(), ::isdigit)) {
                int num = stoi(source1);
                val1 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
            } else {
                currentScreen->ensureVariableExists(source1);
                val1 = currentScreen->getVariable(source1);
            }

            uint16_t val2;
            if (all_of(source2.begin(), source2.end(), ::isdigit)) {
                int num = stoi(source2);
                val2 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
            } else {
                currentScreen->ensureVariableExists(source2);
                val2 = currentScreen->getVariable(source2);
            }
            
            
            uint32_t result = static_cast<uint32_t>(val1) + static_cast<uint32_t>(val2);
            uint16_t finalValue = (result > 65535) ? 65535 : static_cast<uint16_t>(result);
            
            uint16_t oldValue = currentScreen->getVariable(targetVar);
            currentScreen->setVariable(targetVar, finalValue);
            
            
            cout << "debug: " << targetVar << " = " << val1 << " + " << val2 
                 << " = " << finalValue;
            if (oldValue != finalValue) {
                cout << " (" << oldValue << " =" << finalValue << ")";
            }
            cout << endl;
            
            if (result > 65535) {
                cout << "Note: Result capped at 65535 due to uint16_t overflow." << endl;
            }
            
            currentScreen->simulateProgress();
            cout << "Command completed. Progress updated." << endl;
            currentScreen->display();
        }
        else {
            cout << "Wrong args for ADD. Must be ADD(target, source1, source2)." << endl;
        }
    }
    else {
        cout << "ADD arg cannot be empty." << endl;
    }
}

else if (subtractPos != string::npos && subtractEnd != string::npos) {
    string subtractValues = extractCommandValue(command, "SUBTRACT");
    if(subtractValues != ""){
        vector<string> args;
        size_t start = 0;
        size_t end = subtractValues.find(',');
        
        
        while (end != string::npos && args.size() < 2) {
            string arg = subtractValues.substr(start, end - start);
            arg.erase(0, arg.find_first_not_of(" \t\n\r\f\v"));
            arg.erase(arg.find_last_not_of(" \t\n\r\f\v") + 1);
            args.push_back(arg);
            start = end + 1;
            end = subtractValues.find(',', start);
        }
        
        if (start < subtractValues.length()) {
            string lastArg = subtractValues.substr(start);
            lastArg.erase(0, lastArg.find_first_not_of(" \t\n\r\f\v"));
            lastArg.erase(lastArg.find_last_not_of(" \t\n\r\f\v") + 1);
            args.push_back(lastArg);
        }

        if (args.size() == 3) {
            string targetVar = args[0];
            string source1 = args[1];
            string source2 = args[2];
            
            
            currentScreen->ensureVariableExists(targetVar);
            
            
            uint16_t val1;
            if (all_of(source1.begin(), source1.end(), ::isdigit)) {
                int num = stoi(source1);
                val1 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
            } else {
                currentScreen->ensureVariableExists(source1);
                val1 = currentScreen->getVariable(source1);
            }

    
            uint16_t val2;
            if (all_of(source2.begin(), source2.end(), ::isdigit)) {
                int num = stoi(source2);
                val2 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
            } else {
                currentScreen->ensureVariableExists(source2);
                val2 = currentScreen->getVariable(source2);
            }
            
            
            uint16_t finalValue;
            if (val1 >= val2) {
                finalValue = val1 - val2;
            } else {
                finalValue = 0;  
            }
            
            uint16_t oldValue = currentScreen->getVariable(targetVar);
            currentScreen->setVariable(targetVar, finalValue);
           
            cout << "debug: " << targetVar << " = " << val1 << " - " << val2 
                 << " = " << finalValue << endl;
            
            if (val1 < val2) {
                cout << "Note: Result capped at 0 due to uint16_t underflow prevention." << endl;
            }
            
           
            currentScreen->simulateProgress();
            cout << "Command completed. Progress updated." << endl;
            currentScreen->display();
        }
        else {
            cout << "Wrong args for SUBTRACT. Must be SUBTRACT(target, source1, source2)." << endl;
        }
    }
    else {
        cout << "SUBTRACT arg cannot be empty." << endl;
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