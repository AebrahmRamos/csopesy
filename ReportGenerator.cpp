#include "ReportGenerator.h"
#include "ProcessManager.h"
#include "Process.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

std::string ReportGenerator::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%m/%d/%Y, %I:%M:%S %p");
    return ss.str();
}

std::string ReportGenerator::formatProcessInfo(const std::string& name, const std::string& date, int core, int currentLine, int totalLines, bool finished) {
    std::stringstream ss;
    ss << std::left << std::setw(12) << name << " ";
    ss << "(" << date << ")";
    
    if (finished) {
        ss << "     Finished    ";
        ss << std::setw(5) << totalLines << " / " << totalLines;
    } else {
        if (core >= 0) {
            ss << "     Core: " << std::setw(2) << core << "    ";
        } else {
            ss << "     Core: --    ";
        }
        ss << std::setw(5) << currentLine << " / " << totalLines;
    }
    
    return ss.str();
}

UtilizationData ReportGenerator::calculateUtilization(ProcessManager* pm) {
    UtilizationData data;
    data.timestamp = getCurrentTimestamp();
    
    if (!pm) {
        data.cpuUtilization = 0.0;
        data.coresUsed = 0;
        data.coresAvailable = 0;
        data.totalProcesses = 0;
        data.runningProcesses = 0;
        data.finishedProcesses = 0;
        return data;
    }
    
    data.cpuUtilization = pm->getCpuUtilization();
    data.coresUsed = pm->getUsedCores();
    data.coresAvailable = pm->getNumCores();
    
    auto runningProcs = pm->getRunningProcesses();
    auto finishedProcs = pm->getFinishedProcesses();
    
    data.runningProcesses = runningProcs.size();
    data.finishedProcesses = finishedProcs.size();
    data.totalProcesses = data.runningProcesses + data.finishedProcesses;
    
    for (const auto& proc : runningProcs) {
        int core = pm->getProcessCore(proc->getProcessId());
        std::string info = formatProcessInfo(proc->getName(), proc->getCreationDate(), 
                                           core, proc->getCurrentLine(), proc->getTotalLines(), false);
        data.runningProcessList.push_back(info);
    }
    
    for (const auto& proc : finishedProcs) {
        std::string info = formatProcessInfo(proc->getName(), proc->getCreationDate(), 
                                           -1, proc->getCurrentLine(), proc->getTotalLines(), true);
        data.finishedProcessList.push_back(info);
    }
    
    return data;
}

void ReportGenerator::generateReport(ProcessManager* pm, const std::string& filename) {
    UtilizationData data = calculateUtilization(pm);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not create report file " << filename << std::endl;
        return;
    }
    
    file << "CPU Utilization Report\n";
    file << "Timestamp: " << data.timestamp << "\n";
    file << "-----------------------------------------\n";
    file << "CPU utilization: " << std::fixed << std::setprecision(2) << data.cpuUtilization << "%\n";
    file << "Cores used: " << data.coresUsed << "\n";
    file << "Cores available: " << data.coresAvailable << "\n";
    file << "\n";
    
    file << "Running processes:\n";
    if (data.runningProcessList.empty()) {
        file << "No running processes.\n";
    } else {
        for (const auto& info : data.runningProcessList) {
            file << info << "\n";
        }
    }
    file << "\n";
    
    file << "Finished processes:\n";
    if (data.finishedProcessList.empty()) {
        file << "No finished processes.\n";
    } else {
        for (const auto& info : data.finishedProcessList) {
            file << info << "\n";
        }
    }
    file << "-----------------------------------------\n";
    
    file.close();
    std::cout << "Report saved to " << filename << std::endl;
}

void ReportGenerator::displayReport(ProcessManager* pm) {
    UtilizationData data = calculateUtilization(pm);
    
    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << "CPU utilization: " << std::fixed << std::setprecision(2) << data.cpuUtilization << "%" << std::endl;
    std::cout << "Cores used: " << data.coresUsed << std::endl;
    std::cout << "Cores available: " << data.coresAvailable << std::endl;
    std::cout << std::endl;
    
    std::cout << "Running processes:" << std::endl;
    if (data.runningProcessList.empty()) {
        std::cout << "No running processes." << std::endl;
    } else {
        for (const auto& info : data.runningProcessList) {
            std::cout << info << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "Finished processes:" << std::endl;
    if (data.finishedProcessList.empty()) {
        std::cout << "No finished processes." << std::endl;
    } else {
        for (const auto& info : data.finishedProcessList) {
            std::cout << info << std::endl;
        }
    }
    std::cout << "-----------------------------------------" << std::endl;
}
