#pragma once

#include <stdint.h>

#ifdef ARENA_STATIC
#define ARENA_DEF static
#else
#define ARENA_DEF extern
#endif

typedef struct Arena_Region {
    struct Arena_Region *next;
    size_t size;
    size_t index;
    uintptr_t data[];
} Arena_Region;

typedef struct {
    Arena_Region *first;
    Arena_Region *last;
    size_t prev_index;
} Arena;

/**
 * Allocate a new arena of specified size.
 * @param size The size of the arena in bytes.
 * @return Pointer to the arena, or NULL on failure.
 */
ARENA_DEF Arena *arena_create(size_t size);

/**
 * Return a pointer to an arena memory block of specified size.
 * @param arena The arena whose memory to get a pointer to.
 * @param size The size of the memory block in bytes.
 * @return Pointer to arena memory on success, or NULL on failure.
 */
ARENA_DEF void *arena_alloc(Arena *arena, size_t size);

/**
 * Change the size of the arena memory block pointed to by a given pointer.
 * @param arena The arena the memory block belongs to.
 * @param old_ptr The pointer to the memory block.
 * @param old_size The old size of the memory block.
 * @param new_size The new size of the memory block.
 * @return A pointer to the resized memory block, or NULL on failure.
 */
ARENA_DEF void *arena_realloc(Arena *arena, void *old_ptr, size_t old_size,
                              size_t new_size);

/**
 * Free the memory allocated for an arena.
 * @param arena The arena to be destroyed.
 */
ARENA_DEF void arena_destroy(Arena *arena);

#ifdef ARENA_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#define ALIGN sizeof(uintptr_t)
#define ROUND_UP(size) ((size + (ALIGN - 1)) & ~(ALIGN - 1))

static Arena_Region *region_alloc(const size_t size) {
    const size_t region_size = ROUND_UP(size);
    Arena_Region *region = malloc(sizeof(Arena_Region) + region_size);
    if (!region) return NULL;

    region->next = NULL;
    region->size = region_size;
    region->index = 0;

    return region;
}

ARENA_DEF Arena *arena_create(size_t size) {
    if (size == 0) return NULL;

    Arena *arena = malloc(sizeof(Arena));
    if (!arena) return NULL;

    Arena_Region *region = region_alloc(size);
    if (!region) return NULL;

    arena->first = region;
    arena->last = region;

    return arena;
}

ARENA_DEF void *arena_alloc(Arena *arena, const size_t size) {
    if (size == 0 || !arena) return NULL;

    const size_t alloc_size = ROUND_UP(size);
    Arena_Region *current = arena->last;
    if (current->index + alloc_size > current->size) {
        Arena_Region *region = region_alloc(alloc_size);
        if (!region) return NULL;

        current->next = region;
        arena->last = current = region;
    }

    void *ptr = &current->data[current->index];
    arena->prev_index = current->index;
    current->index += alloc_size;

    return ptr;
}

ARENA_DEF void *arena_realloc(Arena *arena, void *old_ptr,
                              const size_t old_size,
                              const size_t new_size) {

    if (!old_ptr || old_size == 0 || !arena) return NULL;
    if (new_size <= old_size) return old_ptr;
    // Check if the pointer is the result of the last allocation,
    // in which case we can just increment the buffer offset.
    if (arena->last->data + arena->prev_index == old_ptr) {
        const size_t realloc_size = ROUND_UP(new_size);
        if (arena->prev_index + realloc_size <= arena->last->size) {
            arena->last->index = arena->prev_index + realloc_size;
            return old_ptr;
        }
    }

    void *new_ptr = arena_alloc(arena, new_size);
    memcpy(new_ptr, old_ptr, old_size);

    return new_ptr;
}

ARENA_DEF void arena_destroy(Arena *arena) {
    if (!arena) return;

    Arena_Region *current = arena->first;
    while (current) {
        Arena_Region *temp = current;
        current = current->next;
        free(temp);
    }

    arena->first = NULL;
    arena->last = NULL;
    free(arena);
    arena = NULL;
}

#endif // ARENA_IMPLEMENTATION
