#pragma once

#include <stdint.h>
#include <stdlib.h>

#define ALIGN sizeof(uintptr_t)
#define ROUND_UP(size) (size + (ALIGN - 1) & ~(ALIGN - 1))

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
void arena_reset(Arena *arena);

/**
 * Free the memory allocated for the arena.
 * @param arena The arena to be destroyed.
 */
void arena_destroy(Arena *arena);

#ifdef ARENA_IMPLEMENTATION

static Arena_Region *region_alloc(const size_t size) {
    const size_t region_size = ROUND_UP(size);
    Arena_Region *region = malloc(sizeof(Arena_Region) + region_size);
    if (!region) return NULL;

    region->index = 0;
    region->size = region_size;
    region->next = NULL;

    return region;
}

Arena *arena_create(const size_t size) {
    if (size == 0) return NULL;

    Arena *arena = malloc(sizeof(Arena));
    if (!arena) return NULL;

    Arena_Region *region = region_alloc(size);
    if (!region) return NULL;

    arena->region = region;
    arena->index = 0;
    arena->size = ROUND_UP(size);

    return arena;
}

static void arena_expand(Arena *arena) {
    // Expansion logic
}

void *arena_alloc(Arena *arena, const size_t size) {
    if (size == 0 || !arena || !arena->region) return NULL;
    if (arena->size - arena->index < size) {
        // Expand
    }

    arena->index += size;

    return arena->region + (arena->index - size);
}

#endif // ARENA_IMPLEMENTATION
