#include "render_server_sync.h"
#include "ex_alloc/chunk_allocator.h"
#include "platform/memory.h"
#include "platform/mutex.h"
#include "servers/render_server/render_server.h"
#include "error.h"
#include "log.h"
#include "types/signal.h"
#include "types/types.h"
#include <servers/render_context/render_context.h>
#include <string.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad_ogl13/gl.h>


#define MESHES_CHUNK_SIZE 128
#define MESHES_CHUNKS_COUNT 4

#define BUFFERS_CHUNK_SIZE 512
#define BUFFERS_CHUNKS_COUNT 4


/* ============================== Inner Structs ============================== */
struct Mesh {
    RenderServerBufferHandle vertices_buf;
    RenderServerBufferHandle indices_buf;
    RenderServerMaterialHandle material;
};

struct Buffer {
    void* ptr;
    //    u64 size
    RenderServerBufferDataType data_type;
    RenderServerDataOwnMode own_mode;
};

/* =========================================================================== */

//

/* ============================= Global Varibales ============================= */
static boolean g_gladLoaded = false;

/***** Mesh *****/
static ChunkMemoryAllocator g_meshes;
static mutex_handle g_meshesMutex;

/***** Buffer *****/
static ChunkMemoryAllocator g_buffers;
static mutex_handle g_buffersMutex;

/* ============================================================================ */

static void init_glad_cb(void* args, void* ctx) {
    if (!gladLoadGL(RenderContext.get_proc_addr)) {
        LOG_FATAL("OpenGL load error (glad)")
    }
    g_gladLoaded = true;
}

#define INIT_SUBSYSTEM_VARS(var_name, type, chunk_size, chunks_count)                                   \
    chunk_memory_allocator_constructor(&var_name, sizeof(type), chunk_size, chunks_count);              \
    var_name##Mutex = mutex_new();


static boolean _init(void) {
    SignalCallbackHandler h = RenderContext.signal_connect("gl_context_created", init_glad_cb, NULL);
    if (h == 0) {
        return false;
    }

    INIT_SUBSYSTEM_VARS(g_meshes, struct Mesh, MESHES_CHUNK_SIZE, MESHES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_buffers, struct Buffer, BUFFERS_CHUNK_SIZE, BUFFERS_CHUNKS_COUNT);

    return true;
}

#define QUIT_SUBSYSTEM_VARS(var_name)                                                                   \
    chunk_memory_allocator_destructor(&var_name);                                                       \
    mutex_free(var_name##Mutex)

static boolean _quit(void) {
    QUIT_SUBSYSTEM_VARS(g_meshes);
    QUIT_SUBSYSTEM_VARS(g_buffers);

    return true;
}

static boolean frame_begin(void) {
    if (!g_gladLoaded)
        return false;

    glClearColor(1.0f, 0.0f, 0.0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}

static boolean frame_end(void) {
    return true;
}


//
static RenderServerMeshHandle mesh_create(void) {
    struct Mesh* ptr = NULL;
    RenderServerMeshHandle h = chunk_memory_allocator_alloc_mem(&g_meshes, (void**) &ptr);
    if (h) {
        ptr->indices_buf = 0;
        ptr->vertices_buf = 0;
        ptr->material = 0;
    }
    return h;
}

static boolean mesh_set_vertices_buffer(RenderServerMeshHandle hdl, RenderServerBufferHandle buffer) {
    return false;
}

static boolean mesh_set_indices_buffer(RenderServerMeshHandle hdl, RenderServerBufferHandle buffer) {
    return false;
}

static boolean mesh_destroy(void) {
    return false;
}


static RenderServerBufferHandle buffer_create(void) {
    struct Buffer* ptr = NULL;
    RenderServerBufferHandle h = chunk_memory_allocator_alloc_mem(&g_buffers, (void**) &ptr);
    if (h) {
        ptr->ptr = NULL;
    }
    return h;
}


static boolean buffer_set_data(
        RenderServerBufferHandle hdl, const void* data, u64 data_size, RenderServerDataType data_type,
        RenderServerDataOwnMode data_own_mode, RenderServerBufferUsageHint usage_hint
) {
    ERROR_ARGS_CHECK_3(hdl, data, data_size, { return false; });

    if (data_own_mode < RENDER_SERVER_DATA_OWN_MODE_FIRST ||
        data_own_mode > RENDER_SERVER_DATA_OWN_MODE_LAST) {
        LOG_ERROR("RenderServer::buffer_set_data: Unknown 'data_own_mode' - %d", data_own_mode);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    if (usage_hint < RENDER_SERVER_BUFFER_USAGE_HINT_FIRST ||
        usage_hint > RENDER_SERVER_BUFFER_USAGE_HINT_LAST) {
        LOG_ERROR("RenderServer::buffer_set_data: Unknown 'usage_hint' - %d", data_own_mode);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    // In OpenGL 1.x you haven't any buffer by default.
    // All data you send to GPU imediatly

    struct Buffer* ptr = chunk_memory_allocator_get_real_ptr(&g_buffers, hdl);
    if (!ptr)
        return false;

    if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_COPY) {
        void* new_data_ptr = trealloc(ptr->ptr, data_size);
        ERROR_ALLOC_CHECK(new_data_ptr, { return false; });
        memcpy(new_data_ptr, data, data_size);

        ptr->ptr = new_data_ptr;
    } else if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_BORROW) {
        // Yes, cast const void* -> void*. But we don't be change or free the data
        ptr->ptr = (void*) data;
    } else if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_PTR) {
        ptr->ptr = (void*) data;
    } else {
        LOG_ERROR("RenderServer::buffer_set_data: unknown 'data_own_mode' - '%c'", data_own_mode);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    ptr->data_type = data_type;
    ptr->own_mode = data_own_mode;
    return true;
}

static boolean buffer_destroy(RenderServerBufferHandle hdl) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });

    struct Buffer* ptr = chunk_memory_allocator_get_real_ptr(&g_buffers, hdl);
    if (!ptr)
        return false;

    if (ptr->own_mode == RENDER_SERVER_DATA_OWN_MODE_COPY ||
        ptr->own_mode == RENDER_SERVER_DATA_OWN_MODE_BORROW) {
        tfree(ptr);
    }

    return chunk_memory_allocator_free_mem(&g_buffers, hdl);
}

// static SignalCallback


#define REGISTER(fn) render_server_backend_set_function(backend, #fn, (void (*)(void)) fn)

void render_server_opengl_13_backend_register(void) {
    RenderServerBackend* backend = render_server_backend_new();

    REGISTER(_init);
    REGISTER(_quit);
    REGISTER(frame_begin);
    REGISTER(frame_end);

    REGISTER(mesh_create);
    REGISTER(mesh_set_vertices_buffer);
    REGISTER(mesh_set_indices_buffer);
    REGISTER(mesh_destroy);

    render_server_register_backend("OpenGL 1.3", backend);
}
