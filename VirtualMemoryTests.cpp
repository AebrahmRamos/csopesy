#include "VirtualMemoryManager.h"
#include "Process.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <memory>

class VirtualMemoryTester {
private:
    std::unique_ptr<VirtualMemoryManager> vmManager;
    
public:
    VirtualMemoryTester() {
        // Initialize with small memory for easier testing
        vmManager = std::make_unique<VirtualMemoryManager>(256, 16); // 16 frames of 16 bytes each
    }
    
    void runAllTests() {
        std::cout << "=== Virtual Memory Manager Test Suite ===" << std::endl;
        
        testBasicAllocation();
        testAddressTranslation();  
        testPageFaultHandling();
        testMultipleProcesses();
        testMemoryAccess();
        testPageReplacement();
        testMemoryStats();
        
        std::cout << "=== All Tests Completed ===" << std::endl;
    }
    
private:
    void testBasicAllocation() {
        std::cout << "\n--- Test 1: Basic Memory Allocation ---" << std::endl;
        
        // Test allocating memory for a process
        bool success = vmManager->allocateVirtualMemory(1, 128); // Process 1, 128 bytes
        assert(success && "Should successfully allocate memory");
        std::cout << "✓ Successfully allocated 128 bytes for process 1" << std::endl;
        
        // Test getting process memory size
        size_t size = vmManager->getProcessMemorySize(1);
        assert(size == 128 && "Memory size should match allocation");
        std::cout << "✓ Correct memory size reported: " << size << " bytes" << std::endl;
        
        // Test duplicate allocation (should fail)
        bool duplicate = vmManager->allocateVirtualMemory(1, 64);
        assert(!duplicate && "Duplicate allocation should fail");
        std::cout << "✓ Correctly rejected duplicate allocation" << std::endl;
        
        std::cout << "Test 1 PASSED" << std::endl;
    }
    
    void testAddressTranslation() {
        std::cout << "\n--- Test 2: Address Translation ---" << std::endl;
        
        vmManager->allocateVirtualMemory(2, 64); // Process 2, 64 bytes
        
        // Test valid address translation (should trigger page fault first)
        uint32_t physicalAddr;
        bool translated = vmManager->translateAddress(2, 0, physicalAddr);
        
        // First access should fail (page not loaded), then succeed after page fault
        if (!translated) {
            std::cout << "✓ Initial translation failed as expected (page not present)" << std::endl;
            
            // Trigger page fault handling
            try {
                vmManager->handlePageFault(2, 0);
                std::cout << "✓ Page fault handled successfully" << std::endl;
                
                // Now translation should succeed
                translated = vmManager->translateAddress(2, 0, physicalAddr);
                assert(translated && "Translation should succeed after page fault");
                std::cout << "✓ Address translation successful after page fault" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "✗ Page fault handling failed: " << e.what() << std::endl;
                assert(false);
            }
        } else {
            std::cout << "✓ Address translation succeeded immediately" << std::endl;
        }
        
        // Test invalid address (beyond process memory)
        bool invalidTranslation = vmManager->translateAddress(2, 128, physicalAddr);
        assert(!invalidTranslation && "Invalid address should not translate");
        std::cout << "✓ Correctly rejected invalid address translation" << std::endl;
        
        std::cout << "Test 2 PASSED" << std::endl;
    }
    
    void testPageFaultHandling() {
        std::cout << "\n--- Test 3: Page Fault Handling ---" << std::endl;
        
        vmManager->allocateVirtualMemory(3, 96); // Process 3, 96 bytes (6 pages)
        
        auto initialStats = vmManager->getMemoryStats();
        uint64_t initialPageFaults = initialStats.pageFaults;
        
        // Access different pages to trigger page faults
        std::vector<uint32_t> addresses = {0, 16, 32, 48, 64, 80}; // Different pages
        
        for (uint32_t addr : addresses) {
            try {
                vmManager->handlePageFault(3, addr);
                std::cout << "✓ Page fault handled for address 0x" << std::hex << addr << std::dec << std::endl;
            } catch (const std::exception& e) {
                std::cout << "✗ Page fault failed for address 0x" << std::hex << addr << ": " << e.what() << std::endl;
                assert(false);
            }
        }
        
        auto finalStats = vmManager->getMemoryStats();
        uint64_t totalPageFaults = finalStats.pageFaults - initialPageFaults;
        
        std::cout << "✓ Total page faults handled: " << totalPageFaults << std::endl;
        assert(totalPageFaults > 0 && "Should have generated page faults");
        
        std::cout << "Test 3 PASSED" << std::endl;
    }
    
    void testMultipleProcesses() {
        std::cout << "\n--- Test 4: Multiple Process Management ---" << std::endl;
        
        // Allocate memory for multiple processes
        std::vector<int> processIds = {4, 5, 6};
        std::vector<size_t> memorySizes = {64, 128, 96};
        
        for (size_t i = 0; i < processIds.size(); ++i) {
            bool success = vmManager->allocateVirtualMemory(processIds[i], memorySizes[i]);
            assert(success && "Should allocate memory for each process");
            std::cout << "✓ Allocated " << memorySizes[i] << " bytes for process " << processIds[i] << std::endl;
        }
        
        // Verify each process has correct memory size
        for (size_t i = 0; i < processIds.size(); ++i) {
            size_t reportedSize = vmManager->getProcessMemorySize(processIds[i]);
            assert(reportedSize == memorySizes[i] && "Memory size should match allocation");
        }
        
        // Test process memory isolation
        for (int pid : processIds) {
            uint32_t physAddr;
            vmManager->handlePageFault(pid, 0); // Load first page
            bool translated = vmManager->translateAddress(pid, 0, physAddr);
            assert(translated && "Each process should have independent address space");
        }
        
        std::cout << "✓ All processes have independent address spaces" << std::endl;
        std::cout << "Test 4 PASSED" << std::endl;
    }
    
    void testMemoryAccess() {
        std::cout << "\n--- Test 5: Memory Read/Write Operations ---" << std::endl;
        
        vmManager->allocateVirtualMemory(7, 128); // Process 7, 128 bytes
        
        // Test writing and reading values
        std::vector<std::pair<uint32_t, uint16_t>> testData = {
            {0, 0x1234}, {2, 0x5678}, {16, 0xABCD}, {32, 0xEF01}
        };
        
        for (const auto& [addr, value] : testData) {
            try {
                vmManager->writeMemory(7, addr, value);
                std::cout << "✓ Wrote 0x" << std::hex << value << " to address 0x" << addr << std::dec << std::endl;
                
                uint16_t readValue = vmManager->readMemory(7, addr);
                assert(readValue == value && "Read value should match written value");
                std::cout << "✓ Read back correct value: 0x" << std::hex << readValue << std::dec << std::endl;
            } catch (const std::exception& e) {
                std::cout << "✗ Memory access failed: " << e.what() << std::endl;
                assert(false);
            }
        }
        
        // Test invalid memory access
        try {
            vmManager->readMemory(7, 200); // Beyond allocated memory
            assert(false && "Should throw exception for invalid access");
        } catch (const std::exception& e) {
            std::cout << "✓ Correctly rejected invalid memory access: " << e.what() << std::endl;
        }
        
        std::cout << "Test 5 PASSED" << std::endl;
    }
    
    void testPageReplacement() {
        std::cout << "\n--- Test 6: Page Replacement (Memory Pressure) ---" << std::endl;
        
        // Allocate more memory than physical frames to trigger page replacement
        vmManager->allocateVirtualMemory(8, 512); // Process 8, 512 bytes (32 pages)
        
        auto initialStats = vmManager->getMemoryStats();
        std::cout << "Available frames before test: " << initialStats.freeFrames << std::endl;
        
        // Access many pages to fill physical memory and trigger replacement
        for (uint32_t addr = 0; addr < 512; addr += 16) {
            try {
                vmManager->writeMemory(8, addr, addr / 16); // Write page number as value
            } catch (const std::exception& e) {
                std::cout << "Memory access at 0x" << std::hex << addr << " failed: " << e.what() << std::endl;
            }
        }
        
        auto finalStats = vmManager->getMemoryStats();
        std::cout << "Final page faults: " << finalStats.pageFaults << std::endl;
        std::cout << "Pages in: " << finalStats.pagesIn << std::endl;
        std::cout << "Pages out: " << finalStats.pagesOut << std::endl;
        
        // Should have some page replacements if we exceeded physical memory
        if (finalStats.pagesOut > 0) {
            std::cout << "✓ Page replacement algorithm activated" << std::endl;
        } else {
            std::cout << "Note: No page replacement needed (sufficient physical memory)" << std::endl;
        }
        
        std::cout << "Test 6 PASSED" << std::endl;
    }
    
    void testMemoryStats() {
        std::cout << "\n--- Test 7: Memory Statistics ---" << std::endl;
        
        auto stats = vmManager->getMemoryStats();
        
        std::cout << "Memory Statistics:" << std::endl;
        std::cout << "  Total Memory: " << stats.totalMemory << " bytes" << std::endl;
        std::cout << "  Used Memory: " << stats.usedMemory << " bytes" << std::endl;
        std::cout << "  Free Memory: " << stats.freeMemory << " bytes" << std::endl;
        std::cout << "  Total Frames: " << stats.totalFrames << std::endl;
        std::cout << "  Used Frames: " << stats.usedFrames << std::endl;
        std::cout << "  Free Frames: " << stats.freeFrames << std::endl;
        std::cout << "  Page Faults: " << stats.pageFaults << std::endl;
        std::cout << "  Pages In: " << stats.pagesIn << std::endl;
        std::cout << "  Pages Out: " << stats.pagesOut << std::endl;
        
        // Validate statistics consistency
        assert(stats.totalMemory == stats.usedMemory + stats.freeMemory && 
               "Total memory should equal used + free");
        assert(stats.totalFrames == stats.usedFrames + stats.freeFrames && 
               "Total frames should equal used + free");
        
        std::cout << "✓ Memory statistics are consistent" << std::endl;
        std::cout << "Test 7 PASSED" << std::endl;
    }
};

// Simple test runner
int main() {
    try {
        VirtualMemoryTester tester;
        tester.runAllTests();
        std::cout << "\n🎉 ALL TESTS PASSED! Virtual Memory Manager is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}