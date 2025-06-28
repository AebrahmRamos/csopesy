#ifndef PROCESSGENERATOR_H
#define PROCESSGENERATOR_H

#include <memory>
#include <thread>
#include <atomic>
#include <random>
#include <chrono>
#include "Process.h"

class ProcessManager; // Forward declaration

class ProcessGenerator {
private:
    std::atomic<bool> generating;
    std::thread generatorThread;
    int processCounter;
    int batchFreq;
    int minIns;
    int maxIns;
    ProcessManager* processManager;
    std::mt19937 rng;
    std::uniform_int_distribution<int> instructionDist;
    
    void generatorLoop();
    
public:
    ProcessGenerator(ProcessManager* pm);
    ~ProcessGenerator();
    
    void startGeneration(int freq, int minInstructions, int maxInstructions);
    void stopGeneration();
    bool isGenerating() const;
    
    std::string generateProcessName();
    std::shared_ptr<Process> createDummyProcess();
    int getProcessCount() const;
};

#endif
