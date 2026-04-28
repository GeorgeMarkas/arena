#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t index;
    size_t size;
    uintptr_t data[];
} Arena;

Arena *arena_create(size_t size);

void *arena_alloc(Arena *arena, size_t size);

bool arena_resize(Arena **arena, size_t new_size);

void arena_destroy(Arena *arena);
