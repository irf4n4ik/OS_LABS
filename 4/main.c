#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>

// Интерфейс аллокатора
typedef struct {
    void* (*allocator_create)(void*, size_t);
    void (*allocator_destroy)(void*);
    void* (*allocator_alloc)(void*, size_t);
    void (*allocator_free)(void*, void*);
} AllocatorAPI;

void* fallback_allocator_create(void* memory, size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

void fallback_allocator_destroy(void* memory) {
    munmap(memory, 0);
}

void* fallback_allocator_alloc(void* memory, size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

void fallback_allocator_free(void* memory, void* ptr) {
    munmap(ptr, 0);
}

int main(int argc, char** argv) {
    AllocatorAPI api;
    void* library = NULL;
    if (argc > 1) {
        library = dlopen(argv[1], RTLD_LAZY);
        if (library) {
            api.allocator_create = dlsym(library, "allocator_create");
            api.allocator_destroy = dlsym(library, "allocator_destroy");
            api.allocator_alloc = dlsym(library, "allocator_alloc");
            api.allocator_free = dlsym(library, "allocator_free");
        }
    }
    if (!library) {
        api.allocator_create = fallback_allocator_create;
        api.allocator_destroy = fallback_allocator_destroy;
        api.allocator_alloc = fallback_allocator_alloc;
        api.allocator_free = fallback_allocator_free;
    }
    
    size_t memory_size = 1024 * 1024; // 1 MB
    void* memory = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Инициализация аллокатора
    void* allocator = api.allocator_create(memory, memory_size);
    if (!allocator) {
        fprintf(stderr, "Failed to create allocator\n");
        munmap(memory, memory_size);
        return 1;
    }

    // Выделение памяти
    void* ptr = api.allocator_alloc(allocator, 128);
    if (!ptr) {
        fprintf(stderr, "Failed to allocate memory\n");
    } else {
        printf("Memory allocated at %p\n", ptr);
    }

    // Освобождение памяти
    if (ptr) {
        api.allocator_free(allocator, ptr);
        printf("Memory freed\n");
    }

    // Уничтожение аллокатора
    api.allocator_destroy(allocator);
    munmap(memory, memory_size);

    // Закрытие библиотеки
    if (library) {
        dlclose(library);
    }

    return 0;
}