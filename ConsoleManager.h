#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <string>
#include <map>
#include <memory>
#include "Screen.h"

class ConsoleManager {
private:
    std::map<std::string, std::shared_ptr<Screen>> screens;
    std::shared_ptr<Screen> currentScreen;
    bool inMainMenu;
    std::string extractName(const std::string& command);
    bool findCommand(const std::string& text, const std::string& command);
public:
    ConsoleManager();
    void clearScreen();
    void printHeader();
    void commandHelp();
    void commandInitialize();
    void commandSchedulerTest();
    void commandSchedulerStop();
    void commandReportUtil();
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
