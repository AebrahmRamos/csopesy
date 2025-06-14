#include "ProcessManager.h"
#include <iostream>
#include <iomanip>

ProcessManager::ProcessManager() {
    scheduler = std::make_unique<Scheduler>(this);
}

ProcessManager::~ProcessManager() {
    if (scheduler) {
        scheduler->stop();
    }
}

void ProcessManager::initialize() {
    // Create 10 processes with 100 print commands each
    for (int i = 1; i <= 10; ++i) {
        std::string name = "process" + std::to_string(i);
        auto process = std::make_shared<Process>(name, i, 100);
        processes.push_back(process);
    }
}

void ProcessManager::startScheduler() {
    scheduler->start();
    
    // Add all processes to scheduler
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
    std::vector<std::shared_ptr<Process>> running;
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            running.push_back(process);
        }
    }
    return running;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getFinishedProcesses() const {
    std::vector<std::shared_ptr<Process>> finished;
    for (const auto& process : processes) {
        if (!process->getIsActive()) {
            finished.push_back(process);
        }
    }
    return finished;
}

bool ProcessManager::hasActiveProcesses() const {
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            return true;
        }
    }
    return false;
}

void ProcessManager::updateProcessCore(int processId, int coreId) {
    processCoreMap[processId] = coreId;
}

int ProcessManager::getProcessCore(int processId) const {
    auto it = processCoreMap.find(processId);
    return (it != processCoreMap.end()) ? it->second : -1;
}
