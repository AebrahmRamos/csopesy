#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <vector>
#include <memory>
#include <map>
#include "Process.h"
#include "Scheduler.h"

class ProcessManager {
private:
    std::vector<std::shared_ptr<Process>> processes;
    std::unique_ptr<Scheduler> scheduler;
    std::map<int, int> processCoreMap; // processId -> coreId

public:
    ProcessManager();
    ~ProcessManager();
    
    void initialize();
    void startScheduler();
    void stopScheduler();
    void showProcessStatus();
    
    std::vector<std::shared_ptr<Process>> getRunningProcesses() const;
    std::vector<std::shared_ptr<Process>> getFinishedProcesses() const;
    bool hasActiveProcesses() const;
    
    void updateProcessCore(int processId, int coreId);
    int getProcessCore(int processId) const;
};

#endif
