#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>

#include "emergency_allocator.h"

Allocator* (*allocator_create)(void*, const size_t) = emergency_allocator_create;
void (*allocator_free)(Allocator* const, void* const) = emergency_allocator_free;
void (*allocator_destroy)(Allocator* const) = emergency_allocator_destroy;
void* (*allocator_alloc)(Allocator* const, const size_t) = emergency_allocator_alloc;
void (*allocator_print_stats)(const Allocator*) = NULL;

// Функция для вывода помощи
void print_help() {
    printf("Usage: ./main <allocator_type> [memory_size]\n");
    printf("  allocator_type: buddy, first_fit, or emergency\n");
    printf("  memory_size: optional, size of memory pool in bytes (default: 4096)\n");
}

void test_allocator(Allocator* allocator, size_t memory_size) {
    printf("Testing allocator with memory size: %zu bytes\n", memory_size);

    // Тест 1: Простое выделение и освобождение памяти
    printf("\nTest 1: Simple allocation and deallocation\n");
    clock_t start = clock();

    int* a = allocator_alloc(allocator, sizeof(int));
    int* b = allocator_alloc(allocator, sizeof(int) * 2);
    int* c = allocator_alloc(allocator, sizeof(int) * 3);

    if (a && b && c) {
        *a = 5;
        b[0] = 10;
        b[1] = 7;
        c[0] = 4;
        c[1] = 6;
        c[2] = 3;

        printf("Allocations successful:\n");
        printf("  a: %d, b: %d %d, c: %d %d %d\n", *a, b[0], b[1], c[0], c[1], c[2]);
    } else {
        printf("Allocation failed!\n");
    }

    allocator_free(allocator, a);
    allocator_free(allocator, b);
    allocator_free(allocator, c);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time spent: %f seconds\n", time_spent);

    // Тест 2: Множественные выделения и освобождения (фрагментация)
    printf("\nTest 2: Multiple allocations and deallocations (fragmentation test)\n");
    start = clock();

    void* blocks[100];
    for (int i = 0; i < 100; i++) {
        blocks[i] = allocator_alloc(allocator, (i % 10 + 1) * sizeof(int));
        if (!blocks[i]) {
            printf("Allocation failed at iteration %d\n", i);
            break;
        }
    }

    for (int i = 0; i < 100; i += 2) {
        if (blocks[i]) {
            allocator_free(allocator, blocks[i]);
            blocks[i] = NULL;
        }
    }

    for (int i = 0; i < 100; i += 2) {
        blocks[i] = allocator_alloc(allocator, (i % 10 + 1) * sizeof(int));
        if (!blocks[i]) {
            printf("Allocation failed at iteration %d\n", i);
            break;
        }
    }

    for (int i = 0; i < 100; i++) {
        if (blocks[i]) {
            allocator_free(allocator, blocks[i]);
        }
    }

    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time spent: %f seconds\n", time_spent);

    // Тест 3: Граничные случаи (выделение памяти больше, чем доступно)
    printf("\nTest 3: Boundary cases (allocating more than available memory)\n");
    void* huge_block = allocator_alloc(allocator, memory_size * 2);
    if (huge_block) {
        printf("Unexpected success: allocated a block larger than available memory!\n");
        allocator_free(allocator, huge_block);
    } else {
        printf("Allocation failed as expected (requested size exceeds available memory).\n");
    }

    if (allocator_print_stats) {
        printf("\nAllocator Stats:\n");
        allocator_print_stats(allocator);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    const char* allocator_type = argv[1];
    size_t memory_size = (argc > 2) ? atoi(argv[2]) : 4096;

    void* library = NULL;
    if (strcmp(allocator_type, "buddy") == 0) {
        library = dlopen("./libbuddy_allocator.so", RTLD_LOCAL | RTLD_NOW);
        if (library) {
            allocator_create = dlsym(library, "buddy_allocator_create");
            allocator_free = dlsym(library, "buddy_allocator_free");
            allocator_destroy = dlsym(library, "buddy_allocator_destroy");
            allocator_alloc = dlsym(library, "buddy_allocator_alloc");
            allocator_print_stats = dlsym(library, "buddy_allocator_print_stats");
        }
    } else if (strcmp(allocator_type, "first_fit") == 0) {
        library = dlopen("./libfree_list_allocator.so", RTLD_LOCAL | RTLD_NOW);
        if (library) {
            allocator_create = dlsym(library, "free_list_allocator_create");
            allocator_free = dlsym(library, "free_list_allocator_free");
            allocator_destroy = dlsym(library, "free_list_allocator_destroy");
            allocator_alloc = dlsym(library, "free_list_allocator_alloc");
            allocator_print_stats = dlsym(library, "free_list_allocator_print_stats");
        }
    } else if (strcmp(allocator_type, "emergency") == 0) { // Используем аварийный аллокатор по умолчанию
    } else {
        printf("Unknown allocator type: %s\n", allocator_type);
        print_help();
        return 1;
    }

    if (library == NULL && strcmp(allocator_type, "emergency") != 0) {
        printf("Failed to load library for allocator type: %s\n", allocator_type);
        printf("Falling back to emergency allocator.\n");
    }

    // Выделяем память для аллокатора
    void* memory_pool = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (memory_pool == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    Allocator* allocator = allocator_create(memory_pool, memory_size);
    if (!allocator) {
        perror("allocator_create");
        munmap(memory_pool, memory_size);
        return 1;
    }
    test_allocator(allocator, memory_size);
    allocator_destroy(allocator);
    munmap(memory_pool, memory_size);

    if (library) {
        dlclose(library);
    }

    return 0;
}