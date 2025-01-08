#ifndef OS4_FIRST_FIT_H
#define OS4_FIRST_FIT_H

#include "emergency_allocator.h"

typedef struct Node {
    void* value;
    size_t size;
    struct Node* next;
} Node;

typedef struct {
    Allocator allocator;
    Node* head;
    size_t total_memory;
    size_t used_memory;
} List;

Allocator* free_list_allocator_create(void* const memory, const size_t size);
void free_list_allocator_destroy(Allocator* const allocator);
void* free_list_allocator_alloc(Allocator * allocator, ssize_t size);
void free_list_allocator_free(Allocator* const allocator, void* memory);
void free_list_allocator_print_stats(const Allocator *allocator);

#endif //OS4_FFIRST_FIT_H