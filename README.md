# Arena Allocator
Arena allocator implemented in C as a single header file.

## Usage
Copy-paste [arena.h](arena.h) and include it.
```c
#include <stdio.h>
#include <assert.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define LOREM_16 "Lorem ipsum sed"
#define LOREM_32 "Lorem ipsum dolor sit amet duis"
#define LOREM_64 "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed in"

int main(void) {
    Arena *arena = arena_create(32);
    assert(arena != NULL);

    char *s1 = arena_alloc(arena, 16);
    assert(s1 != NULL);
    strncpy(s1, LOREM_16, 16);
    printf("%s\n", s1);

    s1 = arena_realloc(arena, s1, 16, 32);
    strncpy(s1, LOREM_32, 32);
    printf("%s\n", s1);

    // Automatically allocates a new region
    char *s2 = arena_alloc(arena, 64);
    assert(s2 != NULL);
    strncpy(s2, LOREM_64, 64);
    printf("%s\n", s2);

    // Free everything
    arena_destroy(arena);

    return 0;
}
```

You can optionally define `ARENA_STATIC` to make the arena implementation
private to the source file that creates it.
```c
#define ARENA_STATIC
#define ARENA_IMPLEMENTATION
#include "arena.h"

// ...
```

The included tests can be run with `make`.

