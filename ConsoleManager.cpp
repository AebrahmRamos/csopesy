#include "ConsoleManager.h"
#include <iostream>
#include <regex>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

ConsoleManager::ConsoleManager() : currentScreen(nullptr), inMainMenu(true), initialized(false) {
    processManager = std::make_unique<ProcessManager>();
    reportGenerator = std::make_unique<ReportGenerator>();
    processManager->initialize();
    processManager->startScheduler();
}

ConsoleManager::~ConsoleManager() {
    if (processManager) {
        processManager->stopScheduler();
    }
}

std::string ConsoleManager::extractName(const std::string& command) {
    std::regex pattern(R"(screen\s+-[rs]\s+(\S+))");
    std::smatch match;
    if (std::regex_search(command, match, pattern) && match.size() > 1) {
        return match[1];
    }
    return "";
}

bool ConsoleManager::findCommand(const std::string& text, const std::string& command) {
    return text.find(command) != std::string::npos;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string ConsoleManager::extractCommandValue(const std::string& command, const std::string type) {
    std::string prefix, value;
    int pos, start, end;
    
    if (type == "PRINT") {
        prefix = "PRINT(\"";
        pos = command.find(prefix);
        if (pos == std::string::npos) {
            return ""; // Return empty if "PRINT(" not found
        }
        start = pos + prefix.size();

        // Check if there's variable concatenation
        int plusPos = command.find("\" + ", start);
        if (plusPos != std::string::npos) {
            // Extract the string part before " + "
            std::string stringPart = command.substr(start, plusPos - start);
            
            // Extract the variable name after " + "
            int varStart = plusPos + 4; // Skip "\" + "
            int varEnd = command.find(')', varStart);
            if (varEnd == std::string::npos) {
                return "";
            }
            std::string varName = command.substr(varStart, varEnd - varStart);
            
            // Look up the variable value
            if (this->declaredVariables.find(varName) != declaredVariables.end()) {
                uint16_t varValue = declaredVariables[varName];
                return stringPart + std::to_string(varValue);
            } else {
                std::cout << "Error: Variable '" << varName << "' is not declared." << std::endl;
                return "";
            }
        } else {
            // Simple string without variables
            end = command.find("\")", start);
            if (start == end || end == std::string::npos) {
                return "";
            }
            return command.substr(start, end - start);
        }
    } else {
        prefix = type + "(";
        pos = command.find(prefix);
        start = pos + prefix.size();
        end = command.find(')', start);

        if (start == end || end == std::string::npos) {
            return "";
        }

        std::string value = command.substr(start, end - start);
        return value;
    }

    return "";
}



bool ConsoleManager::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        config.errorMessage = "Could not open config file: " + filename;
        config.isValid = false;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string key, value;
        if (!(iss >> key >> value)) continue;
        
        if (key == "num-cpu") {
            config.numCpu = std::stoi(value);
        } else if (key == "scheduler") {
            config.scheduler = value;
        } else if (key == "quantum-cycles") {
            config.quantumCycles = std::stoi(value);
        } else if (key == "batch-process-freq") {
            config.batchProcessFreq = std::stoi(value);
        } else if (key == "min-ins") {
            config.minIns = std::stoi(value);
        } else if (key == "max-ins") {
            config.maxIns = std::stoi(value);
        } else if (key == "delays-per-exec") {
            config.delaysPerExec = std::stoi(value);
        }
    }
    
    file.close();
    return validateConfig();
}

bool ConsoleManager::validateConfig() {
    // Validate num-cpu if within 1 - 128
    if (config.numCpu < 1 || config.numCpu > 128) {
        config.errorMessage = "num-cpu must be between 1 and 128. Got: " + std::to_string(config.numCpu);
        config.isValid = false;
        return false;
    }
    
    // Checks if valid scheduler choice
    if (config.scheduler != "fcfs" && config.scheduler != "rr") {
        config.errorMessage = "scheduler must be 'fcfs' or 'rr'. Got: " + config.scheduler;
        config.isValid = false;
        return false;
    }
    
    // Checks if quyantum-cycles is valid
    if (config.quantumCycles < 1) {
        config.errorMessage = "quantum-cycles must be >= 1. Got: " + std::to_string(config.quantumCycles);
        config.isValid = false;
        return false;
    }
    
    // Validate batch-process-freq
    if (config.batchProcessFreq < 1) {
        config.errorMessage = "batch-process-freq must be >= 1. Got: " + std::to_string(config.batchProcessFreq);
        config.isValid = false;
        return false;
    }
    
    // Validate min-ins [1, ∞]
    if (config.minIns < 1) {
        config.errorMessage = "min-ins must be >= 1. Got: " + std::to_string(config.minIns);
        config.isValid = false;
        return false;
    }
    
    // Validate max-ins [1, ∞] and max-ins >= min-ins
    if (config.maxIns < 1) {
        config.errorMessage = "max-ins must be >= 1. Got: " + std::to_string(config.maxIns);
        config.isValid = false;
        return false;
    }
    
    if (config.maxIns < config.minIns) {
        config.errorMessage = "max-ins must be >= min-ins. Got max: " + std::to_string(config.maxIns) + ", min: " + std::to_string(config.minIns);
        config.isValid = false;
        return false;
    }
    
    // Validate delays-per-exec [0, ∞]
    if (config.delaysPerExec < 0) {
        config.errorMessage = "delays-per-exec must be >= 0. Got: " + std::to_string(config.delaysPerExec);
        config.isValid = false;
        return false;
    }
    
    config.isValid = true;
    config.errorMessage = "";
    return true;
}

void ConsoleManager::printConfigError(const std::string& error) {
    std::cout << "\033[31m[CONFIG ERROR]\033[0m " << error << std::endl;
}

void ConsoleManager::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleManager::printHeader() {
    std::cout << "   ___________ ____  ____  _____________  __\n";
    std::cout << "  / ____/ ___// __ \\/ __ \\/ ____/ ___/\\ \\/ /\n";
    std::cout << " / /    \\__ \\/ / / / /_/ / __/  \\__ \\  \\  /\n";
    std::cout << "/ /___ ___/ / /_/ / ____/ /___ ___/ /  / /\n";
    std::cout << "\\____//____/\\____/_/   /_____//____/  /_/\n";
    std::cout << "\033[32m      Welcome to CSOPESY Command Line\033[0m\n\n";
}

void ConsoleManager::commandHelp() {
    if (inMainMenu) {
        std::cout << "\n\033[32m=== CSOPESY Command Help ===\033[0m\n";
        std::cout << "  initialize     - Initialize the system\n";
        std::cout << "  screen -s <n>  - Create a new screen session\n";
        std::cout << "  screen -r <n>  - Resume an existing screen session\n";
        std::cout << "  screen -ls     - List all screen sessions and CPU utilization\n";
        std::cout << "  scheduler-start - Start automatic process generation\n";
        std::cout << "  scheduler-stop - Stop scheduler\n";
        std::cout << "  report-util    - Generate report\n";
        std::cout << "  clear          - Clear the screen\n";
        std::cout << "  help           - Show this help menu\n";
        std::cout << "  exit           - Exit the application\n";
        std::cout << "  nvidia-smi     - Shows GPU summary and running processes\n";
    } else {
        std::cout << "\n\033[32m=== Screen Session Help ===\033[0m\n";
        std::cout << "  process-smi - Show process information\n";
        std::cout << "  exit - Return to main menu\n";
        std::cout << "  help - Show this help menu\n";
        std::cout << "  Any other command will simulate process execution\n";
    }
}

void ConsoleManager::commandInitialize() {
    std::cout << "Reading config.txt..." << std::endl;      // Loading message
    
    if (loadConfig("config.txt")) {
        std::cout << "\033[32m[CONFIG SUCCESS]\033[0m Configuration loaded successfully!" << std::endl;
        std::cout << "  CPU Cores: " << config.numCpu << std::endl;
        std::cout << "  Scheduler: " << config.scheduler << std::endl;
        std::cout << "  Quantum Cycles: " << config.quantumCycles << std::endl;
        std::cout << "  Batch Process Frequency: " << config.batchProcessFreq << std::endl;
        std::cout << "  Min Instructions: " << config.minIns << std::endl;
        std::cout << "  Max Instructions: " << config.maxIns << std::endl;
        std::cout << "  Delays per Execution: " << config.delaysPerExec << std: :endl;
        std::cout << "System initialized successfully." << std::endl;
        
        // Pass config to ProcessManager
        processManager->setConfig(config);
        
        initialized = true;
    } else {
        printConfigError(config.errorMessage);
        std::cout << "System initialization failed. Please fix config.txt and try again." << std::endl;
        initialized = false;
    }
}

void ConsoleManager::commandSchedulerStart() {
    if (processManager) {
        if (!processManager->isGeneratingProcesses()) {
            processManager->startProcessGeneration();
        } else {
            std::cout << "Process generation is already running." << std::endl;
        }
    }
}

void ConsoleManager::commandSchedulerTest() {
    if (processManager) {
        processManager->showProcessStatus();
    }
}

void ConsoleManager::commandSchedulerStop() {
    if (processManager) {
        processManager->stopProcessGeneration();
    }
}

void ConsoleManager::commandReportUtil() {
    if (reportGenerator && processManager) {
        reportGenerator->generateReport(processManager.get(), "csopesy-log.txt");
        std::cout << "Report generated and saved to csopesy-log.txt" << std::endl;
    }
}

void ConsoleManager::commandNvidiaSmi() {
    std::cout << "\n";
    std::cout << "+-----------------------------------------------------------------------------------------+\n";
    std::cout << "| NVIDIA-SMI 535.86.10              Driver Version: 535.86.10      CUDA Version: 12.2     |\n";
    std::cout << "|-----------------------------------------+------------------------+----------------------|\n";
    std::cout << "| GPU  Name                  Persistence-M| Bus-Id          Disp.A | Volatile Uncorr. ECC |\n";
    std::cout << "| Fan  Temp   Perf           Pwr:Usage/Cap|           Memory-Usage | GPU-Util  Compute M. |\n";
    std::cout << "|                                         |                        |               MIG M. |\n";
    std::cout << "|=========================================+========================+======================|\n";
    
    // Get dummy GPU data and print it
    std::vector<GPUInfo> gpus = getDummyGPUData();
    for (const auto& gpu : gpus) {
        printGPUInfo(gpu);
    }
    
    std::cout << "+-----------------------------------------+------------------------+----------------------+\n";
    std::cout << "\n";
    std::cout << "+-----------------------------------------------------------------------------------------+\n";
    std::cout << "| Processes:                                                                              |\n";
    std::cout << "|  GPU   GI   CI        PID   Type   Process name                              GPU Memory |\n";
    std::cout << "|        ID   ID                                                               Usage      |\n";
    std::cout << "|=========================================================================================|\n";
    
    // Get real process data instead of dummy data
    std::vector<ProcessInfo> processes = getRealProcessData();
    for (const auto& process : processes) {
        printProcessInfo(process);
    }
    
    std::cout << "+-----------------------------------------------------------------------------------------+\n";
    std::cout << "\n";
}

void ConsoleManager::commandClear() {
    std::cout << "clear command recognized. Clearing screen." << std::endl;
    clearScreen();
    printHeader();
    if (!inMainMenu && currentScreen) {
        currentScreen->display();
    }
}

void ConsoleManager::commandExit() {
    if (inMainMenu) {
        std::cout << "exit command recognized. Closing application." << std::endl;
        exit(0);
    } else {
        std::cout << "Returning to main menu..." << std::endl;
        currentScreen = nullptr;
        inMainMenu = true;
        clearScreen();
        printHeader();
    }
}

void ConsoleManager::createScreen(const std::string& name) {
    auto screen = std::make_shared<Screen>(name);
    
    // Try to find and attach a real process with this name
    if (processManager) {
        auto process = processManager->findProcessByName(name);
        if (process) {
            screen->attachToProcess(process);
            std::cout << "Screen attached to existing process: " << name << std::endl;
        } else {
            std::cout << "No process found with name '" << name << "'. Screen created without attached process." << std::endl;
        }
    }
    
    screens[name] = screen;
    currentScreen = screen;
    inMainMenu = false;
    clearScreen();
    std::cout << "Screen session '" << name << "' created successfully." << std::endl;
    screen->display();
}

void ConsoleManager::resumeScreen(const std::string& name) {
    auto it = screens.find(name);
    if (it != screens.end()) {
        currentScreen = it->second;
        inMainMenu = false;
        clearScreen();
        std::cout << "Resuming screen session '" << name << "'..." << std::endl;
        currentScreen->display();
    } else {
        std::cout << "Screen '" << name << "' not found." << std::endl;
    }
}

void ConsoleManager::listScreens() {
    if (reportGenerator && processManager) {
        reportGenerator->displayReport(processManager.get());
    } else {
        std::cout << "No screen sessions found." << std::endl;
    }
}

void ConsoleManager::handleScreenCommand(const std::string& command) {
    std::string processName = extractName(command);
    if (processName.empty()) {
        std::cout << "Invalid screen command format." << std::endl;
        std::cout << "Usage: screen -s <name> or screen -r <name>" << std::endl;
        return;
    }
    
    auto it = screens.find(processName);
    
    if (findCommand(command, "screen -s")) {
        if (it != screens.end()) {
            std::cout << "Screen '" << processName << "' already exists. Use 'screen -r " << processName << "' to resume." << std::endl;
        } else {
            createScreen(processName);
        }
    }
    else if (findCommand(command, "screen -r")) {
        // Check if screen already exists
        if (it != screens.end()) {
            // Check if attached process is finished
            if (it->second->hasAttachedProcess() && !it->second->getAttachedProcess()->getIsActive()) {
                std::cout << "Process '" << processName << "' has finished execution and can no longer be accessed." << std::endl;
                return;
            }
            resumeScreen(processName);
        } else {
            // Screen doesn't exist, check if process exists
            if (processManager) {
                auto process = processManager->findProcessByName(processName);
                if (process) {
                    // Check if process is finished
                    if (!process->getIsActive()) {
                        std::cout << "Process '" << processName << "' has finished execution and can no longer be accessed." << std::endl;
                        return;
                    }
                    // Process exists and is active, create screen and attach process
                    auto screen = std::make_shared<Screen>(processName);
                    screen->attachToProcess(process);
                    screens[processName] = screen;
                    currentScreen = screen;
                    inMainMenu = false;
                    clearScreen();
                    std::cout << "Resuming screen session '" << processName << "'..." << std::endl;
                    screen->display();
                } else {
                    std::cout << "Process '" << processName << "' not found." << std::endl;
                }
            } else {
                std::cout << "Process manager not available." << std::endl;
            }
        }
    }
}

void ConsoleManager::processMainMenuCommand(const std::string& command) {
    if (command == "initialize") {
        commandInitialize();
    }
    else if (command == "scheduler-start" && initialized) {
        commandSchedulerStart();
    }
    else if (command == "scheduler-stop" && initialized) {
        commandSchedulerStop();
    }
    else if (command == "report-util" && initialized) {
        commandReportUtil();
    }
    else if (command == "nvidia-smi" && initialized) {
        commandNvidiaSmi();
    }
    else if (command == "clear" && initialized) {
        commandClear();
    }
    else if (command == "exit") {
        commandExit();
    }
    else if (command == "help" && initialized) {
        commandHelp();
    }
    else if (command == "screen -ls" && initialized) {
        listScreens();
    }
    else if (std::regex_match(command, std::regex(R"(screen\s+-[rs]\s+\S+)")) && initialized) {
        handleScreenCommand(command);
    }
    else if (!initialized && command != "initialize" && command != "exit") {
        std::cout << "Please initialize the OS first." << std::endl;
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for available commands." << std::endl;
    }
}

void ConsoleManager::processScreenCommand(const std::string& command) {
    if (command == "exit") {
        commandExit();
    }
    else if (command == "help") {
        commandHelp();
    }
    else if (command == "clear") {
        commandClear();
    }
    else if (command == "process-smi") {
        if (currentScreen) {
            currentScreen->showProcessInfo();
        }
    }
    else if (command.find("PRINT(\"") != std::string::npos) {
        std::string printMsg = extractCommandValue(command, "PRINT");
        if (!printMsg.empty()) {
            std::cout << printMsg << std::endl;
            currentScreen->simulateProgress();
            std::cout << "Command completed. Progress updated." << std::endl;
            currentScreen->display();
        }
        else {
            std::cout << "PRINT arg cannot be empty." << std::endl;
        }
    }
    else if (command.find("DECLARE(") != std::string::npos) {
        std::string declareValue = extractCommandValue(command, "DECLARE");
        if(declareValue != ""){
            int commaPos = declareValue.find(',');
            bool correctArgs = true;
            if (commaPos != std::string::npos) {
                std::string var = declareValue.substr(0, commaPos);
                int spacePos = declareValue.find(' ');
                std::string temp;
                if(spacePos != std::string::npos){
                    temp = declareValue.substr(commaPos + 2);
                } else {
                    temp = declareValue.substr(commaPos + 1);
                }
                
                bool isNumber = std::all_of(temp.begin(), temp.end(), ::isdigit);

                uint16_t value;
                if(isNumber){
                    int conTemp = std::stoi(temp);
                    if (conTemp < 0 || conTemp > 65535) {
                        std::cout << "Value out of range for uint16_t." << std::endl;
                        correctArgs = false;
                    } else {
                        value = static_cast<uint16_t>(conTemp);
                    }
                }
                else {
                    correctArgs = false;
                }

                if (correctArgs == true){
                    declaredVariables[var] = value;
                    std::cout << "Variable '" << var << "' declared with value " << value << std::endl;
                    currentScreen->simulateProgress();
                    std::cout << "Command completed. Progress updated." << std::endl;
                    currentScreen->display();
                }
                else {
                    std::cout << "Wrong args for DECLARE. Must be DECLARE(var, value), where value must be a uint16 number (0 - 65535)." << std::endl;
                }
            } 
            else {
                std::cout << "Wrong args for DECLARE. Must be DECLARE(var, value), where value must be a uint16 number (0 - 65535)." << std::endl;
            }
        } else{
            std::cout << "DECLARE arg cannot be empty."<< std::endl;
        }
    }
    else if (command.find("ADD(") != std::string::npos) {
        std::string addValues = extractCommandValue(command, "ADD");
        if(addValues != ""){
            std::vector<std::string> args = parseCommaSeparatedArgs(addValues);

            if (args.size() == 3) {
                std::string targetVar = args[0];
                std::string source1 = args[1];
                std::string source2 = args[2];
                
                // Ensure target variable exists
                ensureVariableExists(targetVar);
                
                // Process source1
                uint16_t val1;
                if (std::all_of(source1.begin(), source1.end(), ::isdigit)) {
                    int num = std::stoi(source1);
                    val1 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
                } else {
                    ensureVariableExists(source1);
                    val1 = getVariableValue(source1);
                }

                // Process source2
                uint16_t val2;
                if (std::all_of(source2.begin(), source2.end(), ::isdigit)) {
                    int num = std::stoi(source2);
                    val2 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
                } else {
                    ensureVariableExists(source2);
                    val2 = getVariableValue(source2);
                }
                
                // Perform addition with overflow protection
                uint32_t result = static_cast<uint32_t>(val1) + static_cast<uint32_t>(val2);
                uint16_t finalValue = (result > 65535) ? 65535 : static_cast<uint16_t>(result);
                
                uint16_t oldValue = getVariableValue(targetVar);
                setVariableValue(targetVar, finalValue);
                
                // Output debug information (from teammates' implementation)
                std::cout << "debug: " << targetVar << " = " << val1 << " + " << val2 
                         << " = " << finalValue;
                if (oldValue != finalValue) {
                    std::cout << " (" << oldValue << " =" << finalValue << ")";
                }
                std::cout << std::endl;
                
                if (result > 65535) {
                    std::cout << "Note: Result capped at 65535 due to uint16_t overflow." << std::endl;
                }
                
                currentScreen->simulateProgress();
                std::cout << "Command completed. Progress updated." << std::endl;
                currentScreen->display();
            }
            else {
                std::cout << "Wrong args for ADD. Must be ADD(target, source1, source2)." << std::endl;
            }
        }
        else {
            std::cout << "ADD arg cannot be empty." << std::endl;
        }
    }
    else if (command.find("SUBTRACT(") != std::string::npos) {
        std::string subtractValues = extractCommandValue(command, "SUBTRACT");
        if(subtractValues != ""){
            std::vector<std::string> args = parseCommaSeparatedArgs(subtractValues);

            if (args.size() == 3) {
                std::string targetVar = args[0];
                std::string source1 = args[1];
                std::string source2 = args[2];
                
                // Ensure target variable exists
                ensureVariableExists(targetVar);
                
                // Process source1
                uint16_t val1;
                if (std::all_of(source1.begin(), source1.end(), ::isdigit)) {
                    int num = std::stoi(source1);
                    val1 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
                } else {
                    ensureVariableExists(source1);
                    val1 = getVariableValue(source1);
                }

                // Process source2
                uint16_t val2;
                if (std::all_of(source2.begin(), source2.end(), ::isdigit)) {
                    int num = std::stoi(source2);
                    val2 = (num < 0) ? 0 : (num > 65535) ? 65535 : static_cast<uint16_t>(num);
                } else {
                    ensureVariableExists(source2);
                    val2 = getVariableValue(source2);
                }
                
                // Perform subtraction with underflow protection
                uint16_t finalValue;
                if (val1 >= val2) {
                    finalValue = val1 - val2;
                } else {
                    finalValue = 0;  // Prevent underflow
                }
                
                uint16_t oldValue = getVariableValue(targetVar);
                setVariableValue(targetVar, finalValue);
               
                // Output debug information (from teammates' implementation)
                std::cout << "debug: " << targetVar << " = " << val1 << " - " << val2 
                         << " = " << finalValue << std::endl;
                
                if (val1 < val2) {
                    std::cout << "Note: Result capped at 0 due to uint16_t underflow prevention." << std::endl;
                }
                
                currentScreen->simulateProgress();
                std::cout << "Command completed. Progress updated." << std::endl;
                currentScreen->display();
            }
            else {
                std::cout << "Wrong args for SUBTRACT. Must be SUBTRACT(target, source1, source2)." << std::endl;
            }
        }
        else {
            std::cout << "SUBTRACT arg cannot be empty." << std::endl;
        }
    }
    else if (command.find("SLEEP(") != std::string::npos) {
        std::string value = extractCommandValue(command, "SLEEP");
        if (!value.empty()) {
            int ticks = std::stoi(value);
            if (ticks < 0) ticks = 0;
            processManager->sleepCurrentProcess(ticks);
        }
    }

    else if (command.find("FOR(") != std::string::npos) {
        std::string block = extractCommandValue(command, "FOR");
        size_t commaPos = block.find(",");
        if (commaPos != std::string::npos) {
            std::string instructions = block.substr(0, commaPos);
            int repeats = std::stoi(block.substr(commaPos + 1));
            if (repeats <= 0) repeats = 1;

            for (int i = 0; i < repeats; ++i) {
                if (currentScreen->getLoopDepth() >= 3) {
                    std::cout << "Error: Maximum loop nesting exceeded.\n";
                    break;
                }
                currentScreen->enterLoop();
                for (const auto& instr : split(instructions, ';')) {
                    processScreenCommand(instr);
                }
                currentScreen->exitLoop();
            }
        }
    }
    else {
        std::cout << "Executing command in screen '" << currentScreen->getName() << "': " << command << std::endl;
        currentScreen->simulateProgress();
        std::cout << "Command completed. Progress updated." << std::endl;
        currentScreen->display();
    }
}

void ConsoleManager::processCommand(const std::string& command) {
    if (inMainMenu) {
        processMainMenuCommand(command);
    } else {
        processScreenCommand(command);
    }
}

void ConsoleManager::showPrompt() {
    if (inMainMenu) {
        std::cout << "\033[33mType 'exit' to quit, 'clear' to clear screen, 'help' for commands\033[0m\n";
        std::cout << "Enter a command: ";
    } else {
        std::cout << "\n\033[33m[Screen: " << currentScreen->getName() << "] Enter command (or 'exit' to return): \033[0m";
    }
}

void ConsoleManager::run() {
    std::string command;
    clearScreen();
    printHeader();
    while (true) {
        showPrompt();
        std::getline(std::cin, command);
        processCommand(command);
    }
}

void ConsoleManager::printGPUInfo(const GPUInfo& gpu) {
    // First line: GPU ID, Name, Persistence, Bus-ID, Display, ECC
    std::cout << "|" << std::setw(4) << gpu.id << "  ";
    std::cout << std::left << std::setw(26) << gpu.name.substr(0, 23);
    std::cout << std::right << std::setw(5) << gpu.persistence << "    |   ";
    std::cout << std::left << std::setw(15) << gpu.busId << " ";
    std::cout << std::right << std::setw(3) << gpu.display << " |";
    std::cout << std::setw(21) << gpu.ecc << " |\n";
    
    // Second line: Fan, Temp, Perf, Power Usage/Cap, Memory Usage, GPU Util, Compute Mode
    std::cout << "|" << std::setw(3) << gpu.fanPercent << "%";
    std::cout << std::setw(5) << gpu.tempC << "C";
    std::cout << std::setw(6) << gpu.perf;
    std::cout << std::setw(25) << (std::to_string(gpu.powerUsage) + "W / " + std::to_string(gpu.powerCap) + "W");
    std::cout << "|";
    std::cout << std::setw(13) << (std::to_string(gpu.memoryUsed) + "MiB");
    std::cout << " /" << std::setw(7) << (std::to_string(gpu.memoryTotal) + "MiB");
    std::cout << " |";
    std::cout << std::setw(10) << (std::to_string(gpu.gpuUtil) + "%");
    std::cout << std::setw(11) << gpu.computeMode << " |\n";
    
    // Third line: Empty fields with MIG info
    std::cout << "|" << std::setw(41) << " ";
    std::cout << "|" << std::setw(24) << " ";
    std::cout << "|" << std::setw(21) << gpu.mig << " |\n";
}

void ConsoleManager::printProcessInfo(const ProcessInfo& process) {
    // Truncate process name if too long to maintain table alignment
    std::string truncatedName = process.processName;
    if (truncatedName.length() > 38) {
        truncatedName = truncatedName.substr(0, 35) + "...";
    }
    
    std::cout << "|" << std::setw(5) << process.gpu;
    std::cout << std::setw(6) << process.gi;
    std::cout << std::setw(5) << process.ci;
    std::cout << std::setw(10) << process.pid;
    std::cout << std::setw(8) << process.type;
    std::cout << "   " << std::left << std::setw(38) << truncatedName;
    std::cout << std::right << std::setw(13) << (std::to_string(process.memoryUsage) + "MiB");
    std::cout << " |\n";
}

std::vector<GPUInfo> ConsoleManager::getDummyGPUData() {
    std::vector<GPUInfo> gpus;
    
    GPUInfo gpu;
    gpu.id = 0;
    gpu.name = "NVIDIA GeForce RTX 4080";
    gpu.persistence = "Off";
    gpu.busId = "00000000:01:00.0";
    gpu.display = "On";
    gpu.ecc = "N/A";
    gpu.fanPercent = 30;
    gpu.tempC = 45;
    gpu.perf = "P2";
    gpu.powerUsage = 85;
    gpu.powerCap = 320;
    gpu.memoryUsed = 3547;
    gpu.memoryTotal = 16376;
    gpu.gpuUtil = 12;
    gpu.computeMode = "Default";
    gpu.mig = "N/A";
    
    gpus.push_back(gpu);
    return gpus;
}

std::vector<ProcessInfo> ConsoleManager::getRealProcessData() {
    std::vector<ProcessInfo> processes;
    
    if (!processManager) {
        return processes;
    }
    
    auto allProcesses = processManager->getAllProcesses();
    for (const auto& process : allProcesses) {
        ProcessInfo info;
        info.gpu = 0; // CPU processes, not GPU
        info.gi = "N/A";
        info.ci = "N/A";
        info.pid = process->getProcessId();
        info.type = "C"; // CPU process
        info.processName = process->getName();
        info.memoryUsage = 64; // Simulated memory usage
        
        processes.push_back(info);
    }
    
    return processes;
}

std::vector<ProcessInfo> ConsoleManager::getDummyProcessData() {
    std::vector<ProcessInfo> processes;
    
    processes.push_back({0, "N/A", "N/A", 1234, "G", "/System/Applications/Activity Monitor.app", 256});
    processes.push_back({0, "N/A", "N/A", 2468, "C", "python3", 512});
    processes.push_back({0, "N/A", "N/A", 3692, "G", "/Applications/Google Chrome.app", 1024});
    processes.push_back({0, "N/A", "N/A", 4816, "C", "./training_model", 1536});
    processes.push_back({0, "N/A", "N/A", 5940, "G", "/Applications/Blender.app", 219});
    
    return processes;
}

// Helper functions for ADD/SUBTRACT operations (from teammates' implementation)
void ConsoleManager::ensureVariableExists(const std::string& varName) {
    if (declaredVariables.find(varName) == declaredVariables.end()) {
        // Auto-declare with random value as per teammates' implementation
        srand(static_cast<unsigned int>(time(nullptr)));
        uint16_t randomValue = static_cast<uint16_t>(rand() % 65536);
        declaredVariables[varName] = randomValue;
    }
}

uint16_t ConsoleManager::getVariableValue(const std::string& varName) {
    auto it = declaredVariables.find(varName);
    if (it != declaredVariables.end()) {
        return it->second;
    } else {
        return 0;
    }
}

void ConsoleManager::setVariableValue(const std::string& varName, uint16_t value) {
    declaredVariables[varName] = value;
}

// Process argument parsing for ADD/SUBTRACT (from teammates' implementation)
std::vector<std::string> ConsoleManager::parseCommaSeparatedArgs(const std::string& argString) {
    std::vector<std::string> args;
    size_t start = 0;
    size_t end = argString.find(',');
    
    // Parse first two arguments
    while (end != std::string::npos && args.size() < 2) {
        std::string arg = argString.substr(start, end - start);
        arg.erase(0, arg.find_first_not_of(" \t\n\r\f\v"));
        arg.erase(arg.find_last_not_of(" \t\n\r\f\v") + 1);
        args.push_back(arg);
        start = end + 1;
        end = argString.find(',', start);
    }
    
    // Parse last argument
    if (start < argString.length()) {
        std::string lastArg = argString.substr(start);
        lastArg.erase(0, lastArg.find_first_not_of(" \t\n\r\f\v"));
        lastArg.erase(lastArg.find_last_not_of(" \t\n\r\f\v") + 1);
        args.push_back(lastArg);
    }

    return args;
}
