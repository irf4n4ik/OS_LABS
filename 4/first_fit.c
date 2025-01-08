#include <stdio.h>
#include "first_fit.h"

Allocator* free_list_allocator_create(void* const memory, const size_t size) {
    List* list = mmap(NULL, sizeof(List), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (list == MAP_FAILED)
        return NULL;

    list->head = mmap(NULL, sizeof(Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (list->head == MAP_FAILED) {
        munmap(list, sizeof(List));
        return NULL;
    }
    if (memory == NULL) {
        list->head->value = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (!list->head->value) {
            munmap(list, sizeof(List));
            return NULL;
        }
    } else {
        list->head->value = memory;
        list->head->size = size;
        list->head->next = NULL;
    }

    list->total_memory = size;
    list->used_memory = 0;

    return (Allocator*)list;
}

void free_list_allocator_free(Allocator * const allocator, void* memory) {
    List* list = (List*)allocator;
    Node* cur = list->head;

    Node* prev = NULL;
    while (cur) {
        if (cur->next && cur->next != memory) {
          prev = cur;
          cur = cur->next;
          } else if (cur->next == memory) {
              prev = cur->next->next;
              munmap(cur->next->value, cur->next->size);
              munmap(cur->next, sizeof(Node));
          } else {
            return;
        }
    }
}


void* free_list_allocator_alloc(Allocator* allocator, ssize_t size) {
    List* list = (List*)allocator;
    Node* cur = list->head;
    while (cur->next != NULL)
        cur = cur->next;

    cur->next = mmap(NULL, sizeof(Node), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (cur->next == MAP_FAILED) {
        return NULL;
    }
    cur->next->value = mmap(NULL, size, PROT_WRITE| PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (cur->next->value == MAP_FAILED) {
        return NULL;
    }
    cur->next->size = size;
    cur->next->next = NULL;
    return cur->next->value;
}

void free_list_allocator_destroy(Allocator * const allocator) {
    List* list = (List*)allocator;
    Node* cur = list->head;
    while (cur->next != NULL) {
        Node* tmp = cur;
        cur = cur->next;
        munmap(tmp->value, tmp->size);
        munmap(tmp, sizeof(Node));
    }
}

void free_list_allocator_print_stats(const Allocator *allocator) {
    const List *list = (const List *)allocator;
    printf("Free List Allocator Stats:\n");
    printf("  Total Memory: %zu bytes\n", list->total_memory);
    printf("  Used Memory: %zu bytes\n", list->used_memory);
    printf("  Free Memory: %zu bytes\n", list->total_memory - list->used_memory);
}