# Arena Allocator
Arena allocator implemented in C as a single header file.

## Usage
Copy-paste [arena.h](arena.h) into your project and include it. 
```c
#define ARENA_IMPLEMENTATION
#include "arena.h"
```

You can optionally define `ARENA_STATIC` to make the arena implementation
private to the source file that creates it.
```c
#define ARENA_STATIC
#define ARENA_IMPLEMENTATION
#include "arena.h"
```

### Example
```c
#include <stdio.h>
#include <assert.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define LOREM_16 "Lorem ipsum eu."
#define LOREM_32 "Lorem ipsum dolor sit amet est."
#define LOREM_64 "Lorem ipsum dolor sit amet, consectetur adipiscing elit lectus."

int main(void) {
    // Create an arena with an initial region size of 32 bytes  
    Arena *arena = arena_create(32);
    assert(arena != NULL);

    // Return a pointer to a 16-byte block of arena memory
    char *s1 = arena_alloc(arena, 16);
    assert(s1 != NULL);
    strncpy(s1, LOREM_16, 16);
    printf("%s\n", s1);

    // Change the size of the memory block pointed to by `s1` to 32 bytes
    s1 = arena_realloc(arena, s1, 16, 32);
    strncpy(s1, LOREM_32, 32);
    printf("%s\n", s1);

    // Return a pointer to a 64-byte block of arena memory. This exceeds
    // the arena's initial capacity, thus a new region will be allocated.
    char *s2 = arena_alloc(arena, 64);
    assert(s2 != NULL);
    strncpy(s2, LOREM_64, 64);
    printf("%s\n", s2);

    // Free everything
    arena_destroy(arena);

    return 0;
}
```
The included tests can be run with `make`.

