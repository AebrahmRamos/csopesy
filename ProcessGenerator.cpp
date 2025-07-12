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
    
    // Ensure instruction count is within reasonable bounds (added safety check)
    int minSafeIns = std::max(5, minIns);  // At least 5 instructions
    int maxSafeIns = std::min(10000, maxIns);  // At most 10,000 instructions
    
    if (minSafeIns > maxSafeIns) {
        std::swap(minSafeIns, maxSafeIns); // Ensure min < max
    }
    
    // Create a new distribution with the safe values if needed
    std::uniform_int_distribution<int> safeInsDist(minSafeIns, maxSafeIns);
    int instructionCount = safeInsDist(rng);
    
    int processId = processCounter;
    
    // Reduced verbosity - only log minimal process creation info
    // std::cout << "Created process " << name << " (ID: " << processId << ")" << std::endl;
    
    auto process = std::make_shared<Process>(name, processId, instructionCount);
    
    try {
        auto instructions = instructionGenerator.generateRandomInstructions(name, minSafeIns, maxSafeIns);
        process->setInstructions(instructions);
    } catch (const std::exception& e) {
        std::cerr << "Exception while generating instructions: " << e.what() << std::endl;
        // Provide basic instructions if generation fails
        std::vector<std::string> basicInstructions = {
            "# Basic instruction 1",
            "# Basic instruction 2",
            "# Basic instruction 3"
        };
        process->setInstructions(basicInstructions);
    }
    
    return process;
}

int ProcessGenerator::getProcessCount() const {
    return processCounter;
}

void ProcessGenerator::generatorLoop() {
    auto lastGeneration = std::chrono::steady_clock::now();
    
    std::cout << "Process generator started" << std::endl;
              
    // Force immediate generation of first process for testing
    {
        auto newProcess = createDummyProcess();
        
        if (processManager) {
            processManager->addGeneratedProcess(newProcess);
        }
    }
    
    while (generating) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastGeneration);
        
        // Convert batchFreq to milliseconds (faster for testing)
        // Lower value = faster process generation
        int cycleTimeMs = batchFreq * 100; 
        
        if (elapsed.count() >= cycleTimeMs) {
            // Generate new process
            auto newProcess = createDummyProcess();
            
            // Add to process manager
            if (processManager) {
                processManager->addGeneratedProcess(newProcess);
            } else {
                std::cerr << "Error: Process manager is null in generator loop" << std::endl;
            }
            
            lastGeneration = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
