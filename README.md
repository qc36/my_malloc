The C standard library includes 4 malloc-related library functions: malloc(), free(), calloc(), and
realloc(). In this assignment, you only need to implement versions of malloc() and free():

void *malloc(size_t size);
void free(void *ptr);

Please refer to the man pages for full descriptions of the expected operation for these functions.
Essentially, malloc() takes in a size (number of bytes) for a memory allocation, locates an
address in the program’s data region where there is enough space to fit the specified number of
bytes, and returns this address for use by the calling program. The free() function takes an
address (that was returned by a previous malloc operation) and marks that data region as
available again for use.

As you work through implementing malloc() and free(), you will discover that as memory
allocations and deallocations happen, you will sometimes free a region of memory that is
adjacent to other also free memory region(s). Your implementation is required to coalesce
in this situation by merging the adjacent free regions into a single free region of memory.
Hint: For implementing malloc(), you should become familiar with the sbrk() system call. This
system call is useful for: 1) returning the address that represents the current end of the
processes data segment (called program break), and 2) growing the size of the processes data
segment by the amount specified by “increment”.
void *sbrk(intptr_t increment);

In this assignment, you will develop a malloc implementation and study different allocation
policies. In Homework 2, you will make this implementation thread-safe.
Study of Memory Allocation Policies

Your task is to implement 2 versions of malloc and free, each based on a different strategy for
determining the memory region to allocate. The two strategies are:

1. First Fit: Examine the free space tracker (e.g. free list), and allocate an address from
the first free region with enough space to fit the requested allocation size.

2. Best Fit: Examine all of the free space information, and allocate an address from the
free region which has the smallest number of bytes greater than or equal to the
requested allocation size.

To implement your allocation strategies, you will create 4 functions:
//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);
//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

Note, that in all cases, a policy to minimize the size of the process’s data segment should be
used. In other words, if there is no free space that fits an allocation request, then sbrk() should
be used to create that space. However, you do not need to perform any type of garbage
collection (e.g. reducing the size of the process’s data segment, even if allocations at the top of
the data segment have been freed).

On free(), your implementation is required to merge the newly freed region with any currently
free adjacent regions. In other words, your bookkeeping data structure should not contain
multiple adjacent free regions, as this would lead to poor region selection during malloc.
