#include "signal.h"
#include "error.h"
#include "ex_alloc/chunk_allocator.h"
#include "log.h"
#include "platform/memory.h"
#include "types/types.h"
#include "types/vector.h"

vector_template_impl(SignalCallback, SignalCallback);

boolean signal_constructor(Signal* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    self->data = vec_SignalCallback_init();
    return true;
}

boolean signal_destructor(Signal* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    vec_SignalCallback_free(&self->data);
    return true;
}

Signal* signal_new(void) {
    Signal* self = tmalloc(sizeof(Signal));
    ERROR_ALLOC_CHECK(self, { return NULL; });
    boolean s = signal_constructor(self);
    if (s) {
        return self;
    } else {
        tfree(self);
        return NULL;
    }
}


boolean signal_free(Signal* self) {
    if (signal_destructor(self)) {
        tfree(self);
        return true;
    } else {
        return false;
    }
}

boolean signal_emit(Signal* self, void* args) {
    ERROR_ARG_CHECK(self, { return false; });

    const SignalCallback* const data_end_ptr = self->data.data + self->data.size;
    for (const SignalCallback* ptr = self->data.data; ptr < data_end_ptr; ++ptr) {
        if (ptr->func != NULL) {
            ptr->func(args, ptr->ctx);
        }
    }

    return true;
}


// static find_in_vec(const Signal* const self, )

SignalCallbackHandler signal_connect(Signal* self, SignalCallbackFunc func, void* ctx) {
    ERROR_ARGS_CHECK_2(self, func, { return 0; });

    // Finding empty cell in the data vector
    const SignalCallback* const data_end_ptr = self->data.data + self->data.size;
    for (SignalCallback* ptr = self->data.data; ptr < data_end_ptr; ++ptr) {
        if (ptr->func == NULL) {
            ptr->func = func;
            ptr->ctx = ctx;
            return (SignalCallbackHandler) (data_end_ptr - ptr - 1);
        }
    }

    // Otherwise push func to the data vector
    const unsigned char s =
            vec_SignalCallback_push_back(&self->data, (SignalCallback) {.func = func, .ctx = ctx});
    if (!s) {
        LOG_ERROR("Pushing callback to the data vector is failed. I gues is the out-of-memory");
        set_error(ERROR_ALLOCATION_FAILED);
        return 0;
    }

    return self->data.size - 1;
}

boolean signal_disconnect(Signal* self, SignalCallbackHandler handler) {
    ERROR_ARGS_CHECK_2(self, handler, { return false; });
    if (handler >= self->data.size) {
        LOG_ERROR(
                "Handler is out from index boundary. 'handler' = %u, but bonddary is %u (incl.)",
                (unsigned int) handler, (unsigned int) (self->data.size - 1)
        );
        set_error(ERROR_NOT_FOUND);
        return false;
    }

    // SignalCallbackHandler shouldn't be changed on freing other callback

    SignalCallback* callback = &self->data.data[handler];
    if (callback->func == NULL) {
        LOG_ERROR("Handler (%u) already disconnected", (unsigned int) handler);
        set_error(ERROR_NOT_FOUND);
        return false;
    }

    callback->func = NULL;
    callback->ctx = NULL;

    // Removing disconnected handler from end of the data vector
    usize count_to_remove = 0;
    const SignalCallback* const data_start_ptr = self->data.data;
    for (SignalCallback* ptr = self->data.data + self->data.size; ptr-- > data_start_ptr;) {
        if (ptr->func == NULL)
            ++count_to_remove;
        else
            break;
    }

    if (count_to_remove != 0) {
        // It's always will be successful. There is no point in checking erasing
        vec_SignalCallback_erase_range(&self->data, self->data.size - count_to_remove, count_to_remove);
        // It's can fails on memory reallocation, but it's not broke any logic.
        vec_SignalCallback_shrink_to_fit(&self->data);
    }

    return true;
}
