#include "Scheduler.h"
#include "Process.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

Scheduler::Scheduler(ProcessManager* pm) : running(false), activeProcesses(0), processManager(pm),
    schedulerType(SchedulerType::FCFS), quantumCycles(5), numCores(1) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    running = true;
    for (int i = 0; i < numCores; ++i) {
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
            // Wait for a process to be available or for the scheduler to stop
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return !readyQueue.empty() || !running; });
            
            // Exit if the scheduler is stopped and no processes are left
            if (!running && readyQueue.empty()) {
                return;
            }
            
            // Get the next process from the ready queue
            if (!readyQueue.empty()) {
                process = readyQueue.front();
                readyQueue.pop();
            }
        }
        
        // Execute the process
        if (process) {
            executeProcess(process, coreId);
        }
    }
}


void Scheduler::executeProcess(std::shared_ptr<Process> process, int coreId) {
    if (processManager) {
        processManager->updateProcessCore(process->getProcessId(), coreId);
    }
    
    // choose scheduling algorithm
    if (schedulerType == SchedulerType::RR) {
        executeProcessRR(process, coreId);
    } else {
        executeProcessFCFS(process, coreId);
    }
    
    activeProcesses--;
}

void Scheduler::executeProcessFCFS(std::shared_ptr<Process> process, int coreId) {
    if (process->isAutoExecuting()) {
        while (process->hasMoreInstructions() && process->getIsActive()) {
            std::string currentInstruction = process->getCurrentInstruction();
            
            executeInstruction(process, currentInstruction);
            
            process->addToExecutionLog(currentInstruction);
            
            process->advanceInstruction();
        }
    }

    // If the process has finished all instructions, mark it as finished
    if (!process->getIsActive()) {
        activeProcesses--;  // Reduce the active process count
        std::cout << "Process " << process->getName() << " has finished executing." << std::endl;
    }
}


void Scheduler::executeProcessRR(std::shared_ptr<Process> process, int coreId) {
    int cyclesUsed = 0;

    // Check if the process is in auto-execution mode
    if (process->isAutoExecuting()) {
        while (process->hasMoreInstructions() && process->getIsActive() && cyclesUsed < quantumCycles) {
            std::string currentInstruction = process->getCurrentInstruction();
            
            // Execute the instruction
            executeInstruction(process, currentInstruction);
            
            // Add to execution log
            process->addToExecutionLog(currentInstruction);
            
            // Advance to next instruction
            process->advanceInstruction();
            
            cyclesUsed++;

        }

        // Requeue if the process still has instructions left
        if (process->hasMoreInstructions() && process->getIsActive() && cyclesUsed >= quantumCycles && running) {
            requeueProcess(process);
            activeProcesses++;  // Increment active processes when the process is requeued
        }
    } else {
        // Manual execution (legacy handling)
        while (process->getCurrentLine() <= process->getTotalLines() && process->getIsActive() && cyclesUsed < quantumCycles) {
            process->incrementLine();
            cyclesUsed++;
        }

        // Requeue if not finished and still active
        if (process->getCurrentLine() <= process->getTotalLines() && process->getIsActive() && running) {
            requeueProcess(process);
            activeProcesses++;
        }
    }

    // If the process has finished all instructions, mark it as finished
    if (!process->getIsActive()) {
        activeProcesses--;
    }
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 50));
                }
            } catch (...) {
                // Invalid ticks, ignore
            }
        }
    }
    else if (instruction.find("FOR(") == 0) {
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

// Configuration methods
void Scheduler::setSchedulerConfig(const std::string& algorithm, int quantum, int cores) {
    schedulerType = parseSchedulerType(algorithm);
    quantumCycles = quantum;
    
    // If numCores is changing and scheduler is running, restart with new core count
    if (numCores != cores && running) {
        stop();
        numCores = cores;
        start();
    } else {
        numCores = cores;
    }
}

SchedulerType Scheduler::parseSchedulerType(const std::string& algorithm) {
    if (algorithm == "rr") {
        return SchedulerType::RR;
    }
    return SchedulerType::FCFS; // Default to FCFS
}

void Scheduler::requeueProcess(std::shared_ptr<Process> process) {
    {
        // Ensure thread safety when accessing the queue
        std::lock_guard<std::mutex> lock(queueMutex);
        
        if (process->getIsActive()) {  // Only requeue active processes
            readyQueue.push(process);
        }
    }
    cv.notify_one();  // Notify worker thread to pick up a process
}
