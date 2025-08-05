#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <cstdint>
#include <queue>
#include <fstream>

// Forward declarations
class Process;
class FrameAllocator;
class BackingStore;

// Page fault exception for handling memory access violations
class PageFaultException : public std::runtime_error {
private:
    uint32_t faultAddress;
    int processId;
    
public:
    PageFaultException(int pid, uint32_t addr, const std::string& msg)
        : std::runtime_error(msg), processId(pid), faultAddress(addr) {}
    
    uint32_t getAddress() const { return faultAddress; }
    int getProcessId() const { return processId; }
};

// Page table entry structure
struct PageTableEntry {
    bool present = false;        // Page is in physical memory
    bool dirty = false;          // Page has been modified
    bool referenced = false;     // Page has been accessed (for LRU)
    uint32_t frameNumber = 0;    // Physical frame number
    uint32_t diskAddress = 0;    // Address in backing store
    
    // For LRU tracking
    mutable uint64_t lastAccessTime = 0;
    
    PageTableEntry() = default;
    PageTableEntry(bool p, uint32_t frame) : present(p), frameNumber(frame) {}
};

// Frame allocator for managing physical memory frames
class FrameAllocator {
private:
    std::vector<bool> frameStatus;  // true = allocated, false = free
    std::queue<int> freeFrames;
    mutable std::mutex frameMutex;
    int totalFrames;
    int frameSize;
    
public:
    FrameAllocator(int totalMemory, int frameSize);
    ~FrameAllocator() = default;
    
    int allocateFrame();
    void deallocateFrame(int frameNumber);
    bool hasAvailableFrames() const;
    int getAvailableFrameCount() const;
    int getTotalFrames() const { return totalFrames; }
    int getFrameSize() const { return frameSize; }
};

// Backing store for swapped-out pages
class BackingStore {
private:
    std::fstream backingFile;
    mutable std::mutex fileMutex;
    std::string filename;
    std::map<uint32_t, bool> allocatedBlocks;  // Track allocated disk space
    uint32_t nextFreeBlock;
    int blockSize;
    
public:
    BackingStore(const std::string& filename, int blockSize);
    ~BackingStore();
    
    void storePage(int frameNumber, uint32_t diskAddress, const uint8_t* pageData);
    void loadPage(uint32_t diskAddress, int frameNumber, uint8_t* pageData);
    uint32_t allocateDiskSpace();
    void deallocateDiskSpace(uint32_t diskAddress);
    void initialize();
};

// Main Virtual Memory Manager
class VirtualMemoryManager {
private:
    int PAGE_SIZE;                                 // Dynamic page size from config
    static constexpr int SYMBOL_TABLE_SIZE = 64;   // 64 bytes for variables
    
    // Page tables for each process (processId -> page table)
    std::map<int, std::vector<PageTableEntry>> processPageTables;
    
    // Physical memory representation (frame-based)
    std::vector<uint8_t> physicalMemory;
    
    // Components
    std::unique_ptr<FrameAllocator> frameAllocator;
    std::unique_ptr<BackingStore> backingStore;
    
    // Synchronization
    mutable std::shared_mutex vmMutex;  // Allows multiple readers for address translation
    mutable std::mutex pageFaultMutex;  // Exclusive for page fault handling
    
    // Statistics
    uint64_t pageFaults = 0;
    uint64_t pagesIn = 0;
    uint64_t pagesOut = 0;
    mutable uint64_t accessCounter = 0;  // For LRU tracking
    
    // Process memory tracking
    std::map<int, size_t> processMemorySizes;
    std::map<int, uint32_t> processBaseAddresses;
    
public:
    VirtualMemoryManager(int totalPhysicalMemory = 16384, int frameSize = 16);
    ~VirtualMemoryManager();
    
    // Core virtual memory operations
    bool allocateVirtualMemory(int processId, size_t size);
    void deallocateVirtualMemory(int processId);
    
    // Address translation and memory access
    bool translateAddress(int processId, uint32_t virtualAddr, uint32_t& physicalAddr);
    uint16_t readMemory(int processId, uint32_t virtualAddr);
    void writeMemory(int processId, uint32_t virtualAddr, uint16_t value);
    
    // Page fault handling
    void handlePageFault(int processId, uint32_t virtualAddr);
    
    // Memory validation
    bool isValidMemoryAccess(int processId, uint32_t virtualAddr) const;
    
    // Statistics and monitoring
    struct MemoryStats {
        uint64_t pageFaults;
        uint64_t pagesIn;
        uint64_t pagesOut;
        int totalFrames;
        int freeFrames;
        int usedFrames;
        size_t totalMemory;
        size_t usedMemory;
        size_t freeMemory;
    };
    
    MemoryStats getMemoryStats() const;
    void resetStats();
    
    // Process memory information
    size_t getProcessMemorySize(int processId) const;
    std::vector<int> getProcessesInMemory() const;
    
    // For debugging and visualization
    std::string getMemoryMapString(int processId) const;
    void printPageTable(int processId) const;
    
private:
    // Internal helper methods
    int getPageNumber(uint32_t virtualAddr) const;
    int getPageOffset(uint32_t virtualAddr) const;
    uint32_t getPhysicalAddress(int frameNumber, int offset) const;
    
    // Page replacement (LRU)
    int selectVictimPage();
    void evictPage(int victimProcessId, int victimPageNumber);
    void updateAccessTime(PageTableEntry& entry) const;
    
    // Page loading and storing
    void loadPageFromDisk(int processId, int pageNumber);
    void storePageToDisk(int processId, int pageNumber);
    
    // Memory initialization
    void initializeProcessPages(int processId, size_t size);
    void cleanupProcessPages(int processId);
    
    // Validation helpers
    bool isValidProcessId(int processId) const;
    bool isValidPageNumber(int processId, int pageNumber) const;
};

#endif