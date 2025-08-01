#include "VirtualMemoryManager.h"
#include "Process.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cstring>

// FrameAllocator Implementation
FrameAllocator::FrameAllocator(int totalMemory, int frameSize) 
    : frameSize(frameSize), totalFrames(totalMemory / frameSize) {
    frameStatus.resize(totalFrames, false);
    
    // Initialize free frames queue
    for (int i = 0; i < totalFrames; ++i) {
        freeFrames.push(i);
    }
}

int FrameAllocator::allocateFrame() {
    std::lock_guard<std::mutex> lock(frameMutex);
    
    if (freeFrames.empty()) {
        return -1;  // No free frames available
    }
    
    int frameNumber = freeFrames.front();
    freeFrames.pop();
    frameStatus[frameNumber] = true;
    
    return frameNumber;
}

void FrameAllocator::deallocateFrame(int frameNumber) {
    std::lock_guard<std::mutex> lock(frameMutex);
    
    if (frameNumber >= 0 && frameNumber < totalFrames && frameStatus[frameNumber]) {
        frameStatus[frameNumber] = false;
        freeFrames.push(frameNumber);
    }
}

bool FrameAllocator::hasAvailableFrames() const {
    std::lock_guard<std::mutex> lock(frameMutex);
    return !freeFrames.empty();
}

int FrameAllocator::getAvailableFrameCount() const {
    std::lock_guard<std::mutex> lock(frameMutex);
    return freeFrames.size();
}

// BackingStore Implementation
BackingStore::BackingStore(const std::string& filename, int blockSize) 
    : filename(filename), blockSize(blockSize), nextFreeBlock(0) {
    initialize();
}

BackingStore::~BackingStore() {
    if (backingFile.is_open()) {
        backingFile.close();
    }
}

void BackingStore::initialize() {
    std::lock_guard<std::mutex> lock(fileMutex);
    
    backingFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    
    if (!backingFile.is_open()) {
        // Create new file if it doesn't exist
        backingFile.open(filename, std::ios::binary | std::ios::out);
        backingFile.close();
        backingFile.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    
    if (!backingFile.is_open()) {
        throw std::runtime_error("Failed to open backing store file: " + filename);
    }
}

void BackingStore::storePage(int frameNumber, uint32_t diskAddress, const uint8_t* pageData) {
    std::lock_guard<std::mutex> lock(fileMutex);
    
    backingFile.seekp(diskAddress);
    backingFile.write(reinterpret_cast<const char*>(pageData), blockSize);
    backingFile.flush();
}

void BackingStore::loadPage(uint32_t diskAddress, int frameNumber, uint8_t* pageData) {
    std::lock_guard<std::mutex> lock(fileMutex);
    
    backingFile.seekg(diskAddress);
    backingFile.read(reinterpret_cast<char*>(pageData), blockSize);
}

uint32_t BackingStore::allocateDiskSpace() {
    std::lock_guard<std::mutex> lock(fileMutex);
    
    uint32_t allocated = nextFreeBlock;
    nextFreeBlock += blockSize;
    allocatedBlocks[allocated] = true;
    
    return allocated;
}

void BackingStore::deallocateDiskSpace(uint32_t diskAddress) {
    std::lock_guard<std::mutex> lock(fileMutex);
    allocatedBlocks.erase(diskAddress);
}

// VirtualMemoryManager Implementation
VirtualMemoryManager::VirtualMemoryManager(int totalPhysicalMemory, int frameSize) {
    // Initialize physical memory
    physicalMemory.resize(totalPhysicalMemory, 0);
    
    // Initialize components
    frameAllocator = std::make_unique<FrameAllocator>(totalPhysicalMemory, frameSize);
    backingStore = std::make_unique<BackingStore>("csopesy-backing-store.txt", frameSize);
    
    std::cout << "Virtual Memory Manager initialized with " << totalPhysicalMemory 
              << " bytes physical memory, " << frameSize << " bytes per frame" << std::endl;
}

VirtualMemoryManager::~VirtualMemoryManager() {
    // Cleanup is handled by unique_ptr destructors
}

bool VirtualMemoryManager::allocateVirtualMemory(int processId, size_t size) {
    std::unique_lock<std::shared_mutex> lock(vmMutex);
    
    if (size < SYMBOL_TABLE_SIZE) {
        std::cerr << "Error: Process memory size must be at least " << SYMBOL_TABLE_SIZE << " bytes" << std::endl;
        return false;
    }
    
    // Check if process already has memory allocated
    if (processPageTables.find(processId) != processPageTables.end()) {
        std::cerr << "Error: Process " << processId << " already has memory allocated" << std::endl;
        return false;
    }
    
    // Calculate number of pages needed
    int pagesNeeded = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Initialize page table for this process
    processPageTables[processId].resize(pagesNeeded);
    processMemorySizes[processId] = size;
    processBaseAddresses[processId] = 0;  // Virtual addresses start at 0 for each process
    
    // Initialize pages (all start as not present - demand paging)
    for (int i = 0; i < pagesNeeded; ++i) {
        processPageTables[processId][i] = PageTableEntry();
        processPageTables[processId][i].diskAddress = backingStore->allocateDiskSpace();
    }
    
    std::cout << "Allocated " << size << " bytes (" << pagesNeeded << " pages) for process " 
              << processId << std::endl;
    
    return true;
}

void VirtualMemoryManager::deallocateVirtualMemory(int processId) {
    std::unique_lock<std::shared_mutex> lock(vmMutex);
    
    auto it = processPageTables.find(processId);
    if (it == processPageTables.end()) {
        return;  // Process not found
    }
    
    // Free all physical frames used by this process
    for (auto& entry : it->second) {
        if (entry.present) {
            frameAllocator->deallocateFrame(entry.frameNumber);
        }
        // Free backing store space
        backingStore->deallocateDiskSpace(entry.diskAddress);
    }
    
    // Remove from tracking maps
    processPageTables.erase(it);
    processMemorySizes.erase(processId);
    processBaseAddresses.erase(processId);
    
    std::cout << "Deallocated memory for process " << processId << std::endl;
}

bool VirtualMemoryManager::translateAddress(int processId, uint32_t virtualAddr, uint32_t& physicalAddr) {
    std::shared_lock<std::shared_mutex> lock(vmMutex);
    
    if (!isValidMemoryAccess(processId, virtualAddr)) {
        return false;
    }
    
    int pageNumber = getPageNumber(virtualAddr);
    int offset = getPageOffset(virtualAddr);
    
    auto& pageTable = processPageTables[processId];
    
    if (!pageTable[pageNumber].present) {
        // Page fault will be handled by caller
        return false;
    }
    
    // Update access time for LRU
    updateAccessTime(pageTable[pageNumber]);
    
    physicalAddr = getPhysicalAddress(pageTable[pageNumber].frameNumber, offset);
    return true;
}

uint16_t VirtualMemoryManager::readMemory(int processId, uint32_t virtualAddr) {
    uint32_t physicalAddr;
    
    // Try to translate address
    if (!translateAddress(processId, virtualAddr, physicalAddr)) {
        // Handle page fault
        handlePageFault(processId, virtualAddr);
        
        // Retry translation
        if (!translateAddress(processId, virtualAddr, physicalAddr)) {
            throw std::runtime_error("Failed to translate address after page fault handling");
        }
    }
    
    // Read from physical memory (assuming little-endian 16-bit values)
    uint16_t value = 0;
    if (physicalAddr + 1 < physicalMemory.size()) {
        value = physicalMemory[physicalAddr] | (physicalMemory[physicalAddr + 1] << 8);
    }
    
    return value;
}

void VirtualMemoryManager::writeMemory(int processId, uint32_t virtualAddr, uint16_t value) {
    uint32_t physicalAddr;
    
    // Try to translate address
    if (!translateAddress(processId, virtualAddr, physicalAddr)) {
        // Handle page fault
        handlePageFault(processId, virtualAddr);
        
        // Retry translation
        if (!translateAddress(processId, virtualAddr, physicalAddr)) {
            throw std::runtime_error("Failed to translate address after page fault handling");
        }
    }
    
    // Mark page as dirty
    {
        std::unique_lock<std::shared_mutex> lock(vmMutex);
        int pageNumber = getPageNumber(virtualAddr);
        processPageTables[processId][pageNumber].dirty = true;
    }
    
    // Write to physical memory (little-endian 16-bit values)
    if (physicalAddr + 1 < physicalMemory.size()) {
        physicalMemory[physicalAddr] = value & 0xFF;
        physicalMemory[physicalAddr + 1] = (value >> 8) & 0xFF;
    }
}

void VirtualMemoryManager::handlePageFault(int processId, uint32_t virtualAddr) {
    std::lock_guard<std::mutex> pageFaultLock(pageFaultMutex);
    std::unique_lock<std::shared_mutex> vmLock(vmMutex);
    
    int pageNumber = getPageNumber(virtualAddr);
    auto& pageTable = processPageTables[processId];
    
    if (pageTable[pageNumber].present) {
        return;  // Page was loaded by another thread
    }
    
    ++pageFaults;
    ++pagesIn;
    
    // Try to allocate a free frame
    int frameNumber = frameAllocator->allocateFrame();
    
    // If no free frames, evict a page
    if (frameNumber == -1) {
        int victimFrame = selectVictimPage();
        if (victimFrame != -1) {
            frameAllocator->deallocateFrame(victimFrame);
            frameNumber = frameAllocator->allocateFrame();
        }
    }
    
    if (frameNumber == -1) {
        throw std::runtime_error("Unable to allocate frame for page fault");
    }
    
    // Load page from backing store
    uint8_t* pageData = &physicalMemory[frameNumber * PAGE_SIZE];
    backingStore->loadPage(pageTable[pageNumber].diskAddress, frameNumber, pageData);
    
    // Update page table entry
    pageTable[pageNumber].present = true;
    pageTable[pageNumber].frameNumber = frameNumber;
    pageTable[pageNumber].referenced = true;
    updateAccessTime(pageTable[pageNumber]);
    
    std::cout << "Page fault handled for process " << processId 
              << ", virtual address 0x" << std::hex << virtualAddr 
              << ", loaded into frame " << std::dec << frameNumber << std::endl;
}

bool VirtualMemoryManager::isValidMemoryAccess(int processId, uint32_t virtualAddr) const {
    auto it = processMemorySizes.find(processId);
    if (it == processMemorySizes.end()) {
        return false;
    }
    
    return virtualAddr < it->second;
}

VirtualMemoryManager::MemoryStats VirtualMemoryManager::getMemoryStats() const {
    std::shared_lock<std::shared_mutex> lock(vmMutex);
    
    MemoryStats stats;
    stats.pageFaults = pageFaults;
    stats.pagesIn = pagesIn;
    stats.pagesOut = pagesOut;
    stats.totalFrames = frameAllocator->getTotalFrames();
    stats.freeFrames = frameAllocator->getAvailableFrameCount();
    stats.usedFrames = stats.totalFrames - stats.freeFrames;
    stats.totalMemory = physicalMemory.size();
    stats.usedMemory = stats.usedFrames * PAGE_SIZE;
    stats.freeMemory = stats.totalMemory - stats.usedMemory;
    
    return stats;
}

// Private helper methods
int VirtualMemoryManager::getPageNumber(uint32_t virtualAddr) const {
    return virtualAddr / PAGE_SIZE;
}

int VirtualMemoryManager::getPageOffset(uint32_t virtualAddr) const {
    return virtualAddr % PAGE_SIZE;
}

uint32_t VirtualMemoryManager::getPhysicalAddress(int frameNumber, int offset) const {
    return frameNumber * PAGE_SIZE + offset;
}

int VirtualMemoryManager::selectVictimPage() {
    // Simple LRU implementation - find page with oldest access time
    uint64_t oldestTime = UINT64_MAX;
    int victimProcessId = -1;
    int victimPageNumber = -1;
    int victimFrame = -1;
    
    for (const auto& [processId, pageTable] : processPageTables) {
        for (int i = 0; i < pageTable.size(); ++i) {
            if (pageTable[i].present && pageTable[i].lastAccessTime < oldestTime) {
                oldestTime = pageTable[i].lastAccessTime;
                victimProcessId = processId;
                victimPageNumber = i;
                victimFrame = pageTable[i].frameNumber;
            }
        }
    }
    
    if (victimProcessId != -1) {
        evictPage(victimProcessId, victimPageNumber);
        return victimFrame;
    }
    
    return -1;
}

void VirtualMemoryManager::evictPage(int processId, int pageNumber) {
    auto& entry = processPageTables[processId][pageNumber];
    
    // If page is dirty, write to backing store
    if (entry.dirty) {
        uint8_t* pageData = &physicalMemory[entry.frameNumber * PAGE_SIZE];
        backingStore->storePage(entry.frameNumber, entry.diskAddress, pageData);
        ++pagesOut;
    }
    
    // Mark as not present
    entry.present = false;
    entry.dirty = false;
    entry.referenced = false;
    
    std::cout << "Evicted page " << pageNumber << " from process " << processId << std::endl;
}

void VirtualMemoryManager::updateAccessTime(PageTableEntry& entry) const {
    entry.lastAccessTime = ++accessCounter;
    entry.referenced = true;
}

size_t VirtualMemoryManager::getProcessMemorySize(int processId) const {
    std::shared_lock<std::shared_mutex> lock(vmMutex);
    auto it = processMemorySizes.find(processId);
    return it != processMemorySizes.end() ? it->second : 0;
}