#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include "emergency_allocator.h"

typedef struct Block {
    void* start;
    size_t size;
    int is_free;
    struct Block* left;
    struct Block* right;
    struct Block* parent;
    struct Block* next_free;
} Block;

typedef struct BuddyAllocator {
    Block* root;
    Block* free_tree_nodes;
    size_t total_memory;
    size_t used_memory;
} BuddyAllocator;

Allocator* buddy_allocator_create(void* memory, const size_t size);
void buddy_allocator_destroy(Allocator* allocator);
void* buddy_allocator_alloc(Allocator* allocator, size_t size);
void buddy_allocator_free(Allocator* allocator, void* ptr);
void buddy_allocator_print_stats(const Allocator* allocator);

#endif // BUDDY_SYSTEM_H