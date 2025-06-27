#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Screen.h"
#include "ProcessManager.h"

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
    int numCpu = 4;
    std::string scheduler = "fcfs";
    int quantumCycles = 5;
    int batchProcessFreq = 1;
    int minIns = 1000;
    int maxIns = 2000;
    int delaysPerExec = 0;
    
    bool isValid = false;
    std::string errorMessage = "";
};

class ConsoleManager {
private:
    std::shared_ptr<Screen> currentScreen;
    std::map<std::string, std::shared_ptr<Screen>> screens;
    bool inMainMenu;
    bool initialized;
    Config config;
    std::unique_ptr<ProcessManager> processManager;
    std::string extractName(const std::string& command);
    bool findCommand(const std::string& text, const std::string& command);
    std::string extractPrintMsg(const std::string& command);
    bool loadConfig(const std::string& filename);               // Loads the configuration from a file
    bool validateConfig();                                      // Checks for config errors
    void printConfigError(const std::string& error);            // Prints out the error
    void printGPUInfo(const GPUInfo& gpu);
    void printProcessInfo(const ProcessInfo& process);
    std::vector<GPUInfo> getDummyGPUData();
    std::vector<ProcessInfo> getDummyProcessData();
public:
    ConsoleManager();
    ~ConsoleManager();
    void clearScreen();
    void printHeader();
    void commandHelp();
    void commandInitialize();
    void commandSchedulerTest();
    void commandSchedulerStop();
    void commandReportUtil();
    void commandNvidiaSmi();
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
