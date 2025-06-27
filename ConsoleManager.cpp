#include "ConsoleManager.h"
#include <iostream>
#include <regex>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <vector>

ConsoleManager::ConsoleManager() : currentScreen(nullptr), inMainMenu(true) {
    processManager = std::make_unique<ProcessManager>();
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
        std::cout << "  screen -ls     - List all screen sessions\n";
        std::cout << "  scheduler-test - Run scheduler test\n";
        std::cout << "  scheduler-stop - Stop scheduler\n";
        std::cout << "  report-util    - Generate report\n";
        std::cout << "  clear          - Clear the screen\n";
        std::cout << "  help           - Show this help menu\n";
        std::cout << "  exit           - Exit the application\n";
        std::cout << "  nvidia-smi     - Shows GPU summary and running processes\n";
    } else {
        std::cout << "\n\033[32m=== Screen Session Help ===\033[0m\n";
        std::cout << "  exit - Return to main menu\n";
        std::cout << "  help - Show this help menu\n";
        std::cout << "  Any other command will simulate process execution\n";
    }
}

void ConsoleManager::commandInitialize() {
    std::cout << "initialize command recognized. System initialized." << std::endl;
}

void ConsoleManager::commandSchedulerTest() {
    if (processManager) {
        processManager->showProcessStatus();
    }
}

void ConsoleManager::commandSchedulerStop() {
    std::cout << "scheduler-stop command recognized. Scheduler stopped." << std::endl;
}

void ConsoleManager::commandReportUtil() {
    std::cout << "report-util command recognized. Generating report." << std::endl;
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
    
    // Get dummy process data and print it
    std::vector<ProcessInfo> processes = getDummyProcessData();
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
    if (screens.empty()) {
        std::cout << "No screen sessions found." << std::endl;
        return;
    }
    std::cout << "\n\033[32m=== Active Screen Sessions ===\033[0m\n";
    for (const auto& pair : screens) {
        std::cout << "  • " << pair.first << " (Created: " << pair.second->getCreationDate() << ")" << std::endl;
    }
    std::cout << std::endl;
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
        if (it != screens.end()) {
            resumeScreen(processName);
        } else {
            std::cout << "Screen '" << processName << "' not found. Use 'screen -s " << processName << "' to create." << std::endl;
        }
    }
}

void ConsoleManager::processMainMenuCommand(const std::string& command) {
    if (command == "initialize") {
        commandInitialize();
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
    else if (command == "nvidia-smi") {
        commandNvidiaSmi();
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
    else if (command == "screen -ls") {
        listScreens();
    }
    else if (std::regex_match(command, std::regex(R"(screen\s+-[rs]\s+\S+)"))) {
        handleScreenCommand(command);
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

std::vector<ProcessInfo> ConsoleManager::getDummyProcessData() {
    std::vector<ProcessInfo> processes;
    
    processes.push_back({0, "N/A", "N/A", 1234, "G", "/System/Applications/Activity Monitor.app", 256});
    processes.push_back({0, "N/A", "N/A", 2468, "C", "python3", 512});
    processes.push_back({0, "N/A", "N/A", 3692, "G", "/Applications/Google Chrome.app", 1024});
    processes.push_back({0, "N/A", "N/A", 4816, "C", "./training_model", 1536});
    processes.push_back({0, "N/A", "N/A", 5940, "G", "/Applications/Blender.app", 219});
    
    return processes;
}
