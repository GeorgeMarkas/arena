#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef ARENA_STATIC
#define ARENA_DEF static
#else
#define ARENA_DEF extern
#endif

typedef struct {
    size_t size;
    size_t index;
    size_t prev_index;
    uintptr_t data[];
} Arena;

ARENA_DEF Arena *arena_create(size_t size);

ARENA_DEF void *arena_alloc(Arena *arena, size_t size);

ARENA_DEF void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size,
                              size_t new_size);

ARENA_DEF bool arena_resize(Arena **arena, size_t new_size);

ARENA_DEF void arena_destroy(Arena *arena);

#ifdef ARENA_IMPLEMENTATION

#include <stdlib.h>

#define ALIGN sizeof(uintptr_t)
#define ROUND_UP(size) ((size + (ALIGN - 1)) & ~(ALIGN - 1))

ARENA_DEF Arena *arena_create(const size_t size) {
    if (size == 0) return NULL;

    const size_t alloc_size = ROUND_UP(size);
    Arena *arena = malloc(sizeof(Arena) + alloc_size);
    if (arena == NULL) return NULL;

    arena->size = alloc_size;
    arena->index = 0;
    arena->prev_index = 0;

    return arena;
}

ARENA_DEF void *arena_alloc(Arena *arena, const size_t size) {
    if (size == 0) return NULL;

    const size_t alloc_size = ROUND_UP(size);
    if (arena->index + alloc_size > arena->size) return NULL;

    void *ptr = &arena->data[arena->index];
    arena->prev_index = arena->index;
    arena->index += alloc_size;

    return ptr;
}

ARENA_DEF void *arena_realloc(Arena *arena, void *old_ptr,
                              const size_t old_size,
                              const size_t new_size) {

    if (!old_ptr || old_size == 0 || !arena) return NULL;
    if (new_size <= old_size) return old_ptr;
    // Ensure the given pointer is within arena bounds
    if (arena->data <= (uintptr_t *) old_ptr &&
        (uintptr_t *) old_ptr < arena->data + arena->size) {
        // Check if the pointer is the result of the last allocation,
        // in which case we can just increment / decrement the buffer offset.
        if (arena->data + arena->prev_index == old_ptr) {
            const size_t realloc_size = ROUND_UP(new_size);
            if (arena->prev_index + realloc_size > arena->size) return NULL;
            arena->index = arena->prev_index + realloc_size;
            return old_ptr;
        } else {
            void *new_ptr = arena_alloc(arena, new_size);
            memmove(new_ptr, old_ptr, old_size);
            return new_ptr;
        }
    }

    return NULL;
}

ARENA_DEF bool arena_resize(Arena **arena, size_t new_size) {
    // TODO: Implement
}

ARENA_DEF void arena_destroy(Arena *arena) {
    if (!arena) return;
    free(arena);
    arena = NULL;
}

#endif // ARENA_IMPLEMENTATION
