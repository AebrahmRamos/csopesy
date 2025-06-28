#include "Screen.h"
#include "Process.h"
#include <ctime>
#include <iomanip>

Screen::Screen(const std::string& name, int totalCommands) : 
    processName(name), 
    currentLine(1), 
    totalLines(totalCommands), 
    isActive(true),
    arrivalTime(static_cast<int>(time(nullptr))),
    attachedProcess(nullptr) {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", now);
    creationDate = std::string(timeBuffer);
}

void Screen::display() {
    std::cout << "\n==============================" << std::endl;
    std::cout << "Process Name: " << processName << std::endl;
    
    if (attachedProcess) {
        std::cout << "Current line of instruction: " << attachedProcess->getCurrentLine() << " / " << attachedProcess->getTotalLines() << std::endl;
        std::cout << "Created on: " << attachedProcess->getCreationDate() << std::endl;
        
        if (!attachedProcess->getIsActive()) {
            std::cout << "\033[32mStatus: Finished!\033[0m" << std::endl;
        } else {
            std::cout << "Status: Running" << std::endl;
        }
    } else {
        std::cout << "Current line of instruction: " << currentLine << " / " << totalLines << std::endl;
        std::cout << "Created on: " << creationDate << std::endl;
        std::cout << "Status: No attached process" << std::endl;
    }
    
    std::cout << "==============================" << std::endl;
    std::cout << "\033[33m(Type 'process-smi' for process info, 'exit' to return to main menu)\033[0m" << std::endl;
}

void Screen::showProcessInfo() {
    if (!attachedProcess) {
        std::cout << "No process attached to this screen." << std::endl;
        return;
    }
    
    std::cout << "\nProcess name: " << attachedProcess->getName() << std::endl;
    std::cout << "ID: " << attachedProcess->getProcessId() << std::endl;
    
    // Show logs - simplified for now since full instruction system will be implemented by teammates
    std::cout << "Logs:" << std::endl;
    
    // Show a few sample log entries to match the mockup format
    int currentLine = attachedProcess->getCurrentLine();
    int startLog = std::max(1, currentLine - 5); // Show last 5 entries
    
    for (int i = startLog; i < currentLine && i <= attachedProcess->getTotalLines(); i++) {
        // Get current timestamp
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        char timeBuffer[100];
        strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %H:%M:%S", now);
        
        int coreId = attachedProcess->getAssignedCore();
        if (coreId < 0) coreId = 0;
        
        std::cout << "(" << timeBuffer << ") Core:" << coreId 
                  << " \"Hello world from " << attachedProcess->getName() << "!\"" << std::endl;
    }
    
    std::cout << "\nCurrent instruction line: " << attachedProcess->getCurrentLine() << std::endl;
    std::cout << "Lines of code: " << attachedProcess->getTotalLines() << std::endl;
    
    if (!attachedProcess->getIsActive()) {
        std::cout << "\nFinished!" << std::endl;
    }
    
    std::cout << std::endl;
}

void Screen::attachToProcess(std::shared_ptr<Process> process) {
    attachedProcess = process;
}

std::shared_ptr<Process> Screen::getAttachedProcess() const {
    return attachedProcess;
}

bool Screen::hasAttachedProcess() const {
    return attachedProcess != nullptr;
}

void Screen::simulateProgress() {
    if (currentLine < totalLines) {
        currentLine++;
    }
}

void Screen::enterLoop() {
    loopDepth++;
}

void Screen::exitLoop() {
    loopDepth--;
}

int Screen::loopDepth() const {
    return loopDepth;
}

std::string Screen::getName() const { return processName; }
int Screen::getCurrentLine() const { return currentLine; }
int Screen::getTotalLines() const { return totalLines; }
std::string Screen::getCreationDate() const { return creationDate; }
bool Screen::getIsActive() const { return isActive; }
void Screen::setActive(bool active) { isActive = active; }
int Screen::getArrivalTime() const { return arrivalTime; }
