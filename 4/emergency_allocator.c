#include "emergency_allocator.h"

Allocator* emergency_allocator_create(void *const memory, const size_t size){
    EmergencyAllocator* allocator = mmap(NULL, sizeof(EmergencyAllocator), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator == MAP_FAILED)
        return NULL;

    if (memory == NULL) {
        allocator->memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    } else {
        allocator->memory = memory;
    }

    allocator->size = size;
    return (Allocator*)allocator;
}

void emergency_allocator_free(Allocator *const allocator, void *const memory) {
    EmergencyAllocator* tmp_allocator = (EmergencyAllocator*)allocator;
    munmap(memory, tmp_allocator->size);
}

void emergency_allocator_destroy(Allocator *const allocator) {
    EmergencyAllocator* tmp_allocator = (EmergencyAllocator*)allocator;
    emergency_allocator_free((Allocator*)tmp_allocator, tmp_allocator->memory);
    munmap(tmp_allocator, sizeof(EmergencyAllocator));
}

void* emergency_allocator_alloc(Allocator*const allocator, const size_t size) {
    EmergencyAllocator* tmp_allocator = (EmergencyAllocator*)allocator;
    if (!tmp_allocator->memory) {
        tmp_allocator->memory = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (!tmp_allocator->memory) {
            emergency_allocator_destroy(allocator);
            return NULL;
        }
    }
    return tmp_allocator->memory;
}