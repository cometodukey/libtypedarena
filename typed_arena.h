#ifndef __TYPED_ARENA_H__
#define __TYPED_ARENA_H__

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#ifndef TYPED_ARENA_SIZE
    #define TYPED_ARENA_SIZE ((1 << 20) * 2) /* 2MiB */
#endif

#define CONCAT2(str1, str2) str1##str2
#define CONCAT3(str1, str2, str3) str1##str2##str3

enum arena_error
{
    ARENA_EOK,
    ARENA_EFAILED
};

#define arena(type) struct ___arena_##type

#define decl_arena_type(type) \
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
type *CONCAT2(___arena_##type, _alloc)(arena(type) *arena, const size_t nmemb) \
{ \
    const uintptr_t start_of_arena = (const uintptr_t)arena->data; \
    const uintptr_t end_of_arena = start_of_arena + arena->size; \
    typeof(arena->data) start_of_alloc = arena->data + arena->idx; \
    typeof(arena->data) end_of_alloc = start_of_alloc + nmemb; \
    if ((const uintptr_t)start_of_alloc < start_of_arena || \
        (const uintptr_t)start_of_alloc >= end_of_arena  || \
        (const uintptr_t)end_of_alloc < start_of_arena   || \
        (const uintptr_t)end_of_alloc >= end_of_arena) \
    { \
        return NULL; \
    } \
    arena->idx = (end_of_alloc - start_of_alloc) / sizeof(typeof(*arena->data)); \
    assert((const uintptr_t)start_of_alloc % alignof(*arena->data) == 0); \
    return start_of_alloc; \
} \
void CONCAT2(___arena_##type, _recycle)(arena(type) *arena) \
{ \
    arena->idx = 0; \
} \
void CONCAT2(___arena_##type, _free)(arena(type) *arena) \
{ \
    munmap(arena->data, arena->size); \
} \
enum arena_error CONCAT2(___arena_##type, _init)(arena(type) *arena) \
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
}

#define decl_arena_instance(name, type) \
static enum arena_error CONCAT3(___##name, _arena_##type, _init)(void); \
static type *CONCAT3(___##name, _arena_##type, _alloc)(const size_t nmemb); \
static void CONCAT3(___##name, _arena_##type, _recycle)(void); \
static void CONCAT3(___##name, _arena_##type, _free)(void); \
static arena(type) name = { \
    .size = TYPED_ARENA_SIZE, \
    .idx = 0, \
    .data = NULL, \
    .init = CONCAT3(___##name, _arena_##type, _init), \
    .alloc = CONCAT3(___##name, _arena_##type, _alloc), \
    .recycle = CONCAT3(___##name, _arena_##type, _recycle), \
    .free = CONCAT3(___##name, _arena_##type, _free) \
}; \
static enum arena_error CONCAT3(___##name, _arena_##type, _init)(void) \
{ \
    return CONCAT2(___arena_##type, _init)(&name); \
} \
static type *CONCAT3(___##name, _arena_##type, _alloc)(const size_t nmemb) \
{ \
    return CONCAT2(___arena_##type, _alloc)(&name, nmemb); \
} \
static void CONCAT3(___##name, _arena_##type, _recycle)(void) \
{ \
    CONCAT2(___arena_##type, _recycle)(&name); \
} \
static void CONCAT3(___##name, _arena_##type, _free)(void) \
{ \
    CONCAT2(___arena_##type, _free)(&name); \
}

#endif /* __TYPED_ARENA_H__ */
