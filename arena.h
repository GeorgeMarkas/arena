#pragma once

#include <stdint.h>

typedef struct {
    size_t index;
    size_t size;
    struct Arena_Region *next;
    uintptr_t data[];
} Arena_Region;

typedef struct {
    Arena_Region *region;
    size_t index;
    size_t size;
} Arena;

/**
 * Allocate memory for an arena of specified size.
 * @param size The size of the arena in bytes.
 * @return Pointer to the arena on success, or NULL on failure.
 */
Arena *arena_create(size_t size);

/**
 * Return a pointer to a portion of specified size of the specified
 * arena's region.
 * @param arena The arena of which the pointer from the region will be returned.
 * @param size The size of allocated memory in bytes.
 * @return Pointer to the portion of the arena's region on success, or NULL
 * on failure.
 */
void *arena_alloc(Arena *arena, size_t size);

/**
 * Reset the pointer to the arena region to the start of the allocated memory,
 * allowing for reuse of the memory without freeing it.
 * @param arena The arena to be reset.
 */
void *arena_reset(Arena *arena);

/**
 * Free the memory allocated for the arena.
 * @param arena The arena to be destroyed.
 */
void arena_destroy(Arena *arena);

#ifdef ARENA_IMPLEMENTATION
#endif // ARENA_IMPLEMENTATION
