
#ifndef OS4_EMERGENCY_ALLOCATOR_H
#define OS4_EMERGENCY_ALLOCATOR_H

#include <stdlib.h>
#include <sys/mman.h>

typedef struct {
} Allocator;

typedef struct {
    Allocator allocator;
    void* memory;
    size_t size;
} EmergencyAllocator;

Allocator* emergency_allocator_create(void *const memory, const size_t size);

void emergency_allocator_free(Allocator *const allocator, void *const memory);

void emergency_allocator_destroy(Allocator *const allocator);

void* emergency_allocator_alloc(Allocator *const allocator, const size_t size);


#endif //OS4_EMERGENCY_ALLOCATOR_H
