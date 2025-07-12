#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <set>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>

class Process;

// Memory block structure for memory allocation
struct MemoryBlock {
    int startAddress;
    int size;
    bool isFree;
    std::string processName;
    int processId;

    MemoryBlock(int start, int blockSize)
        : startAddress(start), size(blockSize), isFree(true), processId(-1) {}
};

// Define operator== for MemoryBlock for std::find to work
inline bool operator==(const MemoryBlock& a, const MemoryBlock& b) {
    return a.startAddress == b.startAddress && a.size == b.size;
}

class MemoryManager {
private:
    std::vector<MemoryBlock> memoryBlocks;
    int totalMemory;        // Total available memory (16384 bytes)
    int memoryPerProcess;   // Fixed memory per process (4096 bytes)
    int memoryPerFrame;     // Memory per frame (16 bytes)
    std::string fitPolicy;  // F for First-fit
    mutable std::mutex memoryMutex; // For thread-safe operations
    int currentQuantum;     // Current quantum cycle

public:
    MemoryManager(int totalMem = 16384, int memPerProc = 4096, int memPerFrame = 16, const std::string& policy = "F");
    ~MemoryManager();

    // Initialize memory with a single free block
    void initialize();

    // First-fit memory allocation
    bool allocateMemory(std::shared_ptr<Process> process);

    // Memory deallocation
    void deallocateMemory(int processId);

    // Check if memory is available for a process
    bool hasAvailableMemory(int size) const;

    // Calculate external fragmentation
    int calculateExternalFragmentation() const;

    // Generate memory snapshot for the current quantum cycle
    void generateMemorySnapshot(int quantumCycle);

    // Get number of processes in memory
    int getProcessesInMemory() const;

    // Set the current quantum cycle
    void setCurrentQuantum(int quantum);

    // ASCII representation of memory for visualization
    std::string getMemoryASCIIRepresentation() const;

    // Get memory map for a specific process
    std::pair<int, int> getProcessMemoryMap(int processId) const;

private:
    // Generate ASCII printout for the memory snapshot
    std::string generateASCIIPrintout() const;
    
    // Generate formatted timestamp
    std::string getCurrentTimestamp() const;
};

#endif
