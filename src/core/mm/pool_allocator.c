#include "pool_allocator.h"
#include "core/error.h"
#include "core/types/types.h"
#include "core/platform/memory.h"

boolean pool_allocator_constructor(
        PoolAllocator* self, const u64 element_size, const u64 element_count_per_chunk
) {
    self->element_size = element_size;
    self->element_count_per_chunk = element_count_per_chunk;
    return true;
}

boolean pool_allocator_destructor(PoolAllocator* self) {
    return true;
}

PoolAllocator* pool_allocator_new(const u64 element_size, const u64 element_count_per_chunk) {
    PoolAllocator* self = tmalloc(sizeof(PoolAllocator));
    ERROR_ALLOC_CHECK(self, { return NULL; });

    if (!pool_allocator_constructor(self, element_size, element_count_per_chunk)) {
        tfree(self);
        return NULL;
    }

    return self;
}

boolean pool_allocator_free(PoolAllocator* self) {
    if (!pool_allocator_destructor(self)) {
        return false;
    }

    tfree(self);
    return true;
}


void* pool_allocator_alloc(PoolAllocator* self) {
    return tmalloc(self->element_size);
}

boolean pool_allocator_dealloc(PoolAllocator* self, void* ptr) {
    tfree(ptr);
    return true;
}
