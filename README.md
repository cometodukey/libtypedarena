# libtypedarena

A simple typed arena allocator in C.

* Arenas are uniquely tagged with their identifier and type at compile time
* Fast allocations `O(1)`
* No need to free individual allocations
* Arenas can be recycled

## Usage

* `TYPED_ARENA_SIZE`. Set this macro to change the arena size. Default is 2MiB
* `decl_arena_type(type)`. Declare an arena of a given type. Must be done in the global scope
* `decl_arena_instance(name, type)`. Create an instance of an arena with name and type. Must be done in the global scope
* `arena.init()`. Initialise the arena
* `arena.alloc(nmemb)`. Allocate `n` members of the underlying type on the arena
* `arena.recycle()`. Reset the arena
* `arena.free()`. Free the entire arena

## Limitations

* Arenas cannot currently be declared in any non-global scope
* Non-portable. This depends on POSIX `mmap`/`munmap` and GNU `typeof`
* Arena types must be a valid C identifier
    * Anything behind the `union`/`struct`/`enum` namespaces must be typedeffed
    * Pointer types must be typedeffed e.g. `typedef char * char_ptr;`
* Every library definition prefixed with `___arena` is internal and should not be used directly
* Thread and signal unsafe
* It's not currently possible to transmute an arena from one type to another

## Building

This is a header only library. Just drop the header onto your projects include path.
