#ifndef __TYPED_ARENA_H__
#define __TYPED_ARENA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

enum arena_error
{
    ARENA_EOK,
    ARENA_EFAILED
};

static inline bool ___arena_in_range(const uintptr_t start, const uintptr_t end, const uintptr_t pos)
{
    return pos >= start && pos <= end;
}

#ifndef TYPED_ARENA_SIZE
    #define TYPED_ARENA_SIZE ((1 << 20) * 2) /* 2MiB */
#endif

#define CONCAT2(str1, str2) str1##str2
#define CONCAT3(str1, str2, str3) str1##str2##str3

#define arena(type) struct ___arena_##type

#define decl_arena_type(type) \
    /* The arena structure */ \
    arena(type) \
    { \
        size_t size; /* Size of the arena in bytes */ \
        size_t idx; /* Index into the arena. NOT a byte offset */ \
        type *data; /* A pointer to the arena */ \
        enum arena_error (*init)(void); /* The arenas init method */ \
        type *(*alloc)(const size_t nmemb); /* The arenas allocation method */ \
        void (*recycle)(void); /* The arenas recycle method */ \
        void (*free)(void); /* The arenas free method */ \
    }; \
    /* Allocate the arena buffer */ \
    static enum arena_error CONCAT2(___arena_##type, _init)(arena(type) *arena) \
    { \
        type *ptr = (type *)mmap(NULL, arena->size, \
                                PROT_READ | PROT_WRITE, \
                                MAP_PRIVATE | MAP_ANONYMOUS, \
                                -1, 0); \
        if (ptr != (type *)-1) \
        { \
            arena->data = ptr; \
            return ARENA_EOK; \
        } \
        return ARENA_EFAILED; \
    } \
    /* Allocate `nmemb` items of the arenas underlying type */ \
    static type *CONCAT2(___arena_##type, _alloc)(arena(type) *arena, const size_t nmemb) \
    { \
        const uintptr_t start_of_arena = (const uintptr_t)arena->data; \
        const uintptr_t end_of_arena = start_of_arena + arena->size; \
        typeof(arena->data) start_of_alloc = arena->data + arena->idx; \
        typeof(arena->data) end_of_alloc = start_of_alloc + nmemb; \
        /* Bounds check the arena */ \
        if (___arena_in_range(start_of_arena, end_of_arena, (const uintptr_t)start_of_alloc) || \
            ___arena_in_range(start_of_arena, end_of_arena, (const uintptr_t)end_of_alloc)) \
        { \
            /* Compute the new index and make sure it isn't smaller than the original index */ \
            const size_t new_idx = arena->idx + nmemb; \
            if (new_idx >= arena->idx) \
            { \
                arena->idx = new_idx; \
                assert((const uintptr_t)start_of_alloc % alignof(*arena->data) == 0); \
                return start_of_alloc; \
            } \
        } \
        /* Return NULL if the idx shrank, or the allocation start or end are outside of the arena */ \
        return NULL; \
    } \
    /* Reset the arena to it's initial state (post init) */ \
    static inline void CONCAT2(___arena_##type, _recycle)(arena(type) *arena) \
    { \
        arena->idx = 0; \
        /* FIXME: Be smarter with this. Clearing the entire arena is redundant if it wasn't all allocated */ \
        memset(arena->data, 0, arena->size); \
    } \
    /* Free the arena buffer */ \
    static inline void CONCAT2(___arena_##type, _free)(arena(type) *arena) \
    { \
        munmap(arena->data, arena->size); \
    }

#define decl_arena_instance(name, type) \
    static enum arena_error CONCAT3(___arena_##name, _##type, _init)(void); \
    static type *CONCAT3(___arena_##name, _##type, _alloc)(const size_t nmemb); \
    static void CONCAT3(___arena_##name, _##type, _recycle)(void); \
    static void CONCAT3(___arena_##name, _##type, _free)(void); \
    /* Instantiate the arena. Buffer must still be allocated with a call to init */ \
    static arena(type) name = { \
        .size = TYPED_ARENA_SIZE, \
        .idx = 0, \
        .data = NULL, \
        .init = CONCAT3(___arena_##name, _##type, _init), \
        .alloc = CONCAT3(___arena_##name, _##type, _alloc), \
        .recycle = CONCAT3(___arena_##name, _##type, _recycle), \
        .free = CONCAT3(___arena_##name, _##type, _free) \
    }; \
    /* Functions for this specific instance. They call into the functions for this specific type */ \
    static enum arena_error CONCAT3(___arena_##name, _##type, _init)(void) \
    { \
        return CONCAT2(___arena_##type, _init)(&name); \
    } \
    static type *CONCAT3(___arena_##name, _##type, _alloc)(const size_t nmemb) \
    { \
        return CONCAT2(___arena_##type, _alloc)(&name, nmemb); \
    } \
    static void CONCAT3(___arena_##name, _##type, _recycle)(void) \
    { \
        CONCAT2(___arena_##type, _recycle)(&name); \
    } \
    static void CONCAT3(___arena_##name, _##type, _free)(void) \
    { \
        CONCAT2(___arena_##type, _free)(&name); \
    }

#ifdef __cplusplus
}
#endif

#endif /* __TYPED_ARENA_H__ */
