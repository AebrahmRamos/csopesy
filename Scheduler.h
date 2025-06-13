#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>
#include "Screen.h"

class Scheduler {
private:
    static constexpr int NUM_CORES = 4;
    std::queue<std::shared_ptr<Screen>> readyQueue;
    std::vector<std::thread> cpuThreads;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::atomic<int> activeProcesses;

public:
    Scheduler();
    ~Scheduler();
    
    void start();
    void stop();
    void addProcess(std::shared_ptr<Screen> process);
    bool isProcessing() const;
    
private:
    void cpuWorker(int coreId);
    void executeProcess(std::shared_ptr<Screen> process, int coreId);
};

#endif
