#define ARENA_IMPLEMENTATION
#include "../arena.h"

#define TEST_IMPLEMENTATION
#include "test.h"

/* -- arena_create ---------------------------------------------------------- */

TEST(create_size_zero_returns_null) {
    Arena *arena = arena_create(0);
    TEST_ASSERT_NULL(arena);
}

TEST(create_size_nonzero_succeeds) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    arena_destroy(arena);
}

TEST(create_size_rounds_up) {
    Arena *arena = arena_create(1);
    TEST_ASSERT_NOT_NULL(arena);
    TEST_ASSERT(arena->last->size >= 1);
    TEST_ASSERT(arena->last->size % sizeof(uintptr_t) == 0);
    arena_destroy(arena);
}

/* -- arena_alloc ----------------------------------------------------------- */

TEST(alloc_size_zero_returns_null) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 0);
    TEST_ASSERT_NULL(p);
    arena_destroy(arena);
}

TEST(alloc_arena_null_returns_null) {
    void *p = arena_alloc(NULL, 32);
    TEST_ASSERT_NULL(p);
}

TEST(alloc_size_nonzero_succeeds) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 32);
    TEST_ASSERT_NOT_NULL(p);
    arena_destroy(arena);
}

TEST(alloc_is_aligned) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p1 = arena_alloc(arena, 1);
    void *p2 = arena_alloc(arena, 3);
    void *p3 = arena_alloc(arena, 7);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NOT_NULL(p3);
    TEST_ASSERT((uintptr_t) p1 % sizeof(uintptr_t) == 0);
    TEST_ASSERT((uintptr_t) p2 % sizeof(uintptr_t) == 0);
    TEST_ASSERT((uintptr_t) p3 % sizeof(uintptr_t) == 0);
    arena_destroy(arena);
}

    TEST(alloc_sequential_no_overlap) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p1 = arena_alloc(arena, 16);
    void *p2 = arena_alloc(arena, 16);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT((uintptr_t *) p2 >= (uintptr_t *) p1 + 16);
    arena_destroy(arena);
}

TEST(alloc_fills_region_exactly) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 32);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT(arena->last->index == 32);
    arena_destroy(arena);
}

TEST(alloc_overflow_creates_new_region) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    uintptr_t *initial_region = (uintptr_t *) arena->last;
    void *p = arena_alloc(arena, 64);
    TEST_ASSERT_NOT_NULL(p);
    uintptr_t *new_region = (uintptr_t *) arena->last;
    TEST_ASSERT_NOT_NULL(new_region);
    TEST_ASSERT(initial_region != new_region);
    TEST_ASSERT(arena->last->size >= 64);
    arena_destroy(arena);
}

/* -- arena_realloc --------------------------------------------------------- */

TEST(realloc_ptr_null_returns_null) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_realloc(arena, NULL, 32, 64);
    TEST_ASSERT_NULL(p);
    arena_destroy(arena);
}

TEST(realloc_arena_null_returns_null) {
    void *p;
    p = arena_realloc(NULL, NULL, 32, 64);
    TEST_ASSERT_NULL(p);
}

TEST(realloc_old_size_zero_returns_null) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 32);
    p = arena_realloc(arena, p, 0, 64);
    TEST_ASSERT_NULL(p);
    arena_destroy(arena);
}

TEST(realloc_smaller_new_size_returns_same_ptr) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 32);
    void *r = arena_realloc(arena, p, 16, 32);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT(p == r);
    arena_destroy(arena);
}

TEST(realloc_previous_alloc_in_place) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    void *p = arena_alloc(arena, 16);
    void *r = arena_realloc(arena, p, 16, 32);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT(p == r);
    TEST_ASSERT(arena->last->index > arena->prev_index);
    arena_destroy(arena);
}

TEST(realloc_previous_alloc_preserves_data) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    char *s = arena_alloc(arena, 16);
    TEST_ASSERT_NOT_NULL(s);
    memcpy(s, "Lorem ipsum mi.", 16);
    char *r = arena_realloc(arena, s, 16, 32);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT(s == r);
    TEST_ASSERT(memcmp(s, "Lorem ipsum mi.", 16) == 0);
    arena_destroy(arena);
}

TEST(realloc_non_previous_alloc_copies) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    char *s = arena_alloc(arena, 16);
    TEST_ASSERT_NOT_NULL(s);
    memcpy(s, "Lorem ipsum mi.", 16);
    arena_alloc(arena, 16); // Bump the index
    char *r = arena_realloc(arena, s, 16, 32);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT(s != r);
    TEST_ASSERT(memcmp(r, "Lorem ipsum mi.", 16) == 0);
    arena_destroy(arena);
}

TEST(realloc_overflow_creates_new_region) {
    Arena *arena = arena_create(32);
    TEST_ASSERT_NOT_NULL(arena);
    uintptr_t *initial_region = (uintptr_t *) arena->last;
    void *p = arena_alloc(arena, 32);
    TEST_ASSERT_NOT_NULL(p);
    p = arena_realloc(arena, p, 32, 64);
    TEST_ASSERT_NOT_NULL(p);
    uintptr_t *new_region = (uintptr_t *) arena->last;
    TEST_ASSERT_NOT_NULL(new_region);
    TEST_ASSERT(initial_region != new_region);
    TEST_ASSERT(arena->last->index == 64);
    arena_destroy(arena);
}

/* -- arena_destroy --------------------------------------------------------- */

TEST(destroy_null_ptr_no_crash) {
    arena_destroy(NULL);
}

int main(void) {
    // arena_create
    TEST_RUN(create_size_zero_returns_null);
    TEST_RUN(create_size_nonzero_succeeds);
    TEST_RUN(create_size_rounds_up);

    // arena_alloc
    TEST_RUN(alloc_size_zero_returns_null);
    TEST_RUN(alloc_arena_null_returns_null);
    TEST_RUN(alloc_size_nonzero_succeeds);
    TEST_RUN(alloc_is_aligned);
    TEST_RUN(alloc_sequential_no_overlap);
    TEST_RUN(alloc_fills_region_exactly);
    TEST_RUN(alloc_overflow_creates_new_region);

    // arena_realloc
    TEST_RUN(realloc_ptr_null_returns_null);
    TEST_RUN(realloc_arena_null_returns_null);
    TEST_RUN(realloc_old_size_zero_returns_null);
    TEST_RUN(realloc_smaller_new_size_returns_same_ptr);
    TEST_RUN(realloc_previous_alloc_in_place);
    TEST_RUN(realloc_previous_alloc_preserves_data);
    TEST_RUN(realloc_non_previous_alloc_copies);
    TEST_RUN(realloc_overflow_creates_new_region);

    // arena_destroy
    TEST_RUN(destroy_null_ptr_no_crash);

    TEST_CONCLUDE;
}
