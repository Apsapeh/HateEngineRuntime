#include "render_server.h"
#include <core/error.h>
#include <core/log.h>
#include <core/types/vector.h>

/* ==> Backends <== */
#include "opengl_13/render_server_opengl_13.h"
#include <core/platform/mutex.h>
/* ================ */

#define COMMAND_QUEUE_ARRAY_SIZE 2
#define COMMAND_QUEUE_DEFAULT_SIZE 64

RenderServerBackend RenderServer;


// clang-format off
struct BackendPair {
    const char* name;
    RenderServerBackend* backend;
};

vector_template_def_static(backendPair, struct BackendPair)
vector_template_impl_static(backendPair, struct BackendPair)
static vec_backendPair g_registredBackends;


// Async buffers
struct CommandQueue {
    RenderServerCallDefferedFunc function;
    void* ctx;
};

vector_template_def_static(commandQueue, struct CommandQueue)
vector_template_impl_static(commandQueue, struct CommandQueue)

static vec_commandQueue g_commandQueueArr[COMMAND_QUEUE_ARRAY_SIZE];
static unsigned char g_currentCommandQueueIdx = 0;
static mutex_handle g_currentQueueMutex;

static boolean g_isLoaded = false;
static RenderServerThreadMode g_threadMode = 255;
// clang-format on


void render_server_init(void) {
    g_registredBackends = vec_backendPair_init();

    // Async global variables
    for (usize i = 0; i < COMMAND_QUEUE_ARRAY_SIZE; ++i) {
        g_commandQueueArr[i] = vec_commandQueue_init();
        vec_commandQueue_reserve(g_commandQueueArr, COMMAND_QUEUE_DEFAULT_SIZE);
    }

    g_currentQueueMutex = mutex_new();
    if (g_currentQueueMutex == NULL) {
        LOG_FATAL("render_server_init: Mutex can't be created with error: %s", get_error());
    }

    // Register default backend
    render_server_opengl_13_backend_register();
}

void render_server_exit(void) {
    for (usize i = 0; i < g_registredBackends.size; i++)
        render_server_backend_free(g_registredBackends.data[i].backend);

    for (usize i = 0; i < COMMAND_QUEUE_ARRAY_SIZE; ++i) {
        vec_commandQueue_free(&g_commandQueueArr[i]);
    }

    mutex_free(g_currentQueueMutex);

    vec_backendPair_free(&g_registredBackends);
}

boolean render_server_register_backend(const char* name, RenderServerBackend* backend) {
    ERROR_ARGS_CHECK_2(name, backend, { return false; });

    for (usize i = 0; i < g_registredBackends.size; i++) {
        if (strcmp(g_registredBackends.data[i].name, name) == 0) {
            LOG_ERROR("Backend with name '%s' already registered", name);
            set_error(ERROR_ALREADY_EXISTS);
            return false;
        }
    }

    vec_backendPair_push_back(&g_registredBackends, (struct BackendPair) {name, backend});

    return true;
}

boolean render_server_load_backend(const char* name, RenderServerThreadMode th_mode) {
    ERROR_ARGS_CHECK_1(name, { return false; });

    if (th_mode < RENDER_SERVER_THREAD_MODE_FIRST || th_mode > RENDER_SERVER_THREAD_MODE_LAST) {
        LOG_ERROR_OR_DEBUG_FATAL("render_server_load_backend: Unknown thread mode: %d", (int) th_mode);

        set_error(ERROR_INVALID_ARGUMENT);
        th_mode = RENDER_SERVER_THREAD_MODE_SYNC;
    }

    if (g_isLoaded) {
        LOG_WARN_OR_DEBUG_FATAL("(render_server_load_backend) Window server already loaded");
        set_error(ERROR_INVALID_STATE);
        return false;
    }

    for (usize i = 0; i < g_registredBackends.size; i++) {
        if (strcmp(g_registredBackends.data[i].name, name) == 0) {
            RenderServer = *g_registredBackends.data[i].backend;
            g_isLoaded = true;
            g_threadMode = th_mode;
            return true;
        }
    }

    LOG_ERROR_OR_DEBUG_FATAL("render_server_load_backend: Backend with name '%s' not found", name);
    set_error(ERROR_NOT_FOUND);
    return false;
}

boolean render_server_is_loaded(void) {
    return g_isLoaded;
}

RenderServerThreadMode render_server_get_thread_mode(void) {
    return g_threadMode;
}

/* ====================> Frame pipeline functions <==================== */
void render_server_begin_frame(void) {

    // Swap command queue
    const u8 exec_queue_idx = g_currentCommandQueueIdx;

    mutex_lock(g_currentQueueMutex);
    g_currentCommandQueueIdx = (exec_queue_idx + 1) % COMMAND_QUEUE_ARRAY_SIZE;
    vec_commandQueue_clear(&g_commandQueueArr[g_currentCommandQueueIdx]);
    mutex_unlock(g_currentQueueMutex);

    const usize queue_size = g_commandQueueArr[exec_queue_idx].size; // Some optimization
    const struct CommandQueue* queue = g_commandQueueArr[exec_queue_idx].data;
    for (usize i = 0; i < queue_size; ++i) {
        queue->function(queue->ctx);
    }
}

void render_server_end_frame(void) {
}

/* ====================> RenderServer Async functions <==================== */
boolean call_deferred_render_thread(RenderServerCallDefferedFunc function, void* ctx) {
    ERROR_ARGS_CHECK_1(function, { return false; });

    if (g_threadMode == RENDER_SERVER_THREAD_MODE_SYNC) {
        // Call immediatly
        function(ctx);
    } else if (g_threadMode == RENDER_SERVER_THREDA_MODE_ASYNC) {
        // Add to call queue
        mutex_lock(g_currentQueueMutex);
        unsigned char result = vec_commandQueue_push_back(
                &g_commandQueueArr[g_currentCommandQueueIdx],
                (struct CommandQueue) {.function = function, .ctx = ctx}
        );
        mutex_unlock(g_currentQueueMutex);

        if (!result) {
            LOG_ERROR(
                    "call_deferred_render_thread: The function can't be added, some internal error "
                    "occured when adding to the vector, probably a memory allocation error"
            );
            set_error(ERROR_ALLOCATION_FAILED);
            return false;
        }
    } else {
        // Should be unreachable
        LOG_ERROR_OR_DEBUG_FATAL("Yo, what the fuck?")
        return false;
    }

    return true;
}


/* ====================> RenderServerBackend functions <==================== */
static boolean backend_set_get(
        RenderServerBackend* backend, const char* name, void (**fn)(void), unsigned char is_set
);

RenderServerBackend* render_server_backend_new(void) {
    RenderServerBackend* backend = tmalloc(sizeof(RenderServerBackend));
    ERROR_ALLOC_CHECK(backend, { return NULL; });

    // TODO: Add default functions

    return backend;
}

boolean render_server_backend_free(RenderServerBackend* backend) {
    ERROR_ARG_CHECK(backend, { return false; });
    tfree(backend);
    return true;
}

boolean render_server_backend_set_function(
        RenderServerBackend* backend, const char* name, fptr function
) {
    ERROR_ARGS_CHECK_3(backend, name, function, { return false; });
    return backend_set_get(backend, name, (void (**)(void)) function, 1);
}

fptr render_server_backend_get_function(RenderServerBackend* backend, const char* name) {
    ERROR_ARGS_CHECK_2(backend, name, { return false; });
    fptr function = NULL;
    backend_set_get(backend, name, &function, 0);
    return function;
}


static boolean backend_set_get(
        RenderServerBackend* backend, const char* name, void (**fn)(void), unsigned char is_set
) {
#include "setget-pairs-table.h.gen"

    for (usize i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
        if (!strcmp(name, pairs[i].name)) {
            if (is_set)
                *pairs[i].function = (void (*)(void)) fn;
            else
                *fn = *pairs[i].function;
            return true;
        }
    }

    LOG_ERROR("Unknown function name: %s", name);
    set_error(ERROR_NOT_FOUND);
    return false;
}
