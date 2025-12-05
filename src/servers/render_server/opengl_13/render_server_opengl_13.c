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

#ifdef HE_DEBUG
    #define CMA_PTR(cma, hdl) chunk_memory_allocator_get_real_ptr(cma, hdl);
#else
    #define CMA_PTR(cma, hdl) chunk_memory_allocator_get_real_ptr_unsafe(cma, hdl);
#endif


/* ================================ Render Task ============================== */
struct RenderServerRenderTask {
    RenderServerWorld* world;
    RenderServerViewport* viewport;
    RenderServerCamera* camera;
    RenderServerRenderTaskState state;
    i32 priority;
};

static vec_ptr g_renderTaskPtrs;
static boolean g_renderTaskPtrsNeedToSort = false;

static vec_ptr g_renderTaskUniqueSurfaces;
static boolean g_renderTaskUniqueSurfacesNeedToRebuild = false;


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

/* ================================= Viewport ================================ */
struct RenderServerCamera {
    Mat4 view;
    Mat4 projection;
};


/* ==================================  World ================================= */

vector_template_def_static(instanceHandle, usize); // RenderServerInstanceHandle
vector_template_impl_static(instanceHandle, usize); //   RenderServerInstanceHandle

struct RenderServerWorld {
    vec_instanceHandle instances; // TODO: change to hash map
};


/* ============================== Inner Structs ============================== */
struct Instance {
    usize mesh; // RenderServerMeshHandle
    usize material; // RenderServerMaterialHandle
    Mat4 transform;
};

struct Mesh {
    usize vertices_buf; // RenderServerBufferHandle
    usize indices_buf; // RenderServerBufferHandle
};

struct Buffer {
    void* ptr;
    u64 size;
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
    g_renderTaskUniqueSurfaces = vec_ptr_init();

    INIT_SUBSYSTEM_VARS(g_instances, struct Instance, INSTANCES_CHUNK_SIZE, INSTANCES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_meshes, struct Mesh, MESHES_CHUNK_SIZE, MESHES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_buffers, struct Buffer, BUFFERS_CHUNK_SIZE, BUFFERS_CHUNKS_COUNT);

    return true;
}

#define QUIT_SUBSYSTEM_VARS(var_name) chunk_memory_allocator_destructor(&var_name);

static boolean _quit(void) {
    vec_ptr_free(&g_renderTaskPtrs);
    vec_ptr_free(&g_renderTaskUniqueSurfaces);

    QUIT_SUBSYSTEM_VARS(g_instances);
    QUIT_SUBSYSTEM_VARS(g_meshes);
    QUIT_SUBSYSTEM_VARS(g_buffers);

    return true;
}

static int task_comp(const void* a, const void* b) {
    const RenderServerRenderTask* task_a = *((const RenderServerRenderTask**) a);
    const RenderServerRenderTask* task_b = *((const RenderServerRenderTask**) b);

    i32 arg1 = task_a->priority;
    i32 arg2 = task_b->priority;

    usize major1 = USIZE_MAX;
    usize major2 = USIZE_MAX;

    if (task_a->viewport != NULL && task_a->viewport->type == ViewportTypeSurface)
        major1 = (usize) task_a->viewport->data.surface.surface;
    if (task_b->viewport != NULL && task_b->viewport->type == ViewportTypeSurface)
        major2 = (usize) task_b->viewport->data.surface.surface;

    if (major1 < major2)
        return -1;
    if (major1 > major2)
        return 1;

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

    if (g_renderTaskPtrsNeedToSort) {
        qsort(data, vec_size, sizeof(struct RenderServerRenderTask*), task_comp);
        g_renderTaskPtrsNeedToSort = false;
    }

    if (g_renderTaskUniqueSurfacesNeedToRebuild) {
        vec_ptr_clear(&g_renderTaskUniqueSurfaces);
        // O(n^2), but there is fine, because it's very rare operation (in most cases, when creating a
        // task) Also, I guess it will be fastest solution on small amounts (which they are)
        for (usize i = 0; i < vec_size; ++i) {
            struct RenderServerRenderTask* task = data[i];

            if (task->viewport == NULL || task->viewport->type != ViewportTypeSurface)
                continue;

            usize j = 0;
            for (; j < i; ++j) {
                struct RenderServerRenderTask* task_exist = data[j];
                if (task_exist->viewport == NULL || task_exist->viewport->type != ViewportTypeSurface)
                    continue;
                if (task->viewport->data.surface.surface == task_exist->viewport->data.surface.surface)
                    break;
            }

            if (i == j)
                vec_ptr_push_back(&g_renderTaskUniqueSurfaces, task->viewport->data.surface.surface);
        }
        g_renderTaskUniqueSurfacesNeedToRebuild = false;
    }

    for (usize i = 0; i < vec_size; ++i) {
        struct RenderServerRenderTask* task = data[i];
        if (task->state == RENDER_SERVER_RENDER_TASK_STATE_ENABLED && task->viewport && task->world &&
            task->camera) {
            if (task->viewport->type == ViewportTypeSurface) {
                RenderContext.surface_make_current(task->viewport->data.surface.surface);
            }

            RenderServerViewport* viewport = task->viewport;
            RenderServerCamera* camera = task->camera;

            glViewport(viewport->pos.x, viewport->pos.y, viewport->size.x, viewport->size.y);


            glEnable(GL_DEPTH_TEST);
            glEnable(GL_SCISSOR_TEST);

            glScissor(viewport->pos.x, viewport->pos.y, viewport->size.x, viewport->size.y);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);

            // render
            glClearColor(1.0f, 0.0f, 0.0, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // float proj[16] = {0.9747, 0, 0, 0, 0, 1.7321, 0, 0, 0, 0, -1.0002, -1, 0, 0, -0.2000, 0};

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf((GLfloat*) &camera->projection);
            //            glLoadMatrixf(proj);


            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf((GLfloat*) &camera->view);
            const usize instances_size = task->world->instances.size;
            usize* const instances = task->world->instances.data;
            for (usize i = 0; i < instances_size; ++i) {
                struct Instance* const instance = CMA_PTR(&g_instances, instances[i]);

                struct Mesh* const mesh = CMA_PTR(&g_meshes, instance->mesh);
                struct Buffer* const vbo = CMA_PTR(&g_buffers, mesh->vertices_buf);
                struct Buffer* const ebo = CMA_PTR(&g_buffers, mesh->indices_buf);

                glPushMatrix();

                glMultMatrixf((GLfloat*) &instance->transform.m);
                // glMultMatrixf(t);

                glColorPointer(3, GL_FLOAT, 0, vbo->ptr);
                glVertexPointer(3, GL_FLOAT, 0, vbo->ptr);
                glDrawElements(GL_TRIANGLES, ebo->size / 4, GL_UNSIGNED_INT, ebo->ptr);

                glPopMatrix();
            }

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_SCISSOR_TEST);

            if (task->viewport->type == ViewportTypeSurface) {
                // RenderContext.surface_present(task->viewport->data.surface.surface);
            }
        }
    }

    const usize surfaces_size = g_renderTaskUniqueSurfaces.size;
    struct RenderContextSurface** const s_data =
            (struct RenderContextSurface**) g_renderTaskUniqueSurfaces.data;
    for (usize i = 0; i < surfaces_size; ++i) {
        RenderContext.surface_present(s_data[i]);
    }

    return true;
}


static RenderServerRenderTask* render_task_create(void) {
    RenderServerRenderTask* task = tmalloc(sizeof(RenderServerRenderTask));
    ERROR_ALLOC_CHECK(task, { return NULL; });
    task->viewport = NULL;
    task->world = NULL;
    task->camera = NULL;
    task->state = RENDER_SERVER_RENDER_TASK_STATE_ENABLED;
    task->priority = 0;

    unsigned char status = vec_ptr_push_back(&g_renderTaskPtrs, task);
    if (!status) {
        tfree(task);
        set_error(ERROR_ALLOCATION_FAILED);
        LOG_ERROR_OR_DEBUG_FATAL("RenderServer(OpenGL 1.3)::render_task_create");
        return NULL;
    }
    g_renderTaskPtrsNeedToSort = true;
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
    g_renderTaskUniqueSurfacesNeedToRebuild = true;
    return true;
}

static boolean render_task_set_camera(RenderServerRenderTask* task, RenderServerCamera* camera) {
    ERROR_ARGS_CHECK_2(task, camera, { return false; });
    task->camera = camera;
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
    g_renderTaskPtrsNeedToSort = true;
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
            g_renderTaskPtrsNeedToSort = true;
            g_renderTaskUniqueSurfacesNeedToRebuild = true;
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

static boolean viewport_set_position(RenderServerViewport* viewport, const IVec2* const pos) {
    ERROR_ARGS_CHECK_2(viewport, pos, { return false; });
    viewport->pos = *pos;
    return true;
}

static boolean viewport_set_size(RenderServerViewport* viewport, const IVec2* const size) {
    ERROR_ARGS_CHECK_2(viewport, size, { return false; });
    viewport->size = *size;
    return true;
}

static boolean viewport_destroy(RenderServerViewport* viewport) {
    ERROR_ARGS_CHECK_1(viewport, { return false; });
    tfree(viewport);
    return false;
}


static RenderServerCamera* camera_create(void) {
    RenderServerCamera* camera = tmalloc(sizeof(RenderServerCamera));
    ERROR_ALLOC_CHECK(camera, { return false; });
    camera->view = MAT4_ONE_M;
    camera->projection = MAT4_ONE_M;
    return camera;
}

static boolean camera_projection_set(RenderServerCamera* camera, const Mat4* const m) {
    ERROR_ARGS_CHECK_2(camera, m, { return false; });
    camera->projection = *m;
    return true;
}

static boolean camera_view_set(RenderServerCamera* camera, const Mat4* const m) {
    ERROR_ARGS_CHECK_2(camera, m, { return false; });

    Mat4 view = MAT4_ONE_M;
    // Transpose rotation matrix, [0][0]..[2][2]
    // This is calculate an inverse matrix, because the world is moving, not a camera
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            view.m[i][j] = m->m[j][i];
        }
    }

    // Translate Rotation matrix with inversed coords
    mat4_translate_in(&view, -m->m[3][0], -m->m[3][1], -m->m[3][2]);

    camera->view = view;
    return true;
}

static boolean camera_destroy(RenderServerCamera* camera) {
    ERROR_ARGS_CHECK_1(camera, { return false; });
    tfree(camera);
    return true;
}

static RenderServerWorld* world_create(void) {
    RenderServerWorld* world = tmalloc(sizeof(RenderServerWorld));
    ERROR_ALLOC_CHECK(world, { return false; });
    world->instances = vec_instanceHandle_init();
    return world;
}

static boolean world_add_instance(RenderServerWorld* world, RenderServerInstanceHandle instance) {
    ERROR_ARGS_CHECK_2(world, instance, { return false; });
    vec_instanceHandle_push_back(&world->instances, instance);
    return true;
}

static boolean world_del_instance(RenderServerWorld* world, RenderServerInstanceHandle instance) {
    return false;
}

static boolean world_set_ambient_color(RenderServerWorld* world, const Vec4* const color) {
    return false;
}

static boolean world_destroy(RenderServerWorld* world) {
    ERROR_ARGS_CHECK_1(world, { return false; });
    vec_instanceHandle_free(&world->instances);
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
                "RenderServer(OpenGL 1.3)::" fn_name ": Instance with this handle (%u) not found",      \
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

static boolean instance_set_transform(RenderServerInstanceHandle instance, const Mat4* const transform) {
    ERROR_ARGS_CHECK_1(instance, { return false; });

    INSTANCE_GET_PTR("instance_set_mesh")
    ptr->transform = *transform;

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

    ptr->indices_buf = buffer;

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
        ptr->size = 0;
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
            new_data_ptr = tmalloc(data_size);
        else
            new_data_ptr = trealloc(ptr->ptr, data_size);
        ERROR_ALLOC_CHECK(new_data_ptr, { return false; });
        memcpy(new_data_ptr, data, data_size);

        ptr->ptr = new_data_ptr;
    } else if (data_own_mode == RENDER_SERVER_DATA_OWN_MODE_BORROW) {
        // Yes, cast (const void*) to (void*). But this data is borowed
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
    ptr->size = data_size;
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
    REGISTER(render_task_set_camera);
    REGISTER(render_task_set_state);
    REGISTER(render_task_set_priority);
    REGISTER(render_task_destroy);

    REGISTER(viewport_surface_create);
    REGISTER(viewport_set_position);
    REGISTER(viewport_set_size);
    REGISTER(viewport_destroy);

    REGISTER(camera_create);
    REGISTER(camera_projection_set);
    REGISTER(camera_view_set);
    REGISTER(camera_destroy);

    REGISTER(world_create);
    REGISTER(world_add_instance);
    REGISTER(world_del_instance);
    REGISTER(world_set_ambient_color);
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
