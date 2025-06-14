#include "Process.h"
#include <ctime>
#include <iomanip>
#include <sstream>

Process::Process(const std::string& name, int id, int totalCommands) 
    : processName(name), processId(id), currentLine(1), totalLines(totalCommands), 
      isActive(true), assignedCore(-1), startTime(std::chrono::steady_clock::now()) {
    
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", now);
    creationDate = std::string(timeBuffer);
}

std::string Process::getName() const { 
    return processName; 
}

int Process::getCurrentLine() const { 
    return currentLine; 
}

int Process::getTotalLines() const { 
    return totalLines; 
}

std::string Process::getCreationDate() const { 
    return creationDate; 
}

bool Process::getIsActive() const { 
    return isActive; 
}

int Process::getProcessId() const { 
    return processId; 
}

int Process::getAssignedCore() const { 
    return assignedCore; 
}

void Process::setActive(bool active) { 
    isActive = active; 
}

void Process::setAssignedCore(int core) { 
    assignedCore = core; 
}

void Process::incrementLine() {
    if (currentLine < totalLines) {
        currentLine++;
    }
    if (currentLine >= totalLines) {
        isActive = false;
    }
}
