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
    
    // Memory management attributes
    bool hasMemoryAllocated;
    int memorySize;
    int memoryStartAddress;
    int memoryEndAddress;
    
    // Virtual memory attributes for Phase 2
    size_t virtualMemorySize;
    uint32_t virtualBaseAddress;
    
    // Instruction execution support
    std::vector<std::string> instructions;
    int currentInstructionIndex;
    bool isExecutingAutomatically;
    std::vector<std::string> executionLog;
    

    static const size_t SYMBOL_TABLE_SIZE = 64;  // 64 bytes for symbol table
    static const size_t MAX_VARIABLES = 32;      
    static const uint32_t SYMBOL_TABLE_BASE_ADDR = 0x0;  
    
    std::map<std::string, uint16_t> variables;
    std::map<std::string, uint32_t> variableAddresses;  
    size_t currentVariableCount;
    uint32_t nextVariableAddress;

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
    
    // Memory management getters and setters
    bool getHasMemoryAllocated() const;
    void setHasMemoryAllocated(bool allocated);
    int getMemorySize() const;
    void setMemorySize(int size);
    void setMemoryAddress(int start, int end);
    int getMemoryStartAddress() const;
    int getMemoryEndAddress() const;
    
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
      bool setVariable(const std::string& name, uint16_t value);  // Returns false if symbol table full
    uint16_t getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;
    void ensureVariableExists(const std::string& name);
    std::map<std::string, uint16_t> getAllVariables() const;
    
    // Symbol Table specific methods (core functionality only)
    bool canDeclareMoreVariables() const;
    size_t getVariableCount() const;
    uint32_t getVariableAddress(const std::string& name) const;
    
    
    
    // Virtual memory methods for Phase 2
    void setVirtualMemorySize(size_t size);
    size_t getVirtualMemorySize() const;
    void setVirtualBaseAddress(uint32_t address);
    uint32_t getVirtualBaseAddress() const;
    
    // Memory access methods that may trigger page faults
    uint16_t readVirtualMemory(uint32_t virtualAddr);
    void writeVirtualMemory(uint32_t virtualAddr, uint16_t value);
    bool isValidVirtualAddress(uint32_t virtualAddr) const;
    bool isSymbolTableAddress(uint32_t virtualAddr) const;  // NEW
    
    // Execution state
    bool isAutoExecuting() const;
    void setAutoExecuting(bool autoExec);
    int getCurrentInstructionIndex() const;
    std::vector<std::string> getExecutionLog() const;

    private:
    uint32_t allocateVariableAddress();
    void initializeSymbolTable();
};

#endif
