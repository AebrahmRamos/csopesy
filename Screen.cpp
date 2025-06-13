#include "Screen.h"
#include <ctime>

Screen::Screen(const std::string& name, int totalCommands) : 
    processName(name), 
    currentLine(1), 
    totalLines(totalCommands), 
    isActive(true),
    arrivalTime(static_cast<int>(time(nullptr))) {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", now);
    creationDate = std::string(timeBuffer);
}

void Screen::display() {
    std::cout << "\n==============================" << std::endl;
    std::cout << "Process Name: " << processName << std::endl;
    std::cout << "Current line of instruction: " << currentLine << " / " << totalLines << std::endl;
    std::cout << "Created on: " << creationDate << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "\033[33m(Type 'exit' to return to main menu)\033[0m" << std::endl;
}

void Screen::simulateProgress() {
    if (currentLine < totalLines) {
        currentLine++;
    }
}

std::string Screen::getName() const { return processName; }
int Screen::getCurrentLine() const { return currentLine; }
int Screen::getTotalLines() const { return totalLines; }
std::string Screen::getCreationDate() const { return creationDate; }
bool Screen::getIsActive() const { return isActive; }
void Screen::setActive(bool active) { isActive = active; }
int Screen::getArrivalTime() const { return arrivalTime; }
