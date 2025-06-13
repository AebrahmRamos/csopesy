#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Screen.h"
#include "Scheduler.h"

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

class ConsoleManager {
private:
    std::shared_ptr<Screen> currentScreen;
    std::map<std::string, std::shared_ptr<Screen>> screens;
    bool inMainMenu;
    std::unique_ptr<Scheduler> scheduler;
    std::string extractName(const std::string& command);
    bool findCommand(const std::string& text, const std::string& command);
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
