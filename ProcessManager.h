#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include "Process.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include "VirtualMemoryManager.h"

class ProcessGenerator;
struct Config; // Forward declaration

class ProcessManager {
private:
    std::vector<std::shared_ptr<Process>> processes;
    std::unique_ptr<Scheduler> scheduler;
    std::unique_ptr<ProcessGenerator> generator;
    std::unique_ptr<MemoryManager> memoryManager;
    std::unique_ptr<VirtualMemoryManager> vmManager;
    std::map<int, int> processCoreMap;          // maps processId:coreId pairs
    mutable std::mutex processMutex;            // For thread-safe process operations
    int numCores;
    std::unique_ptr<Config> storedConfig; // Store copy of config
    int currentQuantumCycle; // Current quantum cycle counter
    bool useVirtualMemory; // Phase 2 feature flag

public:
    ProcessManager();
    ~ProcessManager();
    
    void initialize();
    void startScheduler();
    void stopScheduler();
    void showProcessStatus();
    
    // For the process generation and basing on the config 
    void setConfig(const Config& config);
    void startProcessGeneration();
    void stopProcessGeneration();
    bool isGeneratingProcesses() const;
    void addGeneratedProcess(std::shared_ptr<Process> process);
    
    // Memory management
    bool allocateMemoryToProcess(std::shared_ptr<Process> process);
    void releaseProcessMemory(std::shared_ptr<Process> process);
    void generateMemorySnapshot();
    void incrementQuantumCycle();
    int getCurrentQuantumCycle() const;
    
    // Process accessing like screen and report util
    std::vector<std::shared_ptr<Process>> getRunningProcesses() const;
    std::vector<std::shared_ptr<Process>> getFinishedProcesses() const;
    std::vector<std::shared_ptr<Process>> getAllProcesses() const;
    bool hasActiveProcesses() const;
    std::shared_ptr<Process> findProcessByName(const std::string& name) const;
    
    // Core management
    void updateProcessCore(int processId, int coreId);
    int getProcessCore(int processId) const;
    int getNumCores() const;
    int getUsedCores() const;
    double getCpuUtilization() const;

    void sleepCurrentProcess(int ticks);
    
    // Virtual memory management methods (Phase 2)
    void enableVirtualMemory(bool enable);
    bool isVirtualMemoryEnabled() const;
    std::shared_ptr<Process> createProcess(const std::string& name);
    std::shared_ptr<Process> createProcessWithMemory(const std::string& name, size_t memorySize, const std::vector<std::string>& instructions = {});
    uint16_t readProcessMemory(int processId, uint32_t virtualAddr);
    void writeProcessMemory(int processId, uint32_t virtualAddr, uint16_t value);
    
    // Enhanced statistics for Phase 2
    struct DetailedStats {
        size_t totalMemory;
        size_t usedMemory;
        size_t freeMemory;
        uint64_t totalCpuTicks;
        uint64_t idleCpuTicks;
        uint64_t activeCpuTicks;
        uint64_t pagesIn;
        uint64_t pagesOut;
        uint64_t pageFaults;
        double cpuUtilization;
        int runningProcessCount;
        int totalProcessCount;
    };
    
    DetailedStats getDetailedStats() const;
};

#endif
