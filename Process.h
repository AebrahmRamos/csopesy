#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <chrono>
#include <vector>
#include <map>

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
    
    // Instruction execution support
    std::vector<std::string> instructions;
    int currentInstructionIndex;
    bool isExecutingAutomatically;
    std::vector<std::string> executionLog;
    std::map<std::string, uint16_t> variables;

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
    
    // Instruction management
    void setInstructions(const std::vector<std::string>& instructionList);
    std::string getCurrentInstruction() const;
    bool hasMoreInstructions() const;
    void advanceInstruction();
    void addToExecutionLog(const std::string& instruction);
    
    // Variable management
    void setVariable(const std::string& name, uint16_t value);
    uint16_t getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;
    void ensureVariableExists(const std::string& name);
    std::map<std::string, uint16_t> getAllVariables() const;
    
    // Execution state
    bool isAutoExecuting() const;
    void setAutoExecuting(bool autoExec);
    int getCurrentInstructionIndex() const;
    std::vector<std::string> getExecutionLog() const;
};

#endif
