#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

// Forward declaration to avoid circular dependency
class Process;
class ProcessManager;

enum class SchedulerType {
    FCFS,  // First Come First Served
    RR     // Round Robin
};

class Scheduler {
private:
    static constexpr int NUM_CORES = 4;
    std::queue<std::shared_ptr<Process>> readyQueue;
    std::vector<std::thread> cpuThreads;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::atomic<int> activeProcesses;
    ProcessManager* processManager;
    
    // Scheduling configuration
    SchedulerType schedulerType;
    int quantumCycles;
    int numCores;

public:
    Scheduler(ProcessManager* pm);
    ~Scheduler();
    
    void start();
    void stop();
    void addProcess(std::shared_ptr<Process> process);
    bool isProcessing() const;
    
    // Configuration methods
    void setSchedulerConfig(const std::string& algorithm, int quantum, int cores);
    
private:
    void cpuWorker(int coreId);
    void executeProcess(std::shared_ptr<Process> process, int coreId);
    void executeProcessFCFS(std::shared_ptr<Process> process, int coreId);
    void executeProcessRR(std::shared_ptr<Process> process, int coreId);
    void executeInstruction(std::shared_ptr<Process> process, const std::string& instruction);
    void executeArithmeticInstruction(std::shared_ptr<Process> process, const std::string& instruction, const std::string& operation);
    uint16_t getValueFromArgument(std::shared_ptr<Process> process, const std::string& arg);
    
    // Helper methods
    SchedulerType parseSchedulerType(const std::string& algorithm);
    void requeueProcess(std::shared_ptr<Process> process);
};

#endif
