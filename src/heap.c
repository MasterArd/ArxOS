#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

#define HEAP_SIZE (1024 * 1024) // 1 MB heap memory pool
#define ALIGNMENT 8             // Align payload requests to 8-byte boundaries

typedef struct BlockHeader {
    size_t size;              // Size of usable payload space (excluding header)
    bool is_free;             // True if block is available for allocation
    struct BlockHeader *next; // Pointer to next adjacent block in memory
} BlockHeader;

#define HEADER_SIZE sizeof(BlockHeader)

// Raw pool of memory acting as the physical heap storage
static uint8_t heap_memory[HEAP_SIZE];
static BlockHeader *free_list_head = NULL;

/**
 * Helper to round up sizes to the nearest 8-byte alignment boundary.
 */
static size_t align_up(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

/**
 * Initializes the heap pool into one large free block.
 */
void heap_init(void) {
    free_list_head = (BlockHeader *)heap_memory;
    free_list_head->size = HEAP_SIZE - HEADER_SIZE;
    free_list_head->is_free = true;
    free_list_head->next = NULL;
}

/**
 * Allocates a contiguous chunk of memory of at least `size` bytes.
 */
void *kaligned_malloc(size_t size) {
    if (size == 0) return NULL;

    // Auto-initialize if heap hasn't been set up yet
    if (free_list_head == NULL) {
        heap_init();
    }

    size_t aligned_size = align_up(size);
    BlockHeader *current = free_list_head;

    while (current != NULL) {
        if (current->is_free && current->size >= aligned_size) {
            
            // Split block if remaining space can hold another header + minimum payload (16 bytes)
            if (current->size >= aligned_size + HEADER_SIZE + 16) {
                BlockHeader *new_block = (BlockHeader *)((uint8_t *)current + HEADER_SIZE + aligned_size);
                new_block->size = current->size - aligned_size - HEADER_SIZE;
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = aligned_size;
                current->next = new_block;
            }

            current->is_free = false;
            
            // Return pointer past the header structure to the actual payload area
            return (void *)(current + 1);
        }
        current = current->next;
    }

    // Out of memory or no sufficiently large block found
    return NULL;
}

/**
 * Merges adjacent free memory blocks to combat external fragmentation.
 */
static void heap_coalesce(void) {
    BlockHeader *current = free_list_head;

    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            current->size += HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

/**
 * Frees allocated memory back to the heap pool.
 */
void kfree(void *ptr) {
    if (!ptr) return;

    // Retrieve header by shifting back from payload pointer
    BlockHeader *header = (BlockHeader *)ptr - 1;
    header->is_free = true;

    // Clean up fragmented chunks
    heap_coalesce();
}

/**
 * Optional helper to print structural memory contents for debugging.
 */
void heap_dump(void) {
    BlockHeader *current = free_list_head;
    size_t index = 0;

    print("\n=== HEAP DUMP ===\n");
    while (current != NULL) {
        print("Block [%zu] @ %p | Size: %zu bytes | Status: %s\n",
               index++,
               (void *)current,
               current->size,
               current->is_free ? "FREE" : "ALLOCATED");
        current = current->next;
    }
    print("=================\n\n");
}

int main(void) {
    heap_init();
    print("Heap initialized with size: %d bytes\n", HEAP_SIZE);
    heap_dump();

    // 1. Test basic allocations
    int *numbers = (int *)kaligned_malloc(5 * sizeof(int));
    char *text = (char *)kaligned_malloc(24 * sizeof(char));

    if (numbers && text) {
        for (int i = 0; i < 5; i++) numbers[i] = (i + 1) * 10;
        print("Allocated integers: %d, %d, %d, %d, %d\n", 
               numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
    }

    heap_dump();

    // 2. Test free and coalescing
    print("Freeing integer array...\n");
    kfree(numbers);
    heap_dump();

    print("Freeing text array...\n");
    kfree(text);
    heap_dump(); // Should merge back into 1 large block

    return 0;
}