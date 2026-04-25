// Created by Timofey Kirichenko
// 02 feb 2026

// #ifndef H_SRC_CORE_MM_POOL_ALLOCATOR
// #define H_SRC_CORE_MM_POOL_ALLOCATOR
//
//
// #endif

#define POOL_ALLOCATOR_ELEMENT_SIZE 8

#include <core/types/types.h>

/**
 * @api
 */
typedef struct {
    usize element_size; // static
    usize element_count_per_chunk; // static

    void* chunks;
    usize chunks_count;
} PoolAllocator;

struct PoolAllocatorChunk {
    usize data_range_begin;
    usize data_range_end;
    void* data;
};


/**
 *
 */
boolean pool_allocator_constructor(
        PoolAllocator* self, const u64 element_size, const u64 element_count_per_chunk
);

boolean pool_allocator_destructor(PoolAllocator* self);

/**
 * @param chunk_min_size Minimal size (in element) of a chunk. It will be aligned to 8. 27 -> 32;
 * 95 -> 96; 256 -> 256
 *
 * @param minimal_chunks_count Count of chunks will never be lower than that value
 *
 * @return Pointer to allocated memory on succes, 0 on failure
 *
 * @error "InvalidArgument"
 * @error "AllocationFailed"
 * @api
 */
PoolAllocator* pool_allocator_new(const u64 element_size, const u64 element_count_per_chunk);

/**
 *
 * @error "InvalidArgument"
 * @api
 */
boolean pool_allocator_free(PoolAllocator* self);

/**
 * @return 0 on failure
 *
 * @param [out] real_ptr Ptr to real memory. Can be NULL
 *
 * @error "InvalidArgument"
 * @error "AllocationFailed" Real memory can't be allocated
 * @api
 */
void* pool_allocator_alloc(PoolAllocator* self);

/**
 * @error "InvalidArgument"
 * @api
 */
boolean pool_allocator_dealloc(PoolAllocator* self, void* ptr);
