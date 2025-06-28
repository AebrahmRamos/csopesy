#include "Scheduler.h"
#include "Process.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

Scheduler::Scheduler(ProcessManager* pm) : running(false), activeProcesses(0), processManager(pm) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    running = true;
    for (int i = 0; i < NUM_CORES; ++i) {
        cpuThreads.emplace_back(&Scheduler::cpuWorker, this, i);
    }
}

void Scheduler::stop() {
    running = false;
    cv.notify_all();
    for (auto& thread : cpuThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cpuThreads.clear();
}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process);
        activeProcesses++;
    }
    cv.notify_one();
}

bool Scheduler::isProcessing() const {
    return activeProcesses > 0;
}

void Scheduler::cpuWorker(int coreId) {
    while (running) {
        std::shared_ptr<Process> process = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !running || !readyQueue.empty(); });
            
            if (!running && readyQueue.empty()) {
                return;
            }
            
            if (!readyQueue.empty()) {
                process = readyQueue.front();
                readyQueue.pop();
            }
        }
        
        if (process) {
            executeProcess(process, coreId);
        }
    }
}

void Scheduler::executeProcess(std::shared_ptr<Process> process, int coreId) {
    if (processManager) {
        processManager->updateProcessCore(process->getProcessId(), coreId);
    }
    
    // Execute instructions automatically for auto-executing processes
    if (process->isAutoExecuting()) {
        while (process->hasMoreInstructions() && process->getIsActive()) {
            std::string currentInstruction = process->getCurrentInstruction();
            
            // Execute the instruction
            executeInstruction(process, currentInstruction);
            
            // Add to execution log
            process->addToExecutionLog(currentInstruction);
            
            // Advance to next instruction
            process->advanceInstruction();
            
            // Simulate CPU work delay (from config delays-per-exec)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else {
        // Legacy behavior for manually created processes
        while (process->getCurrentLine() <= process->getTotalLines() && process->getIsActive()) {
            process->incrementLine();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    activeProcesses--;
}

void Scheduler::executeInstruction(std::shared_ptr<Process> process, const std::string& instruction) {
    // Parse and execute the instruction
    if (instruction.find("PRINT(") == 0) {
        // PRINT instruction - just log it (output will be shown in process-smi)
        // No additional action needed, instruction is already logged
    }
    else if (instruction.find("DECLARE(") == 0) {
        // Parse DECLARE(var, value)
        size_t start = instruction.find('(') + 1;
        size_t end = instruction.find(')', start);
        if (end != std::string::npos) {
            std::string params = instruction.substr(start, end - start);
            size_t commaPos = params.find(',');
            if (commaPos != std::string::npos) {
                std::string varName = params.substr(0, commaPos);
                std::string valueStr = params.substr(commaPos + 1);
                
                // Trim whitespace
                varName.erase(0, varName.find_first_not_of(" \t"));
                varName.erase(varName.find_last_not_of(" \t") + 1);
                valueStr.erase(0, valueStr.find_first_not_of(" \t"));
                valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
                
                try {
                    uint16_t value = static_cast<uint16_t>(std::stoi(valueStr));
                    process->setVariable(varName, value);
                } catch (...) {
                    // Invalid value, set to 0
                    process->setVariable(varName, 0);
                }
            }
        }
    }
    else if (instruction.find("ADD(") == 0) {
        // Parse ADD(target, source1, source2)
        executeArithmeticInstruction(process, instruction, "ADD");
    }
    else if (instruction.find("SUBTRACT(") == 0) {
        // Parse SUBTRACT(target, source1, source2)
        executeArithmeticInstruction(process, instruction, "SUBTRACT");
    }
    else if (instruction.find("SLEEP(") == 0) {
        // Parse SLEEP(ticks)
        size_t start = instruction.find('(') + 1;
        size_t end = instruction.find(')', start);
        if (end != std::string::npos) {
            std::string ticksStr = instruction.substr(start, end - start);
            try {
                int ticks = std::stoi(ticksStr);
                if (ticks > 0) {
                    // Sleep for ticks * 100ms (simulating CPU ticks)
                    std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 100));
                }
            } catch (...) {
                // Invalid ticks, ignore
            }
        }
    }
    else if (instruction.find("FOR(") == 0) {
        // FOR loops are more complex and should be handled recursively
        // For now, just log the instruction
        // TODO: Implement nested instruction execution
    }
}

void Scheduler::executeArithmeticInstruction(std::shared_ptr<Process> process, const std::string& instruction, const std::string& operation) {
    // Parse operation(target, source1, source2)
    size_t start = instruction.find('(') + 1;
    size_t end = instruction.find(')', start);
    if (end == std::string::npos) return;
    
    std::string params = instruction.substr(start, end - start);
    std::vector<std::string> args;
    
    // Split by comma
    size_t pos = 0;
    while (pos < params.length() && args.size() < 3) {
        size_t commaPos = params.find(',', pos);
        if (commaPos == std::string::npos) {
            args.push_back(params.substr(pos));
            break;
        } else {
            args.push_back(params.substr(pos, commaPos - pos));
            pos = commaPos + 1;
        }
    }
    
    if (args.size() != 3) return;
    
    // Trim whitespace from all arguments
    for (auto& arg : args) {
        arg.erase(0, arg.find_first_not_of(" \t"));
        arg.erase(arg.find_last_not_of(" \t") + 1);
    }
    
    std::string target = args[0];
    
    // Get values for source1 and source2
    uint16_t val1 = getValueFromArgument(process, args[1]);
    uint16_t val2 = getValueFromArgument(process, args[2]);
    
    // Perform operation
    uint16_t result;
    if (operation == "ADD") {
        uint32_t temp = static_cast<uint32_t>(val1) + static_cast<uint32_t>(val2);
        result = (temp > 65535) ? 65535 : static_cast<uint16_t>(temp);
    } else if (operation == "SUBTRACT") {
        result = (val1 >= val2) ? (val1 - val2) : 0;
    } else {
        result = 0;
    }
    
    // Store result
    process->setVariable(target, result);
}

uint16_t Scheduler::getValueFromArgument(std::shared_ptr<Process> process, const std::string& arg) {
    // Check if argument is a number or variable
    if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
        try {
            int value = std::stoi(arg);
            return (value < 0) ? 0 : (value > 65535) ? 65535 : static_cast<uint16_t>(value);
        } catch (...) {
            return 0;
        }
    } else {
        // It's a variable
        process->ensureVariableExists(arg);
        return process->getVariable(arg);
    }
}
