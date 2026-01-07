#pragma once

#include <core/types/types.h>
#include <core/types/vector.h>


// TODO: Rewrite to typed version

/**
 * @brief Virtual memory ptr
 *
 * @warning 0 is not valid value
 *
 * @api
 */
typedef u64 chunk_allocator_ptr;

/*
 * Chunk stores as SOA.
 * vec_ptr chunks is an array of allocated chunks.
 * chunk_usage_bitfield is an array of bitfields. Each chunk contains u8[chunk_size / 8]
 */


/**
 * @brief Chunk memory allocator for elements with fixed type and size
 *
 * Specially optimized allocator for small types, like thousands allocations/freeing of int in each frame
 *
 * @api
 */
typedef struct ChunkMemoryAllocator {
    vec_ptr chunks;
    vec_u8 chunks_bitfield;
    u32 chunk_bitfield_size; // Size of bitfield in bytes
    u32 chunk_element_count; // Size of chunk in element
    u32 element_size; // Size of each element
    u32 freed_chunks; // Number of freed chunks
    u8 minimal_chunks_count;
} ChunkMemoryAllocator;


/**
 *
 */
boolean chunk_memory_allocator_constructor(
        ChunkMemoryAllocator* self, const u32 element_size, const u32 chunk_min_size,
        const u8 minimal_chunks_count
);

boolean chunk_memory_allocator_destructor(ChunkMemoryAllocator* self);

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
ChunkMemoryAllocator* chunk_memory_allocator_new(
        const u32 element_size, const u32 chunk_min_size, const u8 minimal_chunks_count
);

/**
 *
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_free(ChunkMemoryAllocator* this);

/**
 * @return 0 on failure
 *
 * @param [out] real_ptr Ptr to real memory. Can be NULL
 *
 * @error "InvalidArgument"
 * @error "AllocationFailed" Real memory can't be allocated
 * @api
 */
chunk_allocator_ptr chunk_memory_allocator_alloc_mem(ChunkMemoryAllocator* this, void** real_ptr);

/**
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_free_mem(ChunkMemoryAllocator* this, chunk_allocator_ptr ptr);

/**
 * @brief Return a real pointer to data
 * @warning You don't own this pointer
 *
 * @error "InvalidArgument"
 * @api
 */
void* chunk_memory_allocator_get_real_ptr(ChunkMemoryAllocator* this, chunk_allocator_ptr ptr);

/**
 * @brief Return a real pointer to data. Fast and may cause an segfault or other memory errors
 * @warning You don't own this pointer
 *
 * @api
 */
void* chunk_memory_allocator_get_real_ptr_unsafe(ChunkMemoryAllocator* this, chunk_allocator_ptr ptr);

/* ======================> Iterator <====================== */

/**
 * @brief Iterator over all allocated memory
 *
 * @api
 */
typedef struct ChunkMemoryAllocatorIter {
    u8 meta; // 1 << 0 - is last, 1 << 1 is empty
    u8 bit_idx;
    u32 chunk_idx;
    u32 byte_idx;
    ChunkMemoryAllocator* cma;
    void* ptr;
} ChunkMemoryAllocatorIter;

boolean chunk_memory_allocator_iter_constructor(
        ChunkMemoryAllocatorIter* self, ChunkMemoryAllocator* cma
);
boolean chunk_memory_allocator_iter_destructor(ChunkMemoryAllocatorIter* self);

/**
 * @brief Create new Iterator over all allocated memory
 *
 * @error "InvalidArgument"
 * @error "AllocationFailed"
 * @api
 */
ChunkMemoryAllocatorIter* chunk_memory_allocator_iter_new(ChunkMemoryAllocator* cma);

/**
 * @brief Free Iterator. Use only with "chunk_memory_allocator_iter_new"
 *
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_iter_free(ChunkMemoryAllocatorIter* self);

/**
 * @brief Next element
 *
 * @param real_ptr Pointer to real data. Can be NULL
 *
 * @return true if next element is exist, false if is not (is the last element)
 *
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_iter_next(ChunkMemoryAllocatorIter* self, void** real_ptr);

/**
 * @brief Return a real pointer to data
 * @warning You to owned this pointer
 *
 * @error "InvalidArgument"
 * @api
 */
void* chunk_memory_allocator_iter_get_real_ptr(ChunkMemoryAllocatorIter* self);

/**
 * @brief Is the begin
 *
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_iter_is_begin(ChunkMemoryAllocatorIter* self);


/**
 * @brief Is the end
 *
 * @error "InvalidArgument"
 * @api
 */
boolean chunk_memory_allocator_iter_is_end(ChunkMemoryAllocatorIter* self);
