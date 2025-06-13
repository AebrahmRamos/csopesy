#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <iostream>

class Screen {
private:
    std::string processName;
    int currentLine;
    int totalLines;
    std::string creationDate;
    bool isActive;
    int arrivalTime;

public:
    Screen(const std::string& name, int totalCommands = 100);
    void display();
    void simulateProgress();
    std::string getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCreationDate() const;
    bool getIsActive() const;
    void setActive(bool active);
    int getArrivalTime() const;
};

#endif
