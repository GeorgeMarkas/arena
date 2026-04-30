#include <stdio.h>
#include <stdbool.h>

#define ARENA_STATIC
#define ARENA_IMPLEMENTATION
#include "arena.h"

static int tests_run = 0;
static int tests_passed = 0;
static bool test_failed;

#define TEST(name) static void test_##name(void)
#define RUN(name)                                       \
    do {                                                \
        printf("%-64s", #name "\x1B[0;37m...\x1B[0m");  \
        tests_run++;                                    \
        test_failed = false;                            \
        test_##name();                                  \
        if (!test_failed) {                             \
            printf("\x1B[1;32mOK\x1B[0m\n");            \
            tests_passed++;                             \
        }                                               \
    } while (0);

#define ASSERT(condition)                            \
    do {                                             \
        if (!(condition)) {                          \
            test_failed = true;                      \
            printf("\x1B[1;31mFAIL\x1B[0m\n  "       \
                   "Assertion failed: %s (%s:%d)\n", \
                    #condition, __FILE__, __LINE__); \
            return;                                  \
        }                                            \
    } while(0);

/* -- arena_create ---------------------------------------------------------- */

TEST(create_zero_size_returns_null) {
    const Arena *arena = arena_create(0);
    ASSERT(arena == NULL);
}

TEST(create_nonzero_size_succeeds) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    arena_destroy(arena);
}

TEST(create_size_rounds_up) {
    Arena *arena = arena_create(1);
    ASSERT(arena != NULL);
    ASSERT(arena->last->size >= 1 && arena->last->size % ALIGN == 0);
    arena_destroy(arena);
}

/* -- arena_alloc ----------------------------------------------------------- */

TEST(alloc_zero_returns_null) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 0);
    ASSERT(p == NULL);
    arena_destroy(arena);
}

TEST(alloc_null_arena_returns_null) {
    Arena *arena = NULL;
    void *p = arena_alloc(arena, 8);
    ASSERT(p == NULL);
}

TEST(alloc_nonzero_succeeds) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 8);
    ASSERT(p != NULL);
    arena_destroy(arena);
}

TEST(alloc_is_aligned) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p1 = arena_alloc(arena, 1);
    void *p2 = arena_alloc(arena, 3);
    void *p3 = arena_alloc(arena, 7);
    ASSERT(p1 != NULL && (uintptr_t) p1 % sizeof(uintptr_t) == 0);
    ASSERT(p1 != NULL && (uintptr_t) p2 % sizeof(uintptr_t) == 0);
    ASSERT(p1 != NULL && (uintptr_t) p3 % sizeof(uintptr_t) == 0);
    arena_destroy(arena);
}

TEST(alloc_sequential_no_overlap) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p1 = arena_alloc(arena, 16);
    void *p2 = arena_alloc(arena, 16);
    ASSERT((uintptr_t *) p2 >= (uintptr_t *) p1 + 16);
    arena_destroy(arena);
}

TEST(alloc_fills_region_exactly) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    size_t size = arena->last->size;
    void *p = arena_alloc(arena, size);
    ASSERT(p != NULL);
    ASSERT(arena->last->index == size);
    arena_destroy(arena);
}

TEST(alloc_overflow_adds_region) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    uintptr_t *initial_region = (uintptr_t *) arena->last;
    __attribute__((unused)) void *p = arena_alloc(arena, 64);
    uintptr_t *new_region = (uintptr_t *) arena->last;
    ASSERT(initial_region != new_region);
    ASSERT(arena->last->size == 64)
    arena_destroy(arena);
}

/* -- arena_realloc --------------------------------------------------------- */

TEST(realloc_null_ptr_returns_null) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_realloc(arena, NULL, 16, 32);
    ASSERT(p == NULL);
    arena_destroy(arena);
}

TEST(realloc_null_arena_returns_null) {
    Arena *arena = NULL;
    void *p = arena_alloc(arena, 16);
    p = arena_realloc(arena, p, 16, 32);
    ASSERT(p == NULL);
}

TEST(realloc_zero_old_size_returns_null) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 16);
    p = arena_realloc(arena, p, 0, 32);
    ASSERT(p == NULL);
    arena_destroy(arena);
}

TEST(realloc_smaller_size_returns_same_ptr) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 32);
    void *r = arena_realloc(arena, p, 32, 16);
    ASSERT(p == r);
    arena_destroy(arena);
}

TEST(realloc_same_size_returns_same_ptr) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 32);
    void *r = arena_realloc(arena, p, 32, 32);
    ASSERT(p == r);
    arena_destroy(arena);
}

TEST(realloc_previous_alloc_in_place) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    void *p = arena_alloc(arena, 16);
    void *r = arena_realloc(arena, p, 16, 32);
    ASSERT(p == r);
    ASSERT(arena->last->index > arena->prev_index);
    arena_destroy(arena);
}

TEST(realloc_previous_alloc_preserves_data) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    char *s = arena_alloc(arena, 16);
    ASSERT(s != NULL);
    memcpy(s, "Lorem ipsum nec", 16);
    char *r = arena_realloc(arena, s, 16, 32);
    ASSERT(s == r);
    ASSERT(memcmp(s, "Lorem ipsum nec", 16) == 0);
    arena_destroy(arena);
}

TEST(realloc_non_previous_alloc_copies) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    char *s = arena_alloc(arena, 16);
    ASSERT(s != NULL);
    memcpy(s, "Lorem ipsum nec", 16);
    arena_alloc(arena, 16);
    char *r = arena_realloc(arena, s, 16, 32);
    ASSERT(r != NULL);
    ASSERT(r != s);
    ASSERT(memcmp(r, "Lorem ipsum nec", 16) == 0);
    arena_destroy(arena);
}

TEST(realloc_overflow_adds_region) {
    Arena *arena = arena_create(32);
    ASSERT(arena != NULL);
    uintptr_t *initial_region = (uintptr_t *) arena->last;
    __attribute__((unused)) void *p = arena_alloc(arena, 32);
    arena_realloc(arena, p, 32, 64);
    uintptr_t *new_region = (uintptr_t *) &arena->last;
    ASSERT(initial_region != new_region);
    ASSERT(arena->last->size == 64)
    arena_destroy(arena);
}

/* -- arena_destroy --------------------------------------------------------- */

TEST(destroy_null_no_crash) {
    arena_destroy(NULL);
}

int main(void) {
    // arena_create
    RUN(create_zero_size_returns_null);
    RUN(create_nonzero_size_succeeds);
    RUN(create_size_rounds_up);

    // arena_alloc
    RUN(alloc_zero_returns_null);
    RUN(alloc_null_arena_returns_null);
    RUN(alloc_nonzero_succeeds);
    RUN(alloc_is_aligned);
    RUN(alloc_sequential_no_overlap);
    RUN(alloc_fills_region_exactly);
    RUN(alloc_overflow_adds_region);

    // arena_realloc
    RUN(realloc_null_ptr_returns_null);
    RUN(realloc_null_arena_returns_null);
    RUN(realloc_zero_old_size_returns_null);
    RUN(realloc_smaller_size_returns_same_ptr);
    RUN(realloc_same_size_returns_same_ptr);
    RUN(realloc_previous_alloc_in_place);
    RUN(realloc_previous_alloc_preserves_data);
    RUN(realloc_non_previous_alloc_copies);
    RUN(realloc_overflow_adds_region);

    // arena_destroy
    RUN(destroy_null_no_crash);

    printf("\n\x1B[;1m%d/%d passed\x1B[0m\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}