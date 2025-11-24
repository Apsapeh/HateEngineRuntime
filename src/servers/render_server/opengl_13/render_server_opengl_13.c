#include "render_server_opengl_13.h"
#include "ex_alloc/chunk_allocator.h"
#include "math/ivec2.h"
#include "math/mat4.h"
#include "platform/memory.h"
#include "servers/render_server/render_server.h"
#include "error.h"
#include "log.h"
#include "types/signal.h"
#include "types/types.h"
#include "types/vector.h"
#include <servers/render_context/render_context.h>
#include <string.h>

#include <servers/render_server/methods-signatures.h.gen>

#define GLAD_GL_IMPLEMENTATION
#include <glad_ogl13/gl.h>


#define INSTANCES_CHUNK_SIZE 128
#define INSTANCES_CHUNKS_COUNT 4

#define MESHES_CHUNK_SIZE 128
#define MESHES_CHUNKS_COUNT 4

#define BUFFERS_CHUNK_SIZE 512
#define BUFFERS_CHUNKS_COUNT 4


/* ================================ Render Task ============================== */
struct RenderServerRenderTask {
    RenderServerWorld* world;
    RenderServerViewport* viewport;
    RenderServerRenderTaskState state;
    i32 priority;
};

static vec_ptr g_renderTaskPtrs;
static boolean g_renderTasksNeedToSort = false;


/* ================================= Viewport ================================ */
enum ViewportType {
    ViewportTypeSurface = 0,
};

struct RenderServerViewport {
    unsigned char type; // enum ViewportType
    IVec2 pos;
    IVec2 size;
    union {
        struct {
            RenderContextSurface* surface;
        } surface;
    } data;
};


/* ==================================  World ================================= */
struct RenderServerWorld {
    int tmp;
};


/* ============================== Inner Structs ============================== */
struct Instance {
    RenderServerMeshHandle mesh;
    RenderServerMaterialHandle material;
    Mat4 transform;
};

struct Mesh {
    RenderServerBufferHandle vertices_buf;
    RenderServerBufferHandle indices_buf;
};

struct Buffer {
    void* ptr;
    //    u64 size;
    RenderServerBufferType buffer_type; // Vertex, Index, Normal, etc.
    RenderServerDataType data_type; // Float, Int, etc.
    RenderServerDataOwnMode own_mode; // Copy, Borrow, Ptr
};

/* =========================================================================== */


/* ============================= Global Varibales ============================= */
static boolean g_gladLoaded = false;

static ChunkMemoryAllocator g_instances;

/***** Mesh *****/
static ChunkMemoryAllocator g_meshes;

/***** Buffer *****/
static ChunkMemoryAllocator g_buffers;

/* ============================================================================ */

static void init_glad_cb(void* args, void* ctx) {
    if (!gladLoadGL(RenderContext.get_proc_addr)) {
        LOG_FATAL("OpenGL load error (glad)")
    }
    g_gladLoaded = true;
}

#define INIT_SUBSYSTEM_VARS(var_name, type, chunk_size, chunks_count)                                   \
    chunk_memory_allocator_constructor(&var_name, sizeof(type), chunk_size, chunks_count);


static boolean _init(void) {
    SignalCallbackHandler h = RenderContext.signal_connect("gl_context_created", init_glad_cb, NULL);
    if (h == 0) {
        LOG_ERROR_OR_DEBUG_FATAL("RenderServer::_init: Signal can't be connected");
        return false;
    }

    g_renderTaskPtrs = vec_ptr_init();

    INIT_SUBSYSTEM_VARS(g_instances, struct Instance, INSTANCES_CHUNK_SIZE, INSTANCES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_meshes, struct Mesh, MESHES_CHUNK_SIZE, MESHES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_buffers, struct Buffer, BUFFERS_CHUNK_SIZE, BUFFERS_CHUNKS_COUNT);

    return true;
}

#define QUIT_SUBSYSTEM_VARS(var_name) chunk_memory_allocator_destructor(&var_name);

static boolean _quit(void) {
    vec_ptr_free(&g_renderTaskPtrs);

    QUIT_SUBSYSTEM_VARS(g_instances);
    QUIT_SUBSYSTEM_VARS(g_meshes);
    QUIT_SUBSYSTEM_VARS(g_buffers);

    return true;
}

static int task_comp(const void* a, const void* b) {
    i32 arg1 = ((const RenderServerRenderTask*) a)->priority;
    i32 arg2 = ((const RenderServerRenderTask*) b)->priority;

    if (arg1 < arg2)
        return -1;
    if (arg1 > arg2)
        return 1;
    return 0;
}

static boolean _draw(double delta) {
    if (!g_gladLoaded)
        return false;

    const usize vec_size = g_renderTaskPtrs.size;
    struct RenderServerRenderTask** data = (struct RenderServerRenderTask**) g_renderTaskPtrs.data;


    if (g_renderTasksNeedToSort) {
        qsort(data, vec_size, sizeof(struct RenderServerRenderTask*), task_comp);
        g_renderTasksNeedToSort = false;
    }


    for (usize i = 0; i < vec_size; ++i) {
        struct RenderServerRenderTask* task = data[i];

        if (task->state == RENDER_SERVER_RENDER_TASK_STATE_ENABLED && task->viewport && task->world) {
            if (task->viewport->type == ViewportTypeSurface) {
                RenderContext.surface_make_current(task->viewport->data.surface.surface);
            }
            // render
            glClearColor(1.0f, 0.0f, 0.0, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);


            if (task->viewport->type == ViewportTypeSurface) {
                RenderContext.surface_present(task->viewport->data.surface.surface);
            }
        }
    }

    return true;
}


static RenderServerRenderTask* render_task_create(void) {
    RenderServerRenderTask* task = tmalloc(sizeof(RenderServerRenderTask));
    ERROR_ALLOC_CHECK(task, { return NULL; });
    task->viewport = NULL;
    task->world = NULL;
    task->state = RENDER_SERVER_RENDER_TASK_STATE_ENABLED;
    task->priority = 0;

    unsigned char status = vec_ptr_push_back(&g_renderTaskPtrs, task);
    if (!status) {
        tfree(task);
        LOG_ERROR_OR_DEBUG_FATAL("oentuhs");
        return NULL;
    }
    g_renderTasksNeedToSort = true;
    return task;
}

static boolean render_task_set_world(RenderServerRenderTask* task, RenderServerWorld* world) {
    ERROR_ARGS_CHECK_2(task, world, { return false; });
    task->world = world;
    return true;
}

static boolean render_task_set_viewport(RenderServerRenderTask* task, RenderServerViewport* viewport) {
    ERROR_ARGS_CHECK_2(task, viewport, { return false; });
    task->viewport = viewport;
    return true;
}

static boolean render_task_set_state(RenderServerRenderTask* task, RenderServerRenderTaskState state) {
    ERROR_ARGS_CHECK_2(task, state, { return false; });
    task->state = state;
    return true;
}

static boolean render_task_set_priority(RenderServerRenderTask* task, i32 priority) {
    ERROR_ARGS_CHECK_1(task, { return false; });
    task->priority = priority;
    return true;
}

static boolean render_task_destroy(RenderServerRenderTask* task) {
    ERROR_ARGS_CHECK_1(task, { return false; });
    tfree(task);

    const usize vec_size = g_renderTaskPtrs.size;
    struct RenderServerRenderTask** data = (struct RenderServerRenderTask**) g_renderTaskPtrs.data;
    for (usize i = 0; i < vec_size; ++i) {
        if (data[i] == task) {
            vec_ptr_erase(&g_renderTaskPtrs, i);
            g_renderTasksNeedToSort = true;
        }
    }
    return true;
}


static RenderServerViewport* viewport_surface_create(RenderContextSurface* surface) {
    RenderServerViewport* viewport = tmalloc(sizeof(RenderServerViewport));
    ERROR_ALLOC_CHECK(viewport, { return false; });
    viewport->type = ViewportTypeSurface;
    viewport->data.surface.surface = surface;
    viewport->pos = IVEC2_ZERO_M;
    viewport->size = IVEC2_ZERO_M;
    return viewport;
}

static boolean viewport_set_position(RenderServerViewport* viewport, IVec2 pos) {
    ERROR_ARGS_CHECK_1(viewport, { return false; });
    viewport->pos = pos;
    return true;
}

static boolean viewport_set_size(RenderServerViewport* viewport, IVec2 size) {
    ERROR_ARGS_CHECK_1(viewport, { return false; });
    viewport->size = size;
    return true;
}

static boolean viewport_destroy(RenderServerViewport* viewport) {
    ERROR_ARGS_CHECK_1(viewport, { return false; });
    tfree(viewport);
    return false;
}


static RenderServerWorld* world_create(void) {
    RenderServerWorld* world = tmalloc(sizeof(RenderServerWorld));
    ERROR_ALLOC_CHECK(world, { return false; });
    return world;
}

static boolean world_add_instance(RenderServerWorld* world, RenderServerInstanceHandle instance) {
    return false;
}

static boolean world_del_instance(RenderServerWorld* world, RenderServerInstanceHandle instance) {
    return false;
}

static boolean world_set_ambient_color(RenderServerWorld* world, Vec4 color) {
    return false;
}

static boolean world_draw(RenderServerWorld* world, RenderServerViewport* viewport) {
    return false;
}

static boolean world_destroy(RenderServerWorld* world) {
    ERROR_ARGS_CHECK_1(world, { return false; });
    tfree(world);
    return false;
}


static RenderServerInstanceHandle instance_create(void) {
    struct Instance* ptr = NULL;
    RenderServerInstanceHandle h = chunk_memory_allocator_alloc_mem(&g_instances, (void**) &ptr);
    if (h) {
        ptr->material = 0;
        ptr->mesh = 0;
        ptr->transform = MAT4_ONE_M;
    }
    return h;
}

#define INSTANCE_GET_PTR(fn_name)                                                                       \
    struct Instance* ptr = chunk_memory_allocator_get_real_ptr(&g_instances, instance);                 \
    if (!ptr) {                                                                                         \
        LOG_ERROR_OR_DEBUG_FATAL(                                                                       \
                "RenderServer(OpenGL 1.3)::" #fn_name ": Instance with this handle (%u) not found",     \
                instance                                                                                \
        );                                                                                              \
        set_error(ERROR_NOT_FOUND);                                                                     \
        return false;                                                                                   \
    }

static boolean instance_set_mesh(RenderServerInstanceHandle instance, RenderServerMeshHandle mesh) {
    ERROR_ARGS_CHECK_2(instance, mesh, { return false; });

    INSTANCE_GET_PTR("instance_set_mesh")
    ptr->mesh = mesh;

    return true;
}

static boolean instance_set_material(
        RenderServerInstanceHandle instance, RenderServerMaterialHandle material
) {
    ERROR_ARGS_CHECK_2(instance, material, { return false; });

    INSTANCE_GET_PTR("instance_set_material")
    ptr->material = material;

    return true;
}

static boolean instance_set_transform(RenderServerInstanceHandle instance, Mat4 transform) {
    ERROR_ARGS_CHECK_1(instance, { return false; });

    INSTANCE_GET_PTR("instance_set_mesh")
    ptr->transform = transform;

    return true;
}

static boolean instance_destroy(RenderServerInstanceHandle instance) {
    ERROR_ARGS_CHECK_1(instance, { return false; });
    return chunk_memory_allocator_free_mem(&g_instances, instance);
}


//
static RenderServerMeshHandle mesh_create(void) {
    struct Mesh* ptr = NULL;
    RenderServerMeshHandle h = chunk_memory_allocator_alloc_mem(&g_meshes, (void**) &ptr);
    if (h) {
        ptr->indices_buf = 0;
        ptr->vertices_buf = 0;
    }
    return h;
}

static boolean mesh_set_vertices_buffer(RenderServerMeshHandle hdl, RenderServerBufferHandle buffer) {
    ERROR_ARGS_CHECK_2(hdl, buffer, { return false; });

    struct Mesh* ptr = chunk_memory_allocator_get_real_ptr(&g_meshes, hdl);
    if (!ptr)
        return false;

    ptr->vertices_buf = buffer;

    return true;
}

static boolean mesh_set_indices_buffer(RenderServerMeshHandle hdl, RenderServerBufferHandle buffer) {
    ERROR_ARGS_CHECK_2(hdl, buffer, { return false; });

    struct Mesh* ptr = chunk_memory_allocator_get_real_ptr(&g_meshes, hdl);
    if (!ptr)
        return false;

    ptr->vertices_buf = buffer;

    return true;
}

static boolean mesh_destroy(RenderServerMeshHandle hdl) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    return chunk_memory_allocator_free_mem(&g_meshes, hdl);
}


static RenderServerBufferHandle buffer_create(
        RenderServerBufferType type, RenderServerBufferUsageHint usage_hint
) {
    if (usage_hint < RENDER_SERVER_BUFFER_USAGE_HINT_FIRST ||
        usage_hint > RENDER_SERVER_BUFFER_USAGE_HINT_LAST) {
        LOG_ERROR_OR_DEBUG_FATAL("RenderServer::buffer_create: Unknown 'usage_hint' - %d", usage_hint);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    if (type < RENDER_SERVER_BUFFER_TYPE_FIRST || type > RENDER_SERVER_BUFFER_TYPE_LAST) {
        LOG_ERROR_OR_DEBUG_FATAL("RenderServer::buffer_create: Unknown 'type' - %d", type);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    struct Buffer* ptr = NULL;
    RenderServerBufferHandle h = chunk_memory_allocator_alloc_mem(&g_buffers, (void**) &ptr);
    if (h) {
        ptr->ptr = NULL;
        ptr->own_mode = RENDER_SERVER_DATA_OWN_MODE_PTR;
        ptr->buffer_type = type;
    }
    return h;
}


static boolean buffer_set_data(
        RenderServerBufferHandle hdl, const void* data, u64 data_size, RenderServerDataType data_type,
        RenderServerDataOwnMode data_own_mode
) {
    ERROR_ARGS_CHECK_3(hdl, data, data_size, { return false; });

    if (data_own_mode < RENDER_SERVER_DATA_OWN_MODE_FIRST ||
        data_own_mode > RENDER_SERVER_DATA_OWN_MODE_LAST) {
        LOG_ERROR_OR_DEBUG_FATAL(
                "RenderServer::buffer_set_data: Unknown 'data_own_mode' - %d", data_own_mode
        );
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    // In OpenGL 1.x you don't have any GPU buffers, all data stored in the RAM.
    // All data you should to send to the GPU immediatly in an each frame

    struct Buffer* ptr = chunk_memory_allocator_get_real_ptr(&g_buffers, hdl);
    if (!ptr)
        return false;

    void* prev_ptr = ptr->ptr;

    if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_COPY) {
        void* new_data_ptr;
        if (ptr->own_mode == RENDER_SERVER_DATA_OWN_MODE_PTR)
            new_data_ptr = malloc(data_size);
        else
            new_data_ptr = trealloc(ptr->ptr, data_size);
        ERROR_ALLOC_CHECK(new_data_ptr, { return false; });
        memcpy(new_data_ptr, data, data_size);

        ptr->ptr = new_data_ptr;
    } else if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_BORROW) {
        // Yes, cast const void* -> void*. But this data is borowed
        ptr->ptr = (void*) data;
    } else if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_PTR) {
        ptr->ptr = (void*) data;
    } else {
        LOG_ERROR_OR_DEBUG_FATAL(
                "RenderServer::buffer_set_data: unknown 'data_own_mode' - '%c'", data_own_mode
        );
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    // If prev data is owned by buffer and now not used
    if (ptr->own_mode != RENDER_SERVER_DATA_OWN_MODE_PTR &&
        data_own_mode != RENDER_SERVER_DATA_OWN_MODE_COPY) {
        tfree(prev_ptr);
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
        tfree(ptr->ptr);
    }

    return chunk_memory_allocator_free_mem(&g_buffers, hdl);
}

// static SignalCallback


#define REGISTER(fn) render_server_backend_set_function(backend, #fn, (void (*)(void)) fn)

void render_server_opengl_13_backend_register(void) {
    RenderServerBackend* backend = render_server_backend_new();

    REGISTER(_init);
    REGISTER(_quit);

    REGISTER(_draw);

    REGISTER(render_task_create);
    REGISTER(render_task_set_world);
    REGISTER(render_task_set_viewport);
    REGISTER(render_task_set_state);
    REGISTER(render_task_destroy);

    REGISTER(viewport_surface_create);
    REGISTER(viewport_set_position);
    REGISTER(viewport_set_size);
    REGISTER(viewport_destroy);

    REGISTER(world_create);
    REGISTER(world_add_instance);
    REGISTER(world_del_instance);
    REGISTER(world_set_ambient_color);
    REGISTER(world_draw);
    REGISTER(world_destroy);

    REGISTER(instance_create);
    REGISTER(instance_set_mesh);
    REGISTER(instance_set_material);
    REGISTER(instance_set_transform);
    REGISTER(instance_set_transform);

    REGISTER(mesh_create);
    REGISTER(mesh_set_vertices_buffer);
    REGISTER(mesh_set_indices_buffer);
    REGISTER(mesh_destroy);

    REGISTER(buffer_create);
    REGISTER(buffer_set_data);
    REGISTER(buffer_destroy);

    render_server_register_backend("OpenGL 1.3", backend);
}
