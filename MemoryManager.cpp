#include "MemoryManager.h"
#include "Process.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

MemoryManager::MemoryManager(int totalMem, int memPerProc, int memPerFrame, const std::string& policy)
    : totalMemory(totalMem), memoryPerProcess(memPerProc), memoryPerFrame(memPerFrame), 
      fitPolicy(policy), currentQuantum(0) {
    initialize();
}

MemoryManager::~MemoryManager() {
    // Clean-up if needed
}

void MemoryManager::initialize() {
    std::lock_guard<std::mutex> lock(memoryMutex);
    
    // Check memory parameters for sanity
    if (totalMemory <= 0) {
        std::cerr << "Error: Invalid total memory size: " << totalMemory << std::endl;
        totalMemory = 16384; // Default to 16KB if invalid
    }
    
    if (memoryPerProcess <= 0 || memoryPerProcess > totalMemory) {
        std::cerr << "Error: Invalid memory per process: " << memoryPerProcess << std::endl;
        memoryPerProcess = std::min(4096, totalMemory); // Default to 4KB or totalMemory if smaller
    }
    
    try {
        memoryBlocks.clear();
        // Start with one large free block representing all available memory
        memoryBlocks.emplace_back(0, totalMemory);
        // Removed verbose initialization log
    } catch (const std::exception& e) {
        std::cerr << "Exception in initialize: " << e.what() << std::endl;
    }
}

bool MemoryManager::allocateMemory(std::shared_ptr<Process> process) {
    if (!process) {
        std::cerr << "Error: Null process passed to allocateMemory" << std::endl;
        return false;
    }
    
    // Validate memory parameters before locking
    if (memoryPerProcess <= 0) {
        std::cerr << "Error: Invalid memory per process: " << memoryPerProcess << std::endl;
        return false;
    }
    
    if (totalMemory <= 0 || memoryPerProcess > totalMemory) {
        std::cerr << "Error: Invalid memory configuration - Total: " << totalMemory 
                  << ", Per Process: " << memoryPerProcess << std::endl;
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        // Removed debug output about memory allocation attempt
        
        // Safety check for empty vector
        if (memoryBlocks.empty()) {
            // Initialize with a default block if empty
            try {
                memoryBlocks.emplace_back(0, totalMemory);
            } catch (const std::exception& e) {
                std::cerr << "Exception during memory initialization: " << e.what() << std::endl;
                return false;
            }
        }
        
        // Debug block logging removed for less verbosity
        // Only log in case of errors
        
        int requiredSize = memoryPerProcess;
        int processId = process->getProcessId();
        std::string processName = process->getName();
        
        // First-fit algorithm: find the first free block that's big enough
        for (auto& block : memoryBlocks) {
            if (block.isFree && block.size >= requiredSize) {
                // Found a block that can fit the process
                
                if (block.size == requiredSize) {
                    // Perfect fit - just mark the block as allocated
                    block.isFree = false;
                    block.processName = processName;
                    block.processId = processId;
                } else {
                    // Split the block
                    int remainingSize = block.size - requiredSize;
                    block.size = requiredSize;
                    block.isFree = false;
                    block.processName = processName;
                    block.processId = processId;
                    
                    // Create a new block for the remaining space
                    MemoryBlock newBlock(block.startAddress + requiredSize, remainingSize);
                    
                    // Make sure we don't exceed vector capacity
                    if (memoryBlocks.size() >= memoryBlocks.max_size() - 1) {
                        std::cerr << "Warning: Vector capacity limit reached, not splitting block" << std::endl;
                        // Can't add more blocks, just mark the whole block as used
                        return true;
                    }
                    
                    // Find the index of the current block to avoid iterator invalidation
                    size_t blockIndex = 0;
                    for (size_t i = 0; i < memoryBlocks.size(); ++i) {
                        if (memoryBlocks[i].startAddress == block.startAddress) {
                            blockIndex = i;
                            break;
                        }
                    }
                    
                    // Insert the new block after the current one using the index
                    // Fix: blockIndex + 1 <= memoryBlocks.size() should be <= not <
                    if (blockIndex + 1 < memoryBlocks.size()) {
                        try {
                            memoryBlocks.insert(memoryBlocks.begin() + blockIndex + 1, newBlock);
                        } catch (const std::exception& e) {
                            std::cerr << "Exception while inserting new block: " << e.what() << std::endl;
                        }
                    } else {
                        // Push back if it's at the end
                        try {
                            memoryBlocks.push_back(newBlock);
                        } catch (const std::exception& e) {
                            std::cerr << "Exception while pushing back new block: " << e.what() << std::endl;
                        }
                    }
                }
                return true;
            }
        }
        
        // No suitable block found
        // std::cout << "No suitable free block found for process " << process->getName() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception in allocateMemory: " << e.what() << std::endl;
        return false;
    }
}

void MemoryManager::deallocateMemory(int processId) {
    // Validate input
    if (processId < 0) {
        std::cerr << "Warning: Invalid process ID in deallocateMemory: " << processId << std::endl;
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        // Safety check for empty vector
        if (memoryBlocks.empty()) {
            return;
        }
        
        // Find all blocks allocated to this process and free them
        int blocksFreed = 0;
        for (auto& block : memoryBlocks) {
            if (!block.isFree && block.processId == processId) {
                block.isFree = true;
                block.processName = "";
                block.processId = -1;
                blocksFreed++;
            }
        }
        
        // Removed verbose logging for memory deallocation
        
        // Merge adjacent free blocks - safer implementation
        if (memoryBlocks.size() > 1) { // Only attempt to merge if there's more than one block
            bool merged;
            do {
                merged = false;
                // Use indices instead of iterators to avoid iterator invalidation issues
                for (size_t i = 0; i < memoryBlocks.size() - 1; ++i) {
                    if (memoryBlocks[i].isFree && memoryBlocks[i+1].isFree) {
                        // Merge the blocks
                        memoryBlocks[i].size += memoryBlocks[i+1].size;
                        // Remove the next block
                        memoryBlocks.erase(memoryBlocks.begin() + i + 1);
                        merged = true;
                        break; // Start over to avoid index invalidation
                    }
                }
            } while (merged && memoryBlocks.size() > 1);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in deallocateMemory: " << e.what() << std::endl;
    }
}

bool MemoryManager::hasAvailableMemory(int size) const {
    if (size <= 0) {
        std::cerr << "Warning: Invalid size requested in hasAvailableMemory: " << size << std::endl;
        return false;
    }
    
    if (size > totalMemory) {
        // Quickly return false if the requested size is larger than total memory
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        // Safety check for empty vector
        if (memoryBlocks.empty()) {
            // If the vector is empty, initialize it
            const_cast<MemoryManager*>(this)->initialize();
        }
        
        // Check if any free block is large enough
        for (const auto& block : memoryBlocks) {
            if (block.isFree && block.size >= size) {
                return true;
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception in hasAvailableMemory: " << e.what() << std::endl;
        return false;
    }
}

int MemoryManager::calculateExternalFragmentation() const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    
    int totalFreeMemory = 0;
    int largestFreeBlock = 0;
    
    for (const auto& block : memoryBlocks) {
        if (block.isFree) {
            totalFreeMemory += block.size;
            largestFreeBlock = std::max(largestFreeBlock, block.size);
        }
    }
    
    // External fragmentation is the memory that can't be used for process allocation
    // because it's scattered in small fragments
    if (totalFreeMemory < memoryPerProcess) {
        // All free memory is external fragmentation if it can't hold a process
        return totalFreeMemory;
    }
    
    // Calculate total unusable memory (fragments smaller than memoryPerProcess)
    int unusableMemory = 0;
    for (const auto& block : memoryBlocks) {
        if (block.isFree && block.size < memoryPerProcess) {
            unusableMemory += block.size;
        }
    }
    
    return unusableMemory;
}

void MemoryManager::generateMemorySnapshot(int quantumCycle) {
    currentQuantum = quantumCycle;
    
    // Create memory_stamps directory if it doesn't exist
    std::string dir = "memory_stamps";
    std::system(("mkdir -p " + dir).c_str());
    
    // Create filename with directory path
    std::stringstream filename;
    filename << dir << "/memory_stamp_" << std::setw(2) << std::setfill('0') << quantumCycle << ".txt";
    
    std::ofstream file(filename.str());
    if (!file) {
        std::cerr << "Failed to create memory snapshot file: " << filename.str() << std::endl;
        return;
    }
    
    // Write memory snapshot information
    file << "Timestamp: " << getCurrentTimestamp() << std::endl;
    file << "Number of processes in memory: " << getProcessesInMemory() << std::endl;
    
    int externalFrag = calculateExternalFragmentation();
    file << "Total external fragmentation in KB: " << externalFrag / 1024 << std::endl;
    
    // Add ASCII memory representation
    file << std::endl << generateASCIIPrintout() << std::endl;
    
    file.close();
}

int MemoryManager::getProcessesInMemory() const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    
    if (memoryBlocks.empty()) {
        return 0;
    }
    
    // Count unique process IDs in allocated blocks
    std::set<int> uniqueProcessIds;
    for (const auto& block : memoryBlocks) {
        if (!block.isFree && block.processId >= 0) {
            uniqueProcessIds.insert(block.processId);
        }
    }
    
    return uniqueProcessIds.size();
}

void MemoryManager::setCurrentQuantum(int quantum) {
    currentQuantum = quantum;
}

std::string MemoryManager::getMemoryASCIIRepresentation() const {
    return generateASCIIPrintout();
}

std::pair<int, int> MemoryManager::getProcessMemoryMap(int processId) const {
    std::lock_guard<std::mutex> lock(memoryMutex);
    
    int startAddress = -1;
    int endAddress = -1;
    
    // Safety check for valid processId
    if (processId < 0) {
        std::cout << "Warning: Invalid process ID in getProcessMemoryMap: " << processId << std::endl;
        return {-1, -1};
    }
    
    // Safety check for empty vector
    if (memoryBlocks.empty()) {
        std::cout << "Warning: Memory blocks vector is empty in getProcessMemoryMap" << std::endl;
        return {-1, -1};
    }
    
    try {
        for (const auto& block : memoryBlocks) {
            if (!block.isFree && block.processId == processId) {
                // Found a block allocated to this process
                if (startAddress == -1 || block.startAddress < startAddress) {
                    startAddress = block.startAddress;
                }
                
                int blockEnd = block.startAddress + block.size;
                if (blockEnd > endAddress) {
                    endAddress = blockEnd;
                }
            }
        }
        
        if (startAddress == -1 || endAddress == -1) {
            std::cout << "Warning: No memory blocks found for process ID: " << processId << std::endl;
        } 
        // else 
        // {
        //     std::cout << "Memory map for process ID " << processId 
        //               << ": start=" << startAddress << ", end=" << endAddress << std::endl;
        // }
    } catch (const std::exception& e) {
        std::cerr << "Exception in getProcessMemoryMap: " << e.what() << std::endl;
        return {-1, -1};
    }
    
    return {startAddress, endAddress};
}

std::string MemoryManager::generateASCIIPrintout() const {
    std::stringstream ss;
    try {
        std::lock_guard<std::mutex> lock(memoryMutex); // Added lock for thread safety
        
        // Memory end marker
        ss << "----end---- = " << totalMemory << std::endl << std::endl;
        
        // Check if we have any memory blocks
        if (memoryBlocks.empty()) {
            // If no blocks, just print start marker
            ss << "----start---- = 0" << std::endl;
            return ss.str();
        }
        
        // Create a copy of memory blocks for safety
    std::vector<MemoryBlock> sortedBlocks;
    try {
        sortedBlocks = memoryBlocks; // Make a copy
        
        // Sort blocks by address in descending order for top-down visualization
        std::sort(sortedBlocks.begin(), sortedBlocks.end(), 
              [](const MemoryBlock& a, const MemoryBlock& b) {
                  return a.startAddress > b.startAddress;
              });
    } catch (const std::exception& e) {
        // If any exception occurs, return a basic representation
        ss << "Error generating memory printout: " << e.what() << std::endl;
        ss << "----start---- = 0" << std::endl;
        return ss.str();
    }        // Visualize memory from top (high address) to bottom (low address)
        int currentAddress = totalMemory;
        
        for (const auto& block : sortedBlocks) {
            // Safe bounds checking
            if (block.startAddress < 0 || block.startAddress >= totalMemory ||
                block.size <= 0 || block.startAddress + block.size > totalMemory) {
                ss << "Warning: Invalid block (address=" << block.startAddress 
                   << ", size=" << block.size << ")" << std::endl;
                continue;
            }
            
            // Fill any gap between blocks
            if (block.startAddress + block.size < currentAddress) {
                ss << currentAddress << std::endl;
                currentAddress = block.startAddress + block.size;
            }
            
            if (!block.isFree) {
                // Show allocated block
                ss << currentAddress << std::endl;
                ss << (block.processName.empty() ? "unnamed" : block.processName) << std::endl;
                ss << block.startAddress << std::endl << std::endl;
            }
            
            currentAddress = block.startAddress;
        }
        
        // Memory start marker
        ss << "----start---- = 0" << std::endl;
        
    } catch (const std::exception& e) {
        ss << "Error generating ASCII printout: " << e.what() << std::endl;
    }
    
    return ss.str();
}

std::string MemoryManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm timeInfo;
    
    #ifdef _WIN32
    localtime_s(&timeInfo, &time);
    #else
    localtime_r(&time, &timeInfo);
    #endif
    
    std::stringstream ss;
    
    // Convert 24-hour format to 12-hour format and determine AM/PM
    int hour12 = timeInfo.tm_hour % 12;
    if (hour12 == 0) hour12 = 12;  // 0 hour in 12-hour format is 12
    std::string ampm = (timeInfo.tm_hour >= 12) ? "PM" : "AM";
    
    ss << "(" << std::setfill('0') << std::setw(2) << (timeInfo.tm_mon + 1) << "/"
       << std::setfill('0') << std::setw(2) << timeInfo.tm_mday << "/"
       << (timeInfo.tm_year + 1900) << " "
       << std::setfill('0') << std::setw(2) << hour12 << ":"
       << std::setfill('0') << std::setw(2) << timeInfo.tm_min << ":"
       << std::setfill('0') << std::setw(2) << timeInfo.tm_sec << ampm << ")";
    
    return ss.str();
}
