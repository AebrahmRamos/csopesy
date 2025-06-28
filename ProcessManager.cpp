#include "ProcessManager.h"
#include "ProcessGenerator.h"
#include "ConsoleManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>

ProcessManager::ProcessManager() : numCores(4), storedConfigPtr(nullptr) {
    scheduler = std::make_unique<Scheduler>(this);
    generator = std::make_unique<ProcessGenerator>(this);
}

ProcessManager::~ProcessManager() {
    if (generator) {
        generator->stopGeneration();
    }
    if (scheduler) {
        scheduler->stop();
    }
}

void ProcessManager::setConfig(const Config& config) {
    numCores = config.numCpu;
    storedConfigPtr = const_cast<Config*>(&config);
}

void ProcessManager::startProcessGeneration() {
    if (generator && storedConfigPtr) {
        generator->startGeneration(storedConfigPtr->batchProcessFreq, 
                                 storedConfigPtr->minIns, 
                                 storedConfigPtr->maxIns);
    } else if (generator) {
        // Fallback valuenincase config not set or error
        generator->startGeneration(1, 1000, 2000);
    }
}

void ProcessManager::stopProcessGeneration() {
    if (generator) {
        generator->stopGeneration();
    }
}

bool ProcessManager::isGeneratingProcesses() const {
    return generator ? generator->isGenerating() : false;
}

void ProcessManager::addGeneratedProcess(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(processMutex);
    processes.push_back(process);
    
    // Add to scheduler
    if (scheduler) {
        scheduler->addProcess(process);
    }
}

void ProcessManager::initialize() {
    // Removed since it was old and hardcoded and new processes will be using scheduler-start now
    // New processes will be created via scheduler-start command
}

void ProcessManager::startScheduler() {
    scheduler->start();
    
    for (auto& process : processes) {
        scheduler->addProcess(process);
    }
}

void ProcessManager::stopScheduler() {
    if (scheduler) {
        scheduler->stop();
    }
}

void ProcessManager::showProcessStatus() {
    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << "Running processes:" << std::endl;
    
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            std::cout << std::left << std::setw(12) << process->getName() << " ";
            std::cout << "(" << process->getCreationDate() << ")";
            
            int coreId = getProcessCore(process->getProcessId());
            if (coreId >= 0) {
                std::cout << "     Core: " << std::setw(2) << coreId << "    ";
            } else {
                std::cout << "     Core: --    ";
            }
            
            std::cout << std::setw(5) << process->getCurrentLine() << " / " << process->getTotalLines();
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nFinished processes:" << std::endl;
    for (const auto& process : processes) {
        if (!process->getIsActive()) {
            std::cout << std::left << std::setw(12) << process->getName() << " ";
            std::cout << "(" << process->getCreationDate() << ")";
            std::cout << "     Finished    ";
            std::cout << std::setw(5) << process->getTotalLines() << " / " << process->getTotalLines();
            std::cout << std::endl;
        }
    }
    std::cout << "-----------------------------------------" << std::endl;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getRunningProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::vector<std::shared_ptr<Process>> running;
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            running.push_back(process);
        }
    }
    return running;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getFinishedProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::vector<std::shared_ptr<Process>> finished;
    for (const auto& process : processes) {
        if (!process->getIsActive()) {
            finished.push_back(process);
        }
    }
    return finished;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getAllProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    return processes;
}

std::shared_ptr<Process> ProcessManager::findProcessByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(processMutex);
    for (const auto& process : processes) {
        if (process->getName() == name) {
            return process;
        }
    }
    return nullptr;
}

bool ProcessManager::hasActiveProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            return true;
        }
    }
    return false;
}

int ProcessManager::getNumCores() const {
    return numCores;
}

int ProcessManager::getUsedCores() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::set<int> usedCores;
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            int core = getProcessCore(process->getProcessId());
            if (core >= 0) {
                usedCores.insert(core);
            }
        }
    }
    return usedCores.size();
}

double ProcessManager::getCpuUtilization() const {
    if (numCores == 0) return 0.0;
    return (static_cast<double>(getUsedCores()) / numCores) * 100.0;
}

void ProcessManager::updateProcessCore(int processId, int coreId) {
    processCoreMap[processId] = coreId;
}

int ProcessManager::getProcessCore(int processId) const {
    auto it = processCoreMap.find(processId);
    return (it != processCoreMap.end()) ? it->second : -1;
}
