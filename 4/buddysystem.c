#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

// Структура аллокатора
typedef struct {
    void* memory;
    size_t size;
    size_t min_block_size;
    bool* used_blocks;
} Allocator;

// Создание аллокатора
Allocator* allocator_create(void* memory, size_t size) {
    if (!memory || size < sizeof(Allocator) + sizeof(bool) * (size / 64)) {
        fprintf(stderr, "Error: invalid memory or size\n");
        return NULL;
    }
    Allocator* allocator = (Allocator*)memory;
    allocator->memory = memory;
    allocator->size = size;
    allocator->min_block_size = 64;
    allocator->used_blocks = (bool*)((char*)memory + sizeof(Allocator));
    for (size_t i = 0; i < size / allocator->min_block_size; i++) {
        allocator->used_blocks[i] = false;
    }
    return allocator;
}

// Выделение памяти
void* allocator_alloc(Allocator* allocator, size_t size) {
    if (!allocator || size == 0) {
        return NULL;
    }
    size_t block_size = allocator->min_block_size;
    while (block_size < size) {
        block_size *= 2;
    }
    for (size_t i = 0; i < allocator->size / block_size; i++) {
        if (!allocator->used_blocks[i]) {
            allocator->used_blocks[i] = true;
            return (void*)((char*)allocator->memory + i * block_size);
        }
    }
    return NULL;

// Освобождение памяти
void allocator_free(Allocator* allocator, void* memory) {
    if (!allocator || !memory) {
        return;
    }
    size_t block_size = allocator->min_block_size;
    size_t index = ((char*)memory - (char*)allocator->memory) / block_size;
    allocator->used_blocks[index] = false;
}

// Уничтожение аллокатора
void allocator_destroy(Allocator* allocator) {
    if (allocator) {
        munmap(allocator->memory, allocator->size);
    }
}