#include "Scheduler.h"
#include "Process.h"
#include "ProcessManager.h"
#include <chrono>
#include <iomanip>
#include <sstream>

Scheduler::Scheduler(ProcessManager* pm) : running(false), activeProcesses(0), processManager(pm) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    running = true;
    for (int i = 0; i < NUM_CORES; ++i) {
        cpuThreads.emplace_back(&Scheduler::cpuWorker, this, i);
    }
}

void Scheduler::stop() {
    running = false;
    cv.notify_all();
    for (auto& thread : cpuThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cpuThreads.clear();
}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process);
        activeProcesses++;
    }
    cv.notify_one();
}

bool Scheduler::isProcessing() const {
    return activeProcesses > 0;
}

void Scheduler::cpuWorker(int coreId) {
    while (running) {
        std::shared_ptr<Process> process = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !running || !readyQueue.empty(); });
            
            if (!running && readyQueue.empty()) {
                return;
            }
            
            if (!readyQueue.empty()) {
                process = readyQueue.front();
                readyQueue.pop();
            }
        }
        
        if (process) {
            executeProcess(process, coreId);
        }
    }
}

void Scheduler::executeProcess(std::shared_ptr<Process> process, int coreId) {
    if (processManager) {
        processManager->updateProcessCore(process->getProcessId(), coreId);
    }
    
    while (process->getCurrentLine() <= process->getTotalLines() && process->getIsActive()) {
        process->incrementLine();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
    }
    
    activeProcesses--;
}
