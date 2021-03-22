#include <assert.h>
#include <stdio.h>

/* Override the default size */
#define TYPED_ARENA_SIZE 1024

#include "typed_arena.h"

typedef char * char_ptr;

/* Declare the `char_ptr` arena type */
decl_arena_type(char_ptr)

/* Create an instance of a `char_ptr` arena called `my_arena` */
decl_arena_instance(my_arena, char_ptr)

int main(int argc, char **argv)
{
    /* Initialise the arena */
    if (my_arena.init() == ARENA_EFAILED)
    {
        return 1;
    }

    /* Request an allocation of `argc` char_ptr's */
    char **new_argv = my_arena.alloc(argc);
    if (new_argv == NULL)
    {
        return 1;
    }

    /* Copy argv into the new buffer */
    for (int i = 0; i < argc; i++)
    {
        new_argv[i] = argv[i];
    }

    /* Read back out of the buffer */
    for (int i = 0; i < argc; i++)
    {
        printf("%s ", new_argv[i]);
    }

    putchar('\n');

    /* Recycle the arena */
    my_arena.recycle();

    /* The arena has been reset, so another allocation of `argc` members
       should result in the same pointer as `new_argv`.

       Note: After recycling an arena, it is unsafe to use any allocations
             from before the reset */
    assert(my_arena.alloc(argc) == new_argv);

    /* Free the arena */
    my_arena.free();

    return 0;
}
