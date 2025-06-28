#include "ProcessGenerator.h"
#include "ProcessManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>

ProcessGenerator::ProcessGenerator(ProcessManager* pm) 
    : generating(false), processCounter(0), processManager(pm),
      rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
}

ProcessGenerator::~ProcessGenerator() {
    stopGeneration();
}

void ProcessGenerator::startGeneration(int freq, int minInstructions, int maxInstructions) {
    if (generating) {
        return;
    }
    
    batchFreq = freq;
    minIns = minInstructions;
    maxIns = maxInstructions;
    instructionDist = std::uniform_int_distribution<int>(minIns, maxIns);
    
    generating = true;
    generatorThread = std::thread(&ProcessGenerator::generatorLoop, this);
    
}

void ProcessGenerator::stopGeneration() {
    if (generating) {
        generating = false;
        if (generatorThread.joinable()) {
            generatorThread.join();
        }
    }
}

bool ProcessGenerator::isGenerating() const {
    return generating;
}

std::string ProcessGenerator::generateProcessName() {
    processCounter++;
    std::stringstream ss;
    ss << "p" << std::setfill('0') << std::setw(2) << processCounter;
    return ss.str();
}

std::shared_ptr<Process> ProcessGenerator::createDummyProcess() {
    std::string name = generateProcessName();
    int instructionCount = instructionDist(rng);
    int processId = processCounter;
    
    auto process = std::make_shared<Process>(name, processId, instructionCount);
    
    auto instructions = instructionGenerator.generateRandomInstructions(name, minIns, maxIns);
    process->setInstructions(instructions);
    
    return process;
}

int ProcessGenerator::getProcessCount() const {
    return processCounter;
}

void ProcessGenerator::generatorLoop() {
    auto lastGeneration = std::chrono::steady_clock::now();
    
    while (generating) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastGeneration);
        
        // converts to cpu cycles
        int cycleTimeMs = batchFreq * 100;
        
        if (elapsed.count() >= cycleTimeMs) {
            // Generate new process
            auto newProcess = createDummyProcess();
            
            // Add to process manager
            if (processManager) {
                processManager->addGeneratedProcess(newProcess);
            }
            
            lastGeneration = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
