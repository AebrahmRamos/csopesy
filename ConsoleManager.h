#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Screen.h"
#include "ProcessManager.h"
#include "ReportGenerator.h"

struct GPUInfo {
    int id;
    std::string name;
    std::string persistence;
    std::string busId;
    std::string display;
    std::string ecc;
    int fanPercent;
    int tempC;
    std::string perf;
    int powerUsage;
    int powerCap;
    int memoryUsed;
    int memoryTotal;
    int gpuUtil;
    std::string computeMode;
    std::string mig;
};

struct ProcessInfo {
    int gpu;
    std::string gi;
    std::string ci;
    int pid;
    std::string type;
    std::string processName;
    int memoryUsage;
};

struct Config {
    // Phase 1 parameters
    int numCpu = 2;
    std::string scheduler = "rr";
    int quantumCycles = 4;
    int batchProcessFreq = 1;
    int minIns = 100;
    int maxIns = 100;
    int delaysPerExec = 0;
    
    // Memory management parameters
    int maxOverallMem = 16384;
    int memPerFrame = 16;
    int memPerProc = 4096;
    std::string holeFitPolicy = "F"; // F for First-fit
    
    // Phase 2 parameters
    bool enableVirtualMemory = false;        // Enable virtual memory management
    int minMemPerProc = 64;                  // Minimum process memory
    int maxMemPerProc = 4096;                // Maximum process memory
    std::string pageReplacementAlg = "LRU";  // Page replacement algorithm
    
    bool isValid = false;
    std::string errorMessage = "";
};

class ConsoleManager {
private:
    std::shared_ptr<Screen> currentScreen;
    std::map<std::string, std::shared_ptr<Screen>> screens;
    std::map<std::string, uint16_t> declaredVariables;
    bool inMainMenu;
    bool initialized;
    
    // Process Manager and handling
    std::unique_ptr<ProcessManager> processManager;
    std::unique_ptr<ReportGenerator> reportGenerator;
    
    // Configuration
    Config config;
    bool loadConfig(const std::string& filename);
    bool loadConfig(Config& cfg);
    bool validateConfig();
    Config* getOSConfig() { return &config; }

    // Private helper methods
    
    std::string extractName(const std::string& command);
    bool findCommand(const std::string& text, const std::string& command);
    std::string extractPrintMsg(const std::string& command);
    void printConfigError(const std::string& error);            // Prints out the error
    void printGPUInfo(const GPUInfo& gpu);
    void printProcessInfo(const ProcessInfo& process);
    std::vector<GPUInfo> getDummyGPUData();
    std::vector<ProcessInfo> getDummyProcessData();
    std::vector<ProcessInfo> getRealProcessData();
    std::string extractCommandValue(const std::string& command, const std::string type);
    
    // Phase 2 helper functions for enhanced screen commands
    size_t extractMemorySize(const std::string& command);
    std::vector<std::string> extractCustomInstructions(const std::string& command);
    
    // Helper functions for ADD/SUBTRACT operations
    void ensureVariableExists(const std::string& varName);
    uint16_t getVariableValue(const std::string& varName);
    void setVariableValue(const std::string& varName, uint16_t value);
    std::vector<std::string> parseCommaSeparatedArgs(const std::string& argString);
public:
    ConsoleManager();
    ~ConsoleManager();
    void clearScreen();
    void printHeader();
    void commandHelp();
    void commandInitialize();
    void commandSchedulerStart();
    void commandSchedulerTest();
    void commandSchedulerStop();
    void commandSchedulerHelp();
    void commandStatus();
    void commandReportUtil();
    void commandNvidiaSmi();
    void commandProcessSmi();
    void commandVmstat();
    void commandClear();
    void commandExit();
    void createScreen(const std::string& name);
    void resumeScreen(const std::string& name);
    void listScreens();
    void handleScreenCommand(const std::string& command);
    void processMainMenuCommand(const std::string& command);
    void processScreenCommand(const std::string& command);
    void processCommand(const std::string& command);
    void showPrompt();
    void run();
};

#endif
