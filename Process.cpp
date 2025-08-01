#include "Process.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

Process::Process(const std::string& name, int id, int totalCommands) 
    : processName(name), processId(id), currentLine(1), totalLines(totalCommands), 
      isActive(true), assignedCore(-1), startTime(std::chrono::steady_clock::now()),
      hasMemoryAllocated(false), memorySize(0), memoryStartAddress(-1), memoryEndAddress(-1),
      virtualMemorySize(0), virtualBaseAddress(0),
      currentInstructionIndex(0), isExecutingAutomatically(false) {
    
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

// Instruction management methods
void Process::setInstructions(const std::vector<std::string>& instructionList) {
    instructions = instructionList;
    currentInstructionIndex = 0;
    totalLines = static_cast<int>(instructions.size());
    isExecutingAutomatically = true;
}

std::string Process::getCurrentInstruction() const {
    if (currentInstructionIndex < static_cast<int>(instructions.size())) {
        return instructions[currentInstructionIndex];
    }
    return "";
}

bool Process::hasMoreInstructions() const {
    return currentInstructionIndex < static_cast<int>(instructions.size());
}

void Process::advanceInstruction() {
    if (hasMoreInstructions()) {
        currentInstructionIndex++;
        currentLine = currentInstructionIndex + 1;
        
        if (!hasMoreInstructions()) {
            isActive = false;
        }
    }
}

void Process::addToExecutionLog(const std::string& instruction) {
    executionLog.push_back(instruction);
}

// Variable management methods
void Process::setVariable(const std::string& name, uint16_t value) {
    variables[name] = value;
}

uint16_t Process::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    return 0;
}

bool Process::hasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

void Process::ensureVariableExists(const std::string& name) {
    if (!hasVariable(name)) {
        setVariable(name, 0);
    }
}

std::map<std::string, uint16_t> Process::getAllVariables() const {
    return variables;
}

// Virtual memory methods for Phase 2
void Process::setVirtualMemorySize(size_t size) {
    virtualMemorySize = size;
}

size_t Process::getVirtualMemorySize() const {
    return virtualMemorySize;
}

void Process::setVirtualBaseAddress(uint32_t address) {
    virtualBaseAddress = address;
}

uint32_t Process::getVirtualBaseAddress() const {
    return virtualBaseAddress;
}

uint16_t Process::readVirtualMemory(uint32_t virtualAddr) {
    // This will be implemented to work with VirtualMemoryManager
    // For now, placeholder that will be integrated with ProcessManager
    throw std::runtime_error("Virtual memory access not yet integrated with VirtualMemoryManager");
}

void Process::writeVirtualMemory(uint32_t virtualAddr, uint16_t value) {
    // This will be implemented to work with VirtualMemoryManager
    // For now, placeholder that will be integrated with ProcessManager
    throw std::runtime_error("Virtual memory access not yet integrated with VirtualMemoryManager");
}

bool Process::isValidVirtualAddress(uint32_t virtualAddr) const {
    return virtualAddr < virtualMemorySize;
}

// Execution state methods
bool Process::isAutoExecuting() const {
    return isExecutingAutomatically;
}

void Process::setAutoExecuting(bool autoExec) {
    isExecutingAutomatically = autoExec;
}

int Process::getCurrentInstructionIndex() const {
    return currentInstructionIndex;
}

std::vector<std::string> Process::getExecutionLog() const {
    return executionLog;
}

// Memory management getters and setters
bool Process::getHasMemoryAllocated() const { 
    return hasMemoryAllocated; 
}

void Process::setHasMemoryAllocated(bool allocated) { 
    hasMemoryAllocated = allocated; 
}

int Process::getMemorySize() const { 
    return memorySize; 
}

void Process::setMemorySize(int size) { 
    memorySize = size; 
}

void Process::setMemoryAddress(int start, int end) {
    memoryStartAddress = start;
    memoryEndAddress = end;
}

int Process::getMemoryStartAddress() const { 
    return memoryStartAddress; 
}

int Process::getMemoryEndAddress() const { 
    return memoryEndAddress; 
}
