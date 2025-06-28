#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <string>
#include <vector>

class ProcessManager;

struct UtilizationData {
    double cpuUtilization;
    int coresUsed;
    int coresAvailable;
    int totalProcesses;
    int runningProcesses;
    int finishedProcesses;
    std::vector<std::string> runningProcessList;
    std::vector<std::string> finishedProcessList;
    std::string timestamp;
};

class ReportGenerator {
private:
    std::string getCurrentTimestamp();
    std::string formatProcessInfo(const std::string& name, const std::string& date, int core, int currentLine, int totalLines, bool finished = false);
    
public:
    UtilizationData calculateUtilization(ProcessManager* pm);
    void generateReport(ProcessManager* pm, const std::string& filename = "csopesy-log.txt");
    void displayReport(ProcessManager* pm);
};

#endif
