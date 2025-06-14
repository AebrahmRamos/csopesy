#include "Scheduler.h"
#include "Process.h"
#include "ProcessManager.h"
#include <fstream>
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

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%m/%d/%Y %H:%M:%S");
    return ss.str();
}

void Scheduler::executeProcess(std::shared_ptr<Process> process, int coreId) {
    // Update process manager with core assignment
    if (processManager) {
        processManager->updateProcessCore(process->getProcessId(), coreId);
    }
    
    std::string outputFileName = process->getName() + "_output.txt";
    std::ofstream outputFile(outputFileName, std::ios::app);
    
    while (process->getCurrentLine() <= process->getTotalLines() && process->getIsActive()) {
        std::string timestamp = getCurrentTimestamp();
        outputFile << "(" << timestamp << ") Core " << coreId 
                  << ": Executing print command " << process->getCurrentLine() 
                  << " of process " << process->getName() << std::endl;
                  
        process->incrementLine();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
    }
    
    activeProcesses--;
    outputFile.close();
}
