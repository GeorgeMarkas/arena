#include "arena.h"

#include <stdlib.h>

#define ALIGN sizeof(uintptr_t)
#define ROUND_UP(size) (size + (ALIGN - 1) & ~(ALIGN - 1))

Arena *arena_create(const size_t size) {
    if (size == 0) return NULL;

    const size_t alloc_size = ROUND_UP(size);
    Arena *arena = malloc(sizeof(Arena) + alloc_size);
    if (arena == NULL) return NULL;

    arena->size = alloc_size;
    arena->index = 0;

    return arena;
}

void *arena_alloc(Arena *arena, const size_t size) {
    if (size == 0) return NULL;

    const size_t alloc_size = ROUND_UP(size);
    if (arena->index + alloc_size > arena->size) return NULL;

    void *ptr = &arena->data[arena->index];
    arena->index += alloc_size;

    return ptr;
}

bool arena_resize(Arena **arena, size_t new_size) {
    // TODO: Implement
}

void arena_destroy(Arena *arena) {
    if (!arena) return;
    free(arena);
    arena = NULL;
}
