#include "chunk_allocator.h"
#include <types/vector.h>
#include <platform/memory.h>
#include <log.h>
#include <error.h>
#include "types/types.h"

// #define CHUNK_COUNT_DEFAULT 4


boolean chunk_memory_allocator_constructor(
        ChunkMemoryAllocator* self, const u32 element_size, const u32 chunk_min_size,
        const u8 minimal_chunks_count
) {
    ERROR_ARG_CHECK(self, { return false; });

    if (element_size == 0 || chunk_min_size == 0 || minimal_chunks_count == 0) {
        LOG_ERROR_OR_DEBUG_FATAL(
                "chung_memory_allocator_new: 'elemnt_size' or 'chunk_min_size' or "
                "'minimal_chunks_count' is 0"
        );
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }


    self->minimal_chunks_count = minimal_chunks_count;
    self->freed_chunks = 0;
    self->element_size = element_size;
    self->chunk_bitfield_size = chunk_min_size >> 3;
    if (chunk_min_size % 8 != 0)
        self->chunk_bitfield_size++;
    self->chunk_element_count = self->chunk_bitfield_size << 3;

    // Init chunk array
    self->chunks = vec_ptr_init();
    vec_ptr_reserve(&self->chunks, minimal_chunks_count);

    for (usize i = 0; i < minimal_chunks_count; i++) {
        void* chunk = tmalloc(self->chunk_element_count * self->element_size);
        ERROR_ALLOC_CHECK(chunk, {
            for (usize j = 0; j < i; j++) {
                tfree(self->chunks.data[j]);
            }
            vec_ptr_free(&self->chunks);
            return false;
        });
        vec_ptr_push_back(&self->chunks, chunk);
    }

    // Init bitfield array
    self->chunks_bitfield = vec_u8_init();
    vec_u8_reserve(&self->chunks_bitfield, minimal_chunks_count * self->chunk_bitfield_size);

    for (usize i = 0; i < minimal_chunks_count * self->chunk_bitfield_size; i++) {
        vec_u8_push_back(&self->chunks_bitfield, 0);
    }

    return true;
}

boolean chunk_memory_allocator_destructor(ChunkMemoryAllocator* self) {
    ERROR_ARG_CHECK(self, { return false; });
    for (usize i = 0; i < self->chunks.size; i++) {
        if (self->chunks.data[i]) // free(NULL) is safe by standard, but can fail on exotic systems
                                  // (PalmOS, 3BSD)
            tfree(self->chunks.data[i]);
    }
    vec_ptr_free(&self->chunks);
    vec_u8_free(&self->chunks_bitfield);
    set_error(ERROR_SUCCESS);
    return true;
}

/**
 * @param chunk_min_size Minimal size (in element) of a chunk.
 * It will be aligned to 8. 27 -> 32; 95 -> 96; 256 -> 256
 *
 * @api
 */
ChunkMemoryAllocator* chunk_memory_allocator_new(
        const u32 element_size, const u32 chunk_min_size, const u8 minimal_chunks_count
) {
    ChunkMemoryAllocator* allocator = tmalloc(sizeof(ChunkMemoryAllocator));
    ERROR_ALLOC_CHECK(allocator, { return NULL; });

    boolean success = chunk_memory_allocator_constructor(
            allocator, element_size, chunk_min_size, minimal_chunks_count
    );

    if (success)
        return allocator;
    else {
        tfree(allocator);
        return NULL;
    }
}

/**
 * @api
 */
boolean chunk_memory_allocator_free(ChunkMemoryAllocator* self) {
    if (chunk_memory_allocator_destructor(self)) {
        tfree(self);
        return true;
    } else
        return false;
}

/**
 * @api
 */
chunk_allocator_ptr chunk_memory_allocator_alloc_mem(ChunkMemoryAllocator* this) {
    ERROR_ARG_CHECK(this, return 0;);
    // Find a free chunk
    for (usize c_i = 0; c_i < this->chunks.size; c_i++) {
        if (this->chunks.data[c_i] == NULL) {
            this->chunks.data[c_i] = tmalloc(this->chunk_element_count * this->element_size);
            ERROR_ALLOC_CHECK(this->chunks.data[c_i], { return 0; });
            this->chunks_bitfield.data[c_i * this->chunk_bitfield_size] = 1;
            return c_i * this->chunk_element_count + 1;
        }

        u8* bitfield = &this->chunks_bitfield.data[c_i * this->chunk_bitfield_size];
        for (usize b_i = 0; b_i < this->chunk_bitfield_size; b_i++) {
            if (bitfield[b_i] != 0xFF) {
#if (defined(__GNUC__) || defined(__clang__))
                usize free_bit = __builtin_ctz(~bitfield[b_i]);
                bitfield[b_i] |= 1 << free_bit;
                return c_i * this->chunk_element_count + b_i * 8 + free_bit + 1;
#else
                usize free_bit = 0;
                while (bitfield[b_i] & (1 << free_bit)) {
                    free_bit++;
                }
                bitfield[b_i] |= 1 << free_bit;
                return c_i * this->chunk_element_count + b_i * 8 + free_bit + 1;
#endif
            }
        }
    }

    // No free chunk found, allocate a new one
    void* chunk = tmalloc(this->chunk_element_count * this->element_size);
    ERROR_ALLOC_CHECK(chunk, { return 0; });
    if (!vec_ptr_push_back(&this->chunks, chunk)) {
        tfree(chunk);
        LOG_ERROR("Chunk vector add failed");
        set_error(ERROR_ALLOCATION_FAILED);
        return 0;
    }

    if (this->freed_chunks > 0) {
        this->freed_chunks--;
    }

    for (usize i = 0; i < this->chunk_bitfield_size; i++) {
        if (!vec_u8_push_back(&this->chunks_bitfield, 0)) {
            tfree(chunk);
            vec_ptr_pop_back(&this->chunks);
            vec_u8_erase_range(&this->chunks_bitfield, this->chunks_bitfield.size - i, i);
            LOG_ERROR("Chunk vector bitfield add failed");
            set_error(ERROR_ALLOCATION_FAILED);
            return 0;
        }
    }

    this->chunks_bitfield.data[(this->chunks.size - 1) * this->chunk_bitfield_size] = 1;
    return (this->chunks.size - 1) * this->chunk_element_count + 1;
}

/**
 * @api
 */
boolean chunk_memory_allocator_free_mem(ChunkMemoryAllocator* this, chunk_allocator_ptr ptr) {
    ERROR_ARGS_CHECK_2(this, ptr, );

    --ptr;
    if (ptr >= this->chunk_element_count * this->chunks.size) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid pointer");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    const usize chunk_index = ptr / this->chunk_element_count;
    const usize element_in_chunk = ptr % this->chunk_element_count;
    const usize bitfield_index = chunk_index * this->chunk_bitfield_size;
    const usize byte_index = element_in_chunk / 8;
    const usize bit_index = element_in_chunk % 8;
    const u8 bit_mask = 1 << bit_index;

    this->chunks_bitfield.data[bitfield_index + byte_index] &= ~bit_mask;

    boolean is_mid_of_chunks = false;
    for (usize c_i = this->chunks.size; c_i-- > this->minimal_chunks_count;) {
        boolean is_empty = true;
        for (usize i = 0; i < this->chunk_bitfield_size; ++i) {
            if (this->chunks_bitfield.data[c_i * this->chunk_bitfield_size + i] != 0) {
                is_empty = false;
                is_mid_of_chunks = true;
                break;
            }
        }

        if (!is_empty)
            continue;

        // Free chunk, but don't remove it from the array
        if (this->chunks.data[c_i] != NULL) { // If chunk yet is not freed
            tfree(this->chunks.data[c_i]);
            this->chunks.data[c_i] = NULL;
        }

        if (is_mid_of_chunks)
            continue;

        // If chunk is last in array, remove it
        vec_ptr_pop_back(&this->chunks);
        vec_u8_erase_range(
                &this->chunks_bitfield, c_i * this->chunk_bitfield_size, this->chunk_bitfield_size
        );
        ++this->freed_chunks;
    }

    if (this->freed_chunks > this->chunks.size) {
        vec_ptr_shrink_to_fit(&this->chunks);
        vec_u8_shrink_to_fit(&this->chunks_bitfield);
        this->freed_chunks = 0;
    }
    return true;
}

/**
 * @brief Return a real pointer to data
 * @warning You to owned this pointer
 *
 * @api
 */
void* chunk_memory_allocator_get_real_ptr(ChunkMemoryAllocator* this, chunk_allocator_ptr ptr) {
    ERROR_ARGS_CHECK_2(this, ptr, { return NULL; });

    --ptr;

    if (ptr >= this->chunk_element_count * this->chunks.size) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid pointer");
        set_error(ERROR_INVALID_ARGUMENT);
        return NULL;
    }

    const usize chunk_index = ptr / this->chunk_element_count;
    const usize element_in_chunk = ptr % this->chunk_element_count;
    const usize bitfield_index = chunk_index * this->chunk_bitfield_size;
    const usize byte_index = element_in_chunk / 8;
    const usize bit_index = element_in_chunk % 8;
    const u8 bit_mask = 1 << bit_index;

    if (this->chunks_bitfield.data[bitfield_index + byte_index] & bit_mask) {
        return (u8*) this->chunks.data[chunk_index] + element_in_chunk * this->element_size;
    }
    return NULL;
}


/* ====================================> Iterator <==================================== */

#define IS_END_ITER 1 << 0

boolean chunk_memory_allocator_iter_constructor(
        ChunkMemoryAllocatorIter* self, ChunkMemoryAllocator* cma
) {
    ERROR_ARGS_CHECK_2(self, cma, { return false; });
    self->chunk_idx = 0;
    self->byte_idx = 0;
    self->bit_idx = -1;
    self->meta = 0;
    self->cma = cma;
    self->ptr = NULL; // If NULL - is begin iter

    return true;
}

boolean chunk_memory_allocator_iter_destructor(ChunkMemoryAllocatorIter* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    return true;
}

ChunkMemoryAllocatorIter* chunk_memory_allocator_iter_new(ChunkMemoryAllocator* cma) {
    ChunkMemoryAllocatorIter* allocator = tmalloc(sizeof(ChunkMemoryAllocatorIter));
    ERROR_ALLOC_CHECK(allocator, { return NULL; });

    boolean success = chunk_memory_allocator_iter_constructor(allocator, cma);

    if (success)
        return allocator;
    else {
        tfree(allocator);
        return NULL;
    }
}

boolean chunk_memory_allocator_iter_free(ChunkMemoryAllocatorIter* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    if (chunk_memory_allocator_iter_destructor(self)) {
        tfree(self);
        return true;
    } else {
        return false;
    }
}

boolean chunk_memory_allocator_iter_next(ChunkMemoryAllocatorIter* self, void** real_ptr) {
    ERROR_ARGS_CHECK_1(self, { return false; });

    if (self->meta & IS_END_ITER)
        return false;

    ChunkMemoryAllocator* cma = self->cma;

    u32 start_byte_idx = self->byte_idx;
    u32 start_bit_idx = self->bit_idx + 1;

    for (usize chunk_idx = self->chunk_idx; chunk_idx < cma->chunks.size; ++chunk_idx) {
        const usize chunk_offset = chunk_idx * cma->chunk_bitfield_size;
        // If we iterate just from prev iter then byte_idx = prev iter byte idx
        // Otherwise if we iterate in next chunk byte_idx = 0
        usize byte_idx = start_byte_idx;
        start_byte_idx = 0;
        for (; byte_idx < cma->chunk_bitfield_size; ++byte_idx) {
            const usize bitfield_byte_idx = chunk_offset + byte_idx;
            u8 byte = cma->chunks_bitfield.data[bitfield_byte_idx];

            byte >>= start_bit_idx;
            if (byte == 0) {
                start_bit_idx = 0;
                continue;
            }

#if (defined(__GNUC__) || defined(__clang__))
            self->chunk_idx = chunk_idx;
            self->byte_idx = byte_idx;
            self->bit_idx = start_bit_idx + __builtin_ctz(byte);
#else
            for (u8 i = 0; byte; ++i) {
                if (byte & (1 << i)) {
                    self->chunk_idx = chunk_idx;
                    self->byte_idx = byte_idx;
                    self->bit_idx = start_bit_idx + i;
                    break;
                }
                byte >>= 1;
            }

#endif
            start_bit_idx = 0;

            self->ptr = (u8*) cma->chunks.data[self->chunk_idx] +
                        (self->byte_idx * 8 + self->bit_idx) * cma->element_size;

            if (real_ptr)
                *real_ptr = self->ptr;
            return true;
        }
    }

    self->meta = IS_END_ITER;
    return false;
}

void* chunk_memory_allocator_iter_get_real_ptr(ChunkMemoryAllocatorIter* self) {
    ERROR_ARGS_CHECK_1(self, { return NULL; });

    if (self->ptr == NULL) {
        LOG_ERROR_OR_DEBUG_FATAL(
                "chunk_memory_allocator_iter_get_real_ptr: You must first call "
                "'chunk_memory_allocator_iter_next'"
        );
        set_error(ERROR_INVALID_STATE);
        return NULL;
    }

    if (self->meta & IS_END_ITER) {
        LOG_ERROR_OR_DEBUG_FATAL(
                "chunk_memory_allocator_iter_get_real_ptr: Iterator is ended "
                "'chunk_memory_allocator_iter_next'"
        );
        set_error(ERROR_INVALID_STATE);
        return NULL;
    }

    return self->ptr;
}

boolean chunk_memory_allocator_iter_is_begin(ChunkMemoryAllocatorIter* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    return self->ptr == NULL;
}

boolean chunk_memory_allocator_iter_is_end(ChunkMemoryAllocatorIter* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    return self->meta & IS_END_ITER;
}
