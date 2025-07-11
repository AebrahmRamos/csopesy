class MemoryAllocator {
private:
    MemoryBlock* head;      // Head of the linked list of memory blocks
    size_t totalMemory;     // Total available memory in the pool
    size_t usedMemory;      // Total memory currently in use by processes

public:
    MemoryAllocator(size_t total) : totalMemory(total), usedMemory(0), head(nullptr) {
        head = new MemoryBlock(total);  // Create a single large memory block representing the pool
    }

    ~MemoryAllocator() {
        MemoryBlock* current = head;
        while (current != nullptr) {
            MemoryBlock* temp = current;
            current = current->next;
            delete temp;
        }
    }

    // Allocate memory for a process based on its memory requirements
    void* allocate(size_t size) {
        if (usedMemory + size > totalMemory) {
            std::cout << "Memory Full! Process pushed back to the ready queue." << std::endl;
            return nullptr;  // Not enough memory
        }

        MemoryBlock* current = head;
        while (current != nullptr) {
            if (current->isFree && current->size >= size) {
                current->isFree = false;
                usedMemory += size;

                if (current->size > size) {
                    MemoryBlock* newBlock = new MemoryBlock(current->size - size);
                    current->size = size;
                    newBlock->next = current->next;
                    current->next = newBlock;
                }

                return reinterpret_cast<void*>(current);  // Return a pointer to the allocated memory
            }
            current = current->next;
        }
        return nullptr;  // If no suitable block was found
    }

    // Deallocate memory when a process finishes execution
    void deallocate(void* ptr, size_t size) {
        MemoryBlock* blockToFree = reinterpret_cast<MemoryBlock*>(ptr);
        blockToFree->isFree = true;
        usedMemory -= size;
        mergeBlocks();
    }

    // Merge adjacent free blocks to minimize fragmentation
    void mergeBlocks() {
        MemoryBlock* current = head;
        while (current != nullptr && current->next != nullptr) {
            if (current->isFree && current->next->isFree) {
                current->size += current->next->size;
                MemoryBlock* temp = current->next;
                current->next = current->next->next;
                delete temp;
            } else {
                current = current->next;
            }
        }
    }

    // Generate a report on memory usage
    void generateMemoryReport() {
        std::cout << "Timestamp: " << std::time(0) << std::endl;
        size_t freeMemory = totalMemory - usedMemory;
        size_t fragmentation = freeMemory % 1024;  // External fragmentation in KB
        std::cout << "Processes in memory: " << usedMemory / config.memPerProc << std::endl;
        std::cout << "Total external fragmentation: " << fragmentation << " KB" << std::endl;
        printMemoryState();
    }

    // Print memory state for debugging purposes
    void printMemoryState() {
        MemoryBlock* current = head;
        while (current != nullptr) {
            std::cout << "Block size: " << current->size
                      << ", " << (current->isFree ? "Free" : "Allocated") << std::endl;
            current = current->next;
        }
    }
};
