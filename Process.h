#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <chrono>

class Process {
private:
    std::string processName;
    int currentLine;
    int totalLines;
    std::string creationDate;
    bool isActive;
    int processId;
    int assignedCore;
    std::chrono::steady_clock::time_point startTime;

public:
    Process(const std::string& name, int id, int totalCommands = 100);
    
    // Getters
    std::string getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCreationDate() const;
    bool getIsActive() const;
    int getProcessId() const;
    int getAssignedCore() const;
    
    // Setters
    void setActive(bool active);
    void setAssignedCore(int core);
    void incrementLine();
};

#endif
