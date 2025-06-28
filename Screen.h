#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <iostream>
#include <memory>

class Process; // Forward declaration

class Screen {
private:
    std::string processName;
    int currentLine;
    int totalLines;
    std::string creationDate;
    bool isActive;
    int arrivalTime;
    std::shared_ptr<Process> attachedProcess; 
public:
    Screen(const std::string& name, int totalCommands = 100);
    void display();
    void simulateProgress();
    void showProcessInfo();
    void attachToProcess(std::shared_ptr<Process> process);
    std::shared_ptr<Process> getAttachedProcess() const;
    bool hasAttachedProcess() const;
    std::string getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCreationDate() const;
    bool getIsActive() const;
    void setActive(bool active);
    int getArrivalTime() const;
};

#endif
