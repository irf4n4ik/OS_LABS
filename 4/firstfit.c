#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

typedef struct Block {
    size_t size;
    struct Block* next;
    bool is_free;
} Block;

typedef struct {
    void* memory;
    size_t size;
    Block* free_list;
} Allocator;

// Создание аллокатора
Allocator* allocator_create(void* memory, size_t size) {
    if (!memory || size < sizeof(Allocator) + sizeof(Block)) {
        fprintf(stderr, "Error: invalid memory or size\n");
        return NULL;
    }
    Allocator* allocator = (Allocator*)memory;
    allocator->memory = memory;
    allocator->size = size;
    allocator->free_list = (Block*)((char*)memory + sizeof(Allocator));
    allocator->free_list->size = size - sizeof(Allocator);
    allocator->free_list->next = NULL;
    allocator->free_list->is_free = true;
    return allocator;
}

// Выделение памяти
void* allocator_alloc(Allocator* allocator, size_t size) {
    if (!allocator || size == 0) {
        return NULL;
    }
    Block* current = allocator->free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            current->is_free = false;
            return (void*)((char*)current + sizeof(Block));
        }
        current = current->next;
    }
    return NULL;
}

// Освобождение памяти
void allocator_free(Allocator* allocator, void* memory) {
    if (!allocator || !memory) {
        return;
    }
    Block* block = (Block*)((char*)memory - sizeof(Block));
    block->is_free = true;
}

// Уничтожение аллокатора
void allocator_destroy(Allocator* allocator) {
    if (allocator) {
        munmap(allocator->memory, allocator->size);
    }
}