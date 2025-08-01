#include "ProcessManager.h"
#include "ProcessGenerator.h"
#include "ConsoleManager.h"
#include "MemoryManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>

ProcessManager::ProcessManager() : numCores(4), currentQuantumCycle(0), useVirtualMemory(false) {
    scheduler = std::make_unique<Scheduler>(this);
    generator = std::make_unique<ProcessGenerator>(this);
    memoryManager = std::make_unique<MemoryManager>(16384, 4096, 16, "F");
    // vmManager will be initialized in setConfig() to avoid premature initialization
}

ProcessManager::~ProcessManager() {
    if (generator) {
        generator->stopGeneration();
    }
    if (scheduler) {
        scheduler->stop();
    }
}

void ProcessManager::setConfig(const Config& config) {
    numCores = config.numCpu;
    storedConfig = std::make_unique<Config>(config); // Store a copy in unique_ptr
    
    if (scheduler) {
        scheduler->setSchedulerConfig(config.scheduler, config.quantumCycles, config.numCpu);
    }
    
    if (memoryManager) {
        // Re-initialize memory manager with config values
        // For Phase 2 tests, use min-mem-per-proc as the base memory per process
        int memPerProc = (config.minMemPerProc > 0) ? config.minMemPerProc : config.memPerProc;
        memoryManager = std::make_unique<MemoryManager>(
            config.maxOverallMem,
            memPerProc,
            config.memPerFrame,
            config.holeFitPolicy
        );
    }
    
    // Initialize virtual memory manager for Phase 2 (always create it)
    vmManager = std::make_unique<VirtualMemoryManager>(
        config.maxOverallMem,
        config.memPerFrame
    );
}

void ProcessManager::startProcessGeneration() {
    if (generator && storedConfig) {
        // Validate configuration values
        int freq = storedConfig->batchProcessFreq;
        int minIns = storedConfig->minIns;
        int maxIns = storedConfig->maxIns;
        
        // Sanity check the values - if they seem invalid, use sensible defaults
        if (freq <= 0 || freq > 10) {
            std::cout << "Warning: Invalid batch frequency value (" << freq << "), using default of 1" << std::endl;
            freq = 1;
        }
        
        // More aggressive checking for instruction counts - the crash may be related to these values
        const int MAX_ALLOWED_INSTRUCTIONS = 10000; // Arbitrary limit to prevent extreme values
        
        if (minIns <= 0 || minIns > MAX_ALLOWED_INSTRUCTIONS) {
            std::cout << "Warning: Invalid minIns value (" << minIns << "), using default of 100" << std::endl;
            minIns = 100;
        }
        
        if (maxIns <= 0 || maxIns > MAX_ALLOWED_INSTRUCTIONS || maxIns < minIns) {
            std::cout << "Warning: Invalid maxIns value (" << maxIns << "), using default of 200" << std::endl;
            maxIns = 200;
        }
        
        std::cout << "Starting process generation with config: freq=" << freq 
                  << ", minIns=" << minIns 
                  << ", maxIns=" << maxIns << std::endl;
        
        generator->startGeneration(freq, minIns, maxIns);
    } else if (generator) {
        // Fallback values in case config not set or error
        std::cout << "Starting process generation with fallback values: freq=1, minIns=100, maxIns=200" << std::endl;
        generator->startGeneration(1, 100, 200);
    } else {
        std::cerr << "Error: Cannot start process generation - generator not initialized" << std::endl;
    }
}

void ProcessManager::stopProcessGeneration() {
    if (generator) {
        generator->stopGeneration();
    }
}

bool ProcessManager::isGeneratingProcesses() const {
    return generator ? generator->isGenerating() : false;
}

void ProcessManager::addGeneratedProcess(std::shared_ptr<Process> process) {
    if (!process) return;
    
    {
        std::lock_guard<std::mutex> lock(processMutex);
        processes.push_back(process);
    }
    
    // Try to allocate memory for the process
    bool memoryAllocated = allocateMemoryToProcess(process);
    
    // Only add process to scheduler if memory was successfully allocated
    if (memoryAllocated && scheduler) {
        scheduler->addProcess(process);
    }
    // If memory allocation failed, the process will remain in processes list but not in the ready queue
}

void ProcessManager::initialize() {
    // Removed since it was old and hardcoded and new processes will be using scheduler-start now
    // New processes will be created via scheduler-start command
}

void ProcessManager::startScheduler() {
    if (!scheduler) {
        std::cerr << "Error: Scheduler not initialized!" << std::endl;
        return;
    }
    
    // Start the scheduler if it's not already running
    if (!scheduler->isRunning()) {
        try {
            scheduler->start();
            std::cout << "Scheduler started successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start scheduler: " << e.what() << std::endl;
            return;
        }
    } else {
        std::cout << "Scheduler is already running" << std::endl;
    }
    
    try {
        // Only add processes that have memory allocated
        int addedProcesses = 0;
        int waitingForMemory = 0;
        {
            std::lock_guard<std::mutex> lock(processMutex);
            for (auto& process : processes) {
                if (!process) {
                    std::cerr << "Warning: Null process in processes list" << std::endl;
                    continue;
                }
                
                // Only add to scheduler if memory is already allocated and process is valid
                if (process->getHasMemoryAllocated()) {
                    try {
                        scheduler->addProcess(process);
                        addedProcesses++;
                    } catch (const std::exception& e) {
                        std::cerr << "Failed to add process " << process->getName() 
                                 << " to scheduler: " << e.what() << std::endl;
                    }
                } else {
                    waitingForMemory++;
                    // Try to allocate memory for processes that don't have it yet
                    if (memoryManager && !process->getHasMemoryAllocated()) {
                        bool allocated = allocateMemoryToProcess(process);
                        if (allocated) {
                            try {
                                scheduler->addProcess(process);
                                addedProcesses++;
                                waitingForMemory--;
                            } catch (const std::exception& e) {
                                std::cerr << "Failed to add newly allocated process " << process->getName() 
                                         << " to scheduler: " << e.what() << std::endl;
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << "Scheduler startup summary: " << addedProcesses << " processes added, " 
                 << waitingForMemory << " waiting for memory allocation" << std::endl;
                 
    } catch (const std::exception& e) {
        std::cerr << "Exception in startScheduler: " << e.what() << std::endl;
    }
}

void ProcessManager::stopScheduler() {
    if (scheduler) {
        scheduler->stopGracefully(); // Allow current processes to finish
    }
}

void ProcessManager::showProcessStatus() {
    std::cout << "\n-----------------------------------------" << std::endl;
    
    // Show memory stats if we have a memory manager
    if (memoryManager) {
        int processesInMem = memoryManager->getProcessesInMemory();
        int externalFrag = memoryManager->calculateExternalFragmentation();
        
        std::cout << "Memory Status:" << std::endl;
        std::cout << "  Processes in memory: " << processesInMem << std::endl;
        std::cout << "  External fragmentation: " << externalFrag << " bytes (" 
                 << (externalFrag / 1024) << " KB)" << std::endl;
        std::cout << "  Current Quantum Cycle: " << currentQuantumCycle << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "Running processes:" << std::endl;
    
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            std::cout << std::left << std::setw(12) << process->getName() << " ";
            std::cout << "(" << process->getCreationDate() << ")";
            
            int coreId = getProcessCore(process->getProcessId());
            if (coreId >= 0) {
                std::cout << "     Core: " << std::setw(2) << coreId << "    ";
            } else {
                std::cout << "     Core: --    ";
            }
            
            std::cout << std::setw(5) << process->getCurrentLine() << " / " << process->getTotalLines();
            
            // Add memory allocation status
            if (process->getHasMemoryAllocated()) {
                std::cout << "    [Memory: " << process->getMemoryStartAddress() 
                         << "-" << process->getMemoryEndAddress() << "]";
            } else {
                std::cout << "    [Memory: Waiting]";
            }
            
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nFinished processes:" << std::endl;
    for (const auto& process : processes) {
        if (!process->getIsActive()) {
            std::cout << std::left << std::setw(12) << process->getName() << " ";
            std::cout << "(" << process->getCreationDate() << ")";
            std::cout << "     Finished    ";
            std::cout << std::setw(5) << process->getTotalLines() << " / " << process->getTotalLines();
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nMemory snapshot saved to memory_stamp_" 
             << std::setw(2) << std::setfill('0') << currentQuantumCycle << ".txt"
             << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getRunningProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::vector<std::shared_ptr<Process>> running;
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            running.push_back(process);
        }
    }
    return running;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getFinishedProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::vector<std::shared_ptr<Process>> finished;
    for (const auto& process : processes) {
        if (!process->getIsActive()) {
            finished.push_back(process);
        }
    }
    return finished;
}

std::vector<std::shared_ptr<Process>> ProcessManager::getAllProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    return processes;
}

std::shared_ptr<Process> ProcessManager::findProcessByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(processMutex);
    for (const auto& process : processes) {
        if (process->getName() == name) {
            return process;
        }
    }
    return nullptr;
}

bool ProcessManager::hasActiveProcesses() const {
    std::lock_guard<std::mutex> lock(processMutex);
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            return true;
        }
    }
    return false;
}

int ProcessManager::getNumCores() const {
    return numCores;
}

int ProcessManager::getUsedCores() const {
    std::lock_guard<std::mutex> lock(processMutex);
    std::set<int> usedCores;
    for (const auto& process : processes) {
        if (process->getIsActive()) {
            int core = getProcessCore(process->getProcessId());
            if (core >= 0) {
                usedCores.insert(core);
            }
        }
    }
    return usedCores.size();
}

double ProcessManager::getCpuUtilization() const {
    if (numCores == 0) return 0.0;
    return (static_cast<double>(getUsedCores()) / numCores) * 100.0;
}

void ProcessManager::updateProcessCore(int processId, int coreId) {
    processCoreMap[processId] = coreId;
}

int ProcessManager::getProcessCore(int processId) const {
    auto it = processCoreMap.find(processId);
    return (it != processCoreMap.end()) ? it->second : -1;
}

void ProcessManager::sleepCurrentProcess(int ticks) {
    // Simulate sleeping by adding a delay for the specified number of ticks
    std::cout << "Process sleeping for " << ticks << " ticks.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 10)); // Simulate each tick as 10ms
}

bool ProcessManager::allocateMemoryToProcess(std::shared_ptr<Process> process) {
    if (!process) {
        std::cerr << "Failed to allocate memory: Process is null" << std::endl;
        return false;
    }
    
    if (!memoryManager) {
        std::cerr << "Failed to allocate memory: MemoryManager is null" << std::endl;
        return false;
    }
    
    // Check if the process already has memory allocated
    if (process->getHasMemoryAllocated()) {
        return true;
    }
    
    // Get the required memory size from config or use default
    int memSize = storedConfig ? storedConfig->memPerProc : 4096;
    
    // Safety check for memory size
    if (memSize <= 0) {
        std::cerr << "Invalid memory size configured: " << memSize << std::endl;
        memSize = 4096; // Use default if invalid
    }
    
    // Try to allocate memory using first-fit algorithm
    bool allocated = false;
    try {
        allocated = memoryManager->allocateMemory(process);
    } catch (const std::exception& e) {
        std::cerr << "Exception during memory allocation: " << e.what() << std::endl;
        return false;
    }
    
    if (allocated) {
        // Update process state to indicate memory allocation
        process->setHasMemoryAllocated(true);
        process->setMemorySize(memSize);
        
        try {
            // Get the memory map (start and end addresses) for this process
            auto [start, end] = memoryManager->getProcessMemoryMap(process->getProcessId());
            
            // Verify that we got valid memory addresses
            if (start >= 0 && end > start) {
                process->setMemoryAddress(start, end);
            } else {
                std::cerr << "Warning: Invalid memory addresses returned for process " 
                          << process->getName() << ": start=" << start << ", end=" << end << std::endl;
                // Set some default values to prevent further issues
                process->setMemoryAddress(0, memSize);
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception while retrieving memory map: " << e.what() << std::endl;
            // We'll still return true because memory was allocated
        }
    } else {
        // Only log failures when debugging is needed
        // std::cout << "Failed to allocate memory for process " << process->getName() 
        //          << " (ID: " << process->getProcessId() << ") - Not enough space" << std::endl;
    }
    
    return allocated;
}

void ProcessManager::releaseProcessMemory(std::shared_ptr<Process> process) {
    if (!process || !memoryManager) {
        return;
    }
    
    // Release memory allocated to this process
    memoryManager->deallocateMemory(process->getProcessId());
    
    // Update process state
    process->setHasMemoryAllocated(false);
}

void ProcessManager::generateMemorySnapshot() {
    if (memoryManager) {
        memoryManager->setCurrentQuantum(currentQuantumCycle);
        memoryManager->generateMemorySnapshot(currentQuantumCycle);
    }
}

void ProcessManager::incrementQuantumCycle() {
    currentQuantumCycle++;
    generateMemorySnapshot(); // Generate snapshot after incrementing quantum cycle
}

int ProcessManager::getCurrentQuantumCycle() const {
    return currentQuantumCycle;
}

// Virtual memory management methods (Phase 2)
void ProcessManager::enableVirtualMemory(bool enable) {
    useVirtualMemory = enable;
    std::cout << "Virtual memory " << (enable ? "enabled" : "disabled") << std::endl;
}

bool ProcessManager::isVirtualMemoryEnabled() const {
    return useVirtualMemory;
}

std::shared_ptr<Process> ProcessManager::createProcess(const std::string& name) {
    static int processIdCounter = 1000; // Start with higher IDs for manually created processes
    
    auto process = std::make_shared<Process>(name, processIdCounter++);
    
    // Generate default instructions (4000 instructions as per test config)
    std::vector<std::string> instructions;
    for (int i = 0; i < 4000; ++i) {
        instructions.push_back("PRINT(\"Line " + std::to_string(i+1) + " from " + name + "\")");
    }
    process->setInstructions(instructions);
    
    // Allocate memory using current memory manager
    if (memoryManager && memoryManager->allocateMemory(process)) {
        std::lock_guard<std::mutex> lock(processMutex);
        processes.push_back(process);
        
        // Add to scheduler
        if (scheduler) {
            scheduler->addProcess(process);
        }
        
        return process;
    }
    
    return nullptr; // Memory allocation failed
}

std::shared_ptr<Process> ProcessManager::createProcessWithMemory(const std::string& name, size_t memorySize, const std::vector<std::string>& instructions) {
    static int processIdCounter = 1000; // Start with higher IDs for manually created processes
    
    auto process = std::make_shared<Process>(name, processIdCounter++);
    
    if (useVirtualMemory && vmManager) {
        // Phase 2: Use virtual memory
        process->setVirtualMemorySize(memorySize);
        
        if (!vmManager->allocateVirtualMemory(process->getProcessId(), memorySize)) {
            std::cerr << "Failed to allocate virtual memory for process " << name << std::endl;
            return nullptr;
        }
        
        std::cout << "Created process " << name << " with " << memorySize << " bytes virtual memory" << std::endl;
    } else {
        // Phase 1: Use existing memory manager
        process->setMemorySize(memorySize);
        if (!memoryManager->allocateMemory(process)) {
            std::cerr << "Failed to allocate memory for process " << name << std::endl;
            return nullptr;
        }
    }
    
    // Set custom instructions if provided
    if (!instructions.empty()) {
        process->setInstructions(instructions);
    }
    
    // Add to process list
    {
        std::lock_guard<std::mutex> lock(processMutex);
        processes.push_back(process);
    }
    
    // Add to scheduler for execution and ensure scheduler is running
    if (scheduler) {
        if (!scheduler->isRunning()) {
            scheduler->start();
        }
        scheduler->addProcess(process);
    }
    
    return process;
}

uint16_t ProcessManager::readProcessMemory(int processId, uint32_t virtualAddr) {
    if (useVirtualMemory && vmManager) {
        try {
            return vmManager->readMemory(processId, virtualAddr);
        } catch (const PageFaultException& e) {
            std::cout << "Page fault handled for process " << processId 
                      << " at address 0x" << std::hex << virtualAddr << std::dec << std::endl;
            // Page fault was handled by VirtualMemoryManager, retry
            return vmManager->readMemory(processId, virtualAddr);
        }
    } else {
        // Phase 1: Direct memory access (simplified)
        std::cerr << "Memory read not supported in Phase 1 mode" << std::endl;
        return 0;
    }
}

void ProcessManager::writeProcessMemory(int processId, uint32_t virtualAddr, uint16_t value) {
    if (useVirtualMemory && vmManager) {
        try {
            vmManager->writeMemory(processId, virtualAddr, value);
        } catch (const PageFaultException& e) {
            std::cout << "Page fault handled for process " << processId 
                      << " at address 0x" << std::hex << virtualAddr << std::dec << std::endl;
            // Page fault was handled by VirtualMemoryManager, retry
            vmManager->writeMemory(processId, virtualAddr, value);
        }
    } else {
        // Phase 1: Direct memory access (simplified)
        std::cerr << "Memory write not supported in Phase 1 mode" << std::endl;
    }
}

ProcessManager::DetailedStats ProcessManager::getDetailedStats() const {
    DetailedStats stats = {};
    
    // Get basic CPU stats
    stats.cpuUtilization = getCpuUtilization();
    
    {
        std::lock_guard<std::mutex> lock(processMutex);
        stats.totalProcessCount = processes.size();
        stats.runningProcessCount = 0;
        
        for (const auto& process : processes) {
            if (process->getIsActive()) {
                stats.runningProcessCount++;
            }
        }
    }
    
    if (useVirtualMemory && vmManager) {
        // Phase 2: Get virtual memory stats
        auto vmStats = vmManager->getMemoryStats();
        stats.totalMemory = vmStats.totalMemory;
        stats.usedMemory = vmStats.usedMemory;
        stats.freeMemory = vmStats.freeMemory;
        stats.pagesIn = vmStats.pagesIn;
        stats.pagesOut = vmStats.pagesOut;
        stats.pageFaults = vmStats.pageFaults;
    } else {
        // Phase 1: Get basic memory stats
        if (memoryManager && storedConfig) {
            stats.totalMemory = storedConfig->maxOverallMem;
            int processesInMemory = memoryManager->getProcessesInMemory();
            int memPerProc = (storedConfig->minMemPerProc > 0) ? storedConfig->minMemPerProc : storedConfig->memPerProc;
            stats.usedMemory = processesInMemory * memPerProc;
            stats.freeMemory = stats.totalMemory - stats.usedMemory;
        }
        stats.pagesIn = 0;
        stats.pagesOut = 0;
        stats.pageFaults = 0;
    }
    
    // CPU tick estimation (simplified)
    stats.totalCpuTicks = currentQuantumCycle * numCores;
    stats.activeCpuTicks = static_cast<uint64_t>(stats.totalCpuTicks * (stats.cpuUtilization / 100.0));
    stats.idleCpuTicks = stats.totalCpuTicks - stats.activeCpuTicks;
    
    return stats;
}
