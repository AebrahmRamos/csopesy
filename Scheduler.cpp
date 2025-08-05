#include "Scheduler.h"
#include "Process.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

Scheduler::Scheduler(ProcessManager* pm) : running(false), generationStopped(false), activeProcesses(0), processManager(pm),
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
    generationStopped = true;
    cv.notify_all();
    for (auto& thread : cpuThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cpuThreads.clear();
}

void Scheduler::stopGracefully() {
    // This method allows current processes to finish but prevents new ones from being queued
    generationStopped = true;
    // Don't set running = false - let processes complete naturally
    // cpuWorker threads will continue until queue is empty
}

void Scheduler::addProcess(std::shared_ptr<Process> process) {
    if (!process) {
        std::cerr << "Attempted to add null process to scheduler" << std::endl;
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process);
        activeProcesses++;
        // Removed verbose process addition logging
    }
    cv.notify_one();
}

bool Scheduler::isProcessing() const {
    return activeProcesses > 0;
}

bool Scheduler::isRunning() const {
    return running;
}

void Scheduler::cpuWorker(int coreId) {
    // CPU worker thread running silently
    
    // Continue running until the scheduler is stopped AND the queue is empty
    while (running || !readyQueue.empty()) {
        std::shared_ptr<Process> process = nullptr;
        {
            // Wait for a process to be available or for the scheduler to stop
            std::unique_lock<std::mutex> lock(queueMutex);
            
            cv.wait(lock, [this]() { return !readyQueue.empty() || !running; });
            
            // Only exit if the scheduler is stopped AND there are no more processes to run
            // This allows existing processes to finish even after scheduler-stop is called
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
    if (!process) {
        std::cerr << "Error: Null process passed to executeProcess for core " << coreId << std::endl;
        return;
    }
    
    try {
        if (processManager) {
            processManager->updateProcessCore(process->getProcessId(), coreId);
        }
        
        // Removed scheduler logging
                
        if (schedulerType == SchedulerType::RR) {
            executeProcessRR(process, coreId);
        } else {
            executeProcessFCFS(process, coreId);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in executeProcess: " << e.what() << std::endl;
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
            
            // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // If the process has finished all instructions, mark it as finished
    if (!process->getIsActive()) {
        activeProcesses--;  // Reduce the active process count
        // Process has finished executing
    }
}


void Scheduler::executeProcessRR(std::shared_ptr<Process> process, int coreId) {
    // Removed verbose process execution logging
    
    if (process->isAutoExecuting()) {
        int cyclesExecuted = 0;
        
        // Execute up to the quantum limit
        while (process->hasMoreInstructions() && process->getIsActive() && cyclesExecuted < quantumCycles) {
            std::string currentInstruction = process->getCurrentInstruction();
            
            // Removed instruction execution log
            executeInstruction(process, currentInstruction);
            
            process->addToExecutionLog(currentInstruction);
            process->advanceInstruction();
            
            cyclesExecuted++;
        }
        
        // Removed instruction execution summary log
        
        // If the process is finished, release its memory
        if (!process->hasMoreInstructions() || !process->getIsActive()) {
            // Process is finished, release memory
            if (processManager) {
                processManager->releaseProcessMemory(process);
            }
            process->setAssignedCore(-1);
            
            // Generate memory snapshot at the end of quantum cycle
            if (processManager) {
                // Increment quantum cycle
                processManager->incrementQuantumCycle();
            }
            
            activeProcesses--; // Decrement active process count
            // Process completed
            return; // Process is complete
        }
        
        // Generate memory snapshot at the end of quantum cycle
        if (processManager) {
            // Increment quantum cycle
            processManager->incrementQuantumCycle();
        }
        
        // If the process has more instructions, add it back to the ready queue
        if (process->hasMoreInstructions() && process->getIsActive()) {
            // Process has more work, adding back to ready queue
            std::lock_guard<std::mutex> lock(queueMutex);
            process->setAssignedCore(-1);
            readyQueue.push(process);
        }
    } else {
        // Manual execution mode
        if (process->hasMoreInstructions() && process->getIsActive()) {
            std::string currentInstruction = process->getCurrentInstruction();
            
            executeInstruction(process, currentInstruction);
            
            process->addToExecutionLog(currentInstruction);
            process->advanceInstruction();
            
            // If the process is finished, release its memory
            if (!process->hasMoreInstructions() || !process->getIsActive()) {
                if (processManager) {
                    processManager->releaseProcessMemory(process);
                }
                process->setAssignedCore(-1);
                
                // Generate memory snapshot at the end of quantum cycle
                if (processManager) {
                    processManager->incrementQuantumCycle();
                }
            } else {
                // If the process has more instructions, add it back to the ready queue
                std::lock_guard<std::mutex> lock(queueMutex);
                process->setAssignedCore(-1);
                readyQueue.push(process);
                
                // Generate memory snapshot at the end of quantum cycle
                if (processManager) {
                    processManager->incrementQuantumCycle();
                }
            }
        }
    }
}


void Scheduler::executeInstruction(std::shared_ptr<Process> process, const std::string& instruction) {
    // Parse and execute the instruction
    if (instruction.find("PRINT(") == 0) {
        size_t start = instruction.find('(') + 1;
        size_t end = instruction.find(')', start);
        if (end != std::string::npos) {
            std::string content = instruction.substr(start, end - start);
    
            // Split by '+'
            std::stringstream ss(content);
            std::string segment;
            bool first = true;
            while (std::getline(ss, segment, '+')) {
                // Trim whitespace
                segment.erase(0, segment.find_first_not_of(" \t"));
                segment.erase(segment.find_last_not_of(" \t") + 1);
    
                // If quoted string
                if (!segment.empty() && segment.front() == '"' && segment.back() == '"') {
                    std::cout << segment.substr(1, segment.size() - 2);
                } else if (!segment.empty()) {
                    // Variable: print its value
                    process->ensureVariableExists(segment);
                    std::cout << process->getVariable(segment);
                }
            }
            std::cout << std::endl;
        }
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
        // TODO: Implement nested instruction execution
    }
    else if (instruction.find("READ(") == 0) {
        // Parse READ(var, address) - Phase 2 instruction
        executeMemoryInstruction(process, instruction, "READ");
    }
    else if (instruction.find("WRITE(") == 0) {
        // Parse WRITE(address, value) - Phase 2 instruction
        executeMemoryInstruction(process, instruction, "WRITE");
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

// Phase 2: Memory instruction execution
void Scheduler::executeMemoryInstruction(std::shared_ptr<Process> process, const std::string& instruction, const std::string& operation) {
    try {
        if (operation == "READ") {
            // Parse READ(var, address)
            auto args = parseInstructionArgs(instruction);
            if (args.size() < 2) {
                std::cerr << "Invalid READ instruction format: " << instruction << std::endl;
                return;
            }
            
            std::string varName = args[0];
            uint32_t address = parseHexAddress(args[1]);
            
            // Read from virtual memory (may trigger page fault)
            uint16_t value = processManager->readProcessMemory(process->getProcessId(), address);
            process->setVariable(varName, value);
            
            std::cout << "READ: Process " << process->getProcessId() 
                      << " read value " << value << " from address 0x" 
                      << std::hex << address << std::dec << " into variable " << varName << std::endl;
        }
        else if (operation == "WRITE") {
            // Parse WRITE(address, value)
            auto args = parseInstructionArgs(instruction);
            if (args.size() < 2) {
                std::cerr << "Invalid WRITE instruction format: " << instruction << std::endl;
                return;
            }
            
            uint32_t address = parseHexAddress(args[0]);
            uint16_t value = getValueFromArgument(process, args[1]);
            
            // Write to virtual memory (may trigger page fault)
            processManager->writeProcessMemory(process->getProcessId(), address, value);
            
            std::cout << "WRITE: Process " << process->getProcessId() 
                      << " wrote value " << value << " to address 0x" 
                      << std::hex << address << std::dec << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Memory instruction error: " << e.what() << std::endl;
    }
}

// Helper method to parse hexadecimal addresses
uint32_t Scheduler::parseHexAddress(const std::string& addressStr) {
    std::string trimmed = addressStr;
    
    // Trim whitespace
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
    
    // Handle hex prefix
    if (trimmed.find("0x") == 0 || trimmed.find("0X") == 0) {
        return std::stoul(trimmed, nullptr, 16);
    } else {
        // Assume decimal if no hex prefix
        return std::stoul(trimmed, nullptr, 10);
    }
}

// Helper method to parse instruction arguments
std::vector<std::string> Scheduler::parseInstructionArgs(const std::string& instruction) {
    std::vector<std::string> args;
    
    size_t start = instruction.find('(') + 1;
    size_t end = instruction.find(')', start);
    
    if (end == std::string::npos) {
        return args; // Invalid format
    }
    
    std::string params = instruction.substr(start, end - start);
    
    // Split by comma
    size_t pos = 0;
    while (pos < params.length()) {
        size_t commaPos = params.find(',', pos);
        std::string arg;
        
        if (commaPos == std::string::npos) {
            arg = params.substr(pos);
            pos = params.length();
        } else {
            arg = params.substr(pos, commaPos - pos);
            pos = commaPos + 1;
        }
        
        // Trim whitespace
        arg.erase(0, arg.find_first_not_of(" \t"));
        arg.erase(arg.find_last_not_of(" \t") + 1);
        
        if (!arg.empty()) {
            args.push_back(arg);
        }
    }
    
    return args;
}
