#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include "Process.h"
#include "Scheduler.h"

class ProcessGenerator;
struct Config; // Forward declaration

class ProcessManager {
private:
    std::vector<std::shared_ptr<Process>> processes;
    std::unique_ptr<Scheduler> scheduler;
    std::unique_ptr<ProcessGenerator> generator;
    std::map<int, int> processCoreMap;          // maps processId:coreId pairs
    mutable std::mutex processMutex;            // For thread-safe process operations
    int numCores;
    Config* storedConfigPtr; // Store pointer to config

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
    bool isSchedulerRunning() const;  // Check if scheduler is actively running

    void sleepCurrentProcess(int ticks);
};

#endif
