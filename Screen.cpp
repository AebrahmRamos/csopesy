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
    
    std::cout << "\nProcess Name: " << attachedProcess->getName() << std::endl;
    std::cout << "Process ID:   " << attachedProcess->getProcessId() << std::endl;
    
    if (!attachedProcess->getIsActive()) {
        std::cout << "Status:       Finished!" << std::endl;
    } else {
        std::cout << "Status:       Running" << std::endl;
    }
    
    std::cout << "\n--- Instruction Log ---" << std::endl;
    
    if (attachedProcess->isAutoExecuting()) {
        // Display instruction execution in the approved format
        auto executionLog = attachedProcess->getExecutionLog();
        int currentIndex = attachedProcess->getCurrentInstructionIndex();
        int totalInstructions = attachedProcess->getTotalLines();
        
        // Show executed instructions
        for (int i = 0; i < static_cast<int>(executionLog.size()); i++) {
            std::cout << "[" << std::setfill('0') << std::setw(3) << (i + 1) 
                      << "/" << std::setfill('0') << std::setw(3) << totalInstructions 
                      << "] EXECUTED: " << executionLog[i] << std::endl;
        }
        
        // Show current instruction
        if (attachedProcess->hasMoreInstructions()) {
            std::string currentInstruction = attachedProcess->getCurrentInstruction();
            std::cout << "[" << std::setfill('0') << std::setw(3) << (currentIndex + 1)
                      << "/" << std::setfill('0') << std::setw(3) << totalInstructions
                      << "] > CURRENT: " << currentInstruction << std::endl;
        }
        
        // Show a few pending instructions (if any)
        int pendingCount = std::min(3, totalInstructions - currentIndex - 1);
        for (int i = 1; i <= pendingCount; i++) {
            int pendingIndex = currentIndex + i;
            if (pendingIndex < totalInstructions) {
                // We don't have direct access to pending instructions, so show placeholder
                std::cout << "[" << std::setfill('0') << std::setw(3) << (pendingIndex + 1)
                          << "/" << std::setfill('0') << std::setw(3) << totalInstructions
                          << "] PENDING:  [Next instruction]" << std::endl;
            }
        }
        
        // Show variables
        std::cout << "\n--- Variables ---" << std::endl;
        auto variables = attachedProcess->getAllVariables();
        if (variables.empty()) {
            std::cout << "No variables declared yet." << std::endl;
        } else {
            for (const auto& var : variables) {
                std::cout << var.first << ": " << var.second << std::endl;
            }
        }
    } else {
        // For manually created processes (legacy behavior)
        std::cout << "Manual process - no auto-execution log available." << std::endl;
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
    loopIter++;
}

void Screen::exitLoop() {
    loopIter--;
}

int Screen::getLoopDepth() const {
    return loopIter;
}

std::string Screen::getName() const { return processName; }
int Screen::getCurrentLine() const { return currentLine; }
int Screen::getTotalLines() const { return totalLines; }
std::string Screen::getCreationDate() const { return creationDate; }
bool Screen::getIsActive() const { return isActive; }
void Screen::setActive(bool active) { isActive = active; }
int Screen::getArrivalTime() const { return arrivalTime; }
