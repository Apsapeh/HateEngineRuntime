#include "render_server_opengl_13.h"

#include <core/math/ivec2.h>

#include <core/platform/memory.h>
#include <core/servers/render_server/render_server.h>
#include <core/error.h>
#include <core/log.h>
#include <core/types/signal.h>
#include <core/types/types.h>
#include <core/types/vector.h>
#include <core/servers/render_context/render_context.h>
#include <string.h>

#include <core/servers/render_server/methods-signatures.h.gen>
#include "error.h"


#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include <stb/stb_image_resize2.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad_ogl13/gl.h>

/* ====> Errors <==== */
#define ANY_ERROR "OpenGLAnyError"
/* ================== */

#define INSTANCES_CHUNK_SIZE 128
#define INSTANCES_CHUNKS_COUNT 4

#define MESHES_CHUNK_SIZE 128
#define MESHES_CHUNKS_COUNT 4

#define BUFFERS_CHUNK_SIZE 512
#define BUFFERS_CHUNKS_COUNT 4

#define MATERIALS_CHUNK_SIZE 128
#define MATERIALS_CHUNKS_COUNT 4

#define TEXTURES_CHUNK_SIZE 128
#define TEXTURES_CHUNKS_COUNT 4


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
    usize normals_buf; // RenderServerBufferHandle
    usize uv1_buf; // RenderServerBufferHandle
    usize uv2_buf; // RenderServerBufferHandle
};

struct Buffer {
    RenderServerBufferType buffer_type; // Vertex, Index, Normal, etc.
    RenderServerDataType data_type; // Float, Int, etc.
    RenderServerDataOwnMode own_mode; // Copy, Borrow, Ptr
    void* ptr;
    usize size;
};

struct Material {
    usize albedo_texture; // RenderServerTextureHandle
};

struct Texture {
    boolean mipmap_is_enabled;
    RenderServerDataType data_type;
    RenderServerTextureFormat format;
    RenderServerTextureFilter filter_min;
    RenderServerTextureFilter filter_mag;
    RenderServerTextureMipmapFilter mipmap_filter_min;
    RenderServerTextureWrapMode wrap_s;
    RenderServerTextureWrapMode wrap_t;
    GLuint gl_tex_hdl;
    const u8* data_ptr;
    IVec2 dimensions;
};

/* =========================================================================== */


/* ============================= Global Varibales ============================= */
static boolean g_gladLoaded = false;

/***** Instances *****/
static ChunkMemoryAllocator g_instances;

/***** Mesh *****/
static ChunkMemoryAllocator g_meshes;

/***** Buffer *****/
static ChunkMemoryAllocator g_buffers;

/***** Material *****/
static ChunkMemoryAllocator g_materials;

/***** Texture *****/
static ChunkMemoryAllocator g_textures;


/* ============================================================================ */
// clang-format off
static const GLenum DATA_TYPE_MAP[RENDER_SERVER_DATA_TYPE_COUNT] = {
        [RENDER_SERVER_DATA_TYPE_F32] = GL_FLOAT,
        [RENDER_SERVER_DATA_TYPE_I8 ] = GL_BYTE,
        [RENDER_SERVER_DATA_TYPE_I16] = GL_SHORT,
        [RENDER_SERVER_DATA_TYPE_I32] = GL_INT,
        [RENDER_SERVER_DATA_TYPE_U8 ] = GL_UNSIGNED_BYTE,
        [RENDER_SERVER_DATA_TYPE_U16] = GL_UNSIGNED_SHORT,
        [RENDER_SERVER_DATA_TYPE_U32] = GL_UNSIGNED_INT,
};
// clang-format on

struct TexFormatMapValue {
    u8 size;
    u8 stbir_format; // stbir_pixel_layout
    GLenum gl_value;
};
#define TFMV_M(_size, _gl_val, _stbir_format)                                                           \
    (struct TexFormatMapValue) {                                                                        \
        .gl_value = _gl_val, .size = _size, .stbir_format = _stbir_format                               \
    }

static const struct TexFormatMapValue TEX_FORMAT_MAP[RENDER_SERVER_TEXTURE_FORMAT_COUNT] = {
        [RENDER_SERVER_TEXTURE_FORMAT_RGB] = TFMV_M(3, GL_RGB, STBIR_RGB),
        [RENDER_SERVER_TEXTURE_FORMAT_RGBA] = TFMV_M(4, GL_RGBA, STBIR_RGBA),
        [RENDER_SERVER_TEXTURE_FORMAT_BGR] = TFMV_M(3, GL_BGR, STBIR_BGR),
        [RENDER_SERVER_TEXTURE_FORMAT_BGRA] = TFMV_M(4, GL_BGRA, STBIR_BGRA),
};

#undef TFMV_M

static const GLenum TEX_FILTER_MAP[RENDER_SERVER_TEXTURE_FILTER_COUNT] = {
        [RENDER_SERVER_TEXTURE_FILTER_NEAREST] = GL_NEAREST,
        [RENDER_SERVER_TEXTURE_FILTER_LINEAR] = GL_LINEAR,
};

// clang-format off
static const GLenum TEX_MIPMAP_FILTER_MAP[RENDER_SERVER_TEXTURE_FILTER_COUNT][RENDER_SERVER_TEXTURE_MIPMAP_FILTER_COUNT] = {
    [RENDER_SERVER_TEXTURE_FILTER_NEAREST] = {
        [RENDER_SERVER_TEXTURE_MIPMAP_FILTER_NEAREST] = GL_NEAREST_MIPMAP_NEAREST,
        [RENDER_SERVER_TEXTURE_MIPMAP_FILTER_LINEAR] = GL_NEAREST_MIPMAP_LINEAR,
    },
    [RENDER_SERVER_TEXTURE_FILTER_LINEAR] = {
        [RENDER_SERVER_TEXTURE_MIPMAP_FILTER_NEAREST] = GL_LINEAR_MIPMAP_NEAREST,
        [RENDER_SERVER_TEXTURE_MIPMAP_FILTER_LINEAR] = GL_LINEAR_MIPMAP_LINEAR,
    }
};
// clang-format on

static const GLenum TEX_WRAP_MODE_MAP[RENDER_SERVER_TEXTURE_WRAP_MODE_COUNT] = {
        [RENDER_SERVER_TEXTURE_WRAP_MODE_REPEAT] = GL_REPEAT,
        [RENDER_SERVER_TEXTURE_WRAP_MODE_MIRRORED_REPEAT] = GL_REPEAT,
        [RENDER_SERVER_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE] = GL_CLAMP_TO_EDGE,
};


#define BACKEND_NAME "RenderServer(OpenGL 1.3)"
#define METHOD_NAME ""
#undef METHOD_NAME

#define LOG_HEADER BACKEND_NAME "::" METHOD_NAME ":"

#define CMA_CHECK_LOAD(type, cma, return_block)                                                         \
    struct type* ptr = chunk_memory_allocator_get_real_ptr(&cma, hdl);                                  \
    if (!ptr) {                                                                                         \
        LOG_ERROR_OR_DEBUG_FATAL(LOG_HEADER #type " (%u) not found");                                   \
        set_error(ERROR_NOT_FOUND);                                                                     \
        return_block                                                                                    \
    }


static void init_glad_cb(void* args, void* ctx) {
    if (!gladLoadGL(RenderContext.get_proc_addr)) {
        LOG_FATAL("OpenGL load error (glad)")
    }
    g_gladLoaded = true;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

#define INIT_SUBSYSTEM_VARS(var_name, type, chunk_size, chunks_count)                                   \
    chunk_memory_allocator_constructor(&var_name, sizeof(type), chunk_size, chunks_count);

static boolean _init(void) {
    // First we should create a window. With window will be created an OpenGL context.
    // We don't know when the context will be created (bc it's user's space), this why we use signal
    // system.
    SignalCallbackHandler h = RenderContext.signal_connect("gl_context_created", init_glad_cb, NULL);
    if (h == 0) {
        LOG_ERROR_OR_DEBUG_FATAL("RenderServer::_init: Signal can't be connected");
        return false;
    }

    g_renderTaskPtrs = vec_ptr_init();

    INIT_SUBSYSTEM_VARS(g_instances, struct Instance, INSTANCES_CHUNK_SIZE, INSTANCES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_meshes, struct Mesh, MESHES_CHUNK_SIZE, MESHES_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_buffers, struct Buffer, BUFFERS_CHUNK_SIZE, BUFFERS_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_materials, struct Material, MATERIALS_CHUNK_SIZE, MATERIALS_CHUNKS_COUNT);
    INIT_SUBSYSTEM_VARS(g_textures, struct Texture, TEXTURES_CHUNK_SIZE, TEXTURES_CHUNKS_COUNT);

    return true;
}

#define QUIT_SUBSYSTEM_VARS(var_name) chunk_memory_allocator_destructor(&var_name);

static boolean _quit(void) {
    vec_ptr_free(&g_renderTaskPtrs);

    QUIT_SUBSYSTEM_VARS(g_instances);
    QUIT_SUBSYSTEM_VARS(g_meshes);
    QUIT_SUBSYSTEM_VARS(g_buffers);
    QUIT_SUBSYSTEM_VARS(g_materials);
    QUIT_SUBSYSTEM_VARS(g_textures);

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

#ifdef HE_DEBUG
    #define __CMA_PTR(cma, hdl) chunk_memory_allocator_get_real_ptr(cma, hdl);
#else
    #define __CMA_PTR(cma, hdl) chunk_memory_allocator_get_real_ptr_unsafe(cma, hdl);
#endif


static inline void* cma_ptr(ChunkMemoryAllocator* cma, chunk_allocator_ptr hdl) {
    return hdl == 0 ? NULL : __CMA_PTR(cma, hdl);
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


    RenderContextSurface* current_surface = NULL;
    for (usize i = 0; i < vec_size; ++i) {
        struct RenderServerRenderTask* task = data[i];
        if (task->state == RENDER_SERVER_RENDER_TASK_STATE_ENABLED && task->viewport && task->world &&
            task->camera) {
            if (task->viewport->data.surface.surface != current_surface &&
                task->viewport->type == ViewportTypeSurface) {
                if (current_surface != NULL) {
                    RenderContext.surface_present(current_surface);
                }

                current_surface = task->viewport->data.surface.surface;
                RenderContext.surface_make_current(task->viewport->data.surface.surface);
            }

            RenderServerViewport* viewport = task->viewport;
            RenderServerCamera* camera = task->camera;

            glViewport(viewport->pos.x, viewport->pos.y, viewport->size.x, viewport->size.y);


            glEnable(GL_DEPTH_TEST);
            glEnable(GL_SCISSOR_TEST);

            glScissor(viewport->pos.x, viewport->pos.y, viewport->size.x, viewport->size.y);

            glEnableClientState(GL_VERTEX_ARRAY);
            //            glEnableClientState(GL_COLOR_ARRAY);

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
                struct Instance* const instance = cma_ptr(&g_instances, instances[i]);


                struct Material* const material = cma_ptr(&g_materials, instance->material);
                struct Mesh* const mesh = cma_ptr(&g_meshes, instance->mesh);
                struct Buffer* const vbo = cma_ptr(&g_buffers, mesh->vertices_buf);
                struct Buffer* const ebo = cma_ptr(&g_buffers, mesh->indices_buf);
                struct Buffer* const ubo = cma_ptr(&g_buffers, mesh->uv1_buf);


                glPushMatrix();

                glMultMatrixf((GLfloat*) &instance->transform.m);

                if (ubo && material && material->albedo_texture) {
                    struct Texture* const texture = cma_ptr(&g_textures, material->albedo_texture);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture->gl_tex_hdl);

                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glClientActiveTexture(GL_TEXTURE0);
                    glTexCoordPointer(2, GL_FLOAT, 0, ubo->ptr);
                }


                // glMultMatrixf(t);

                //                glColorPointer(3, GL_FLOAT, 0, vbo->ptr);
                glVertexPointer(3, GL_FLOAT, 0, vbo->ptr);
                glDrawElements(GL_TRIANGLES, ebo->size / 4, GL_UNSIGNED_INT, ebo->ptr);

                if (ubo && material && material->albedo_texture) {
                    glActiveTexture(GL_TEXTURE0);
                    glClientActiveTexture(GL_TEXTURE0);
                    glEnable(GL_TEXTURE_2D);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                }


                glPopMatrix();
            }

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_SCISSOR_TEST);
        }
    }

    if (current_surface != NULL) {
        RenderContext.surface_present(current_surface);
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
    ERROR_ALLOC_CHECK(h, { return 0; });

    ptr->material = 0;
    ptr->mesh = 0;
    ptr->transform = MAT4_ONE_M;
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
    ERROR_ALLOC_CHECK(h, { return 0; });

    ptr->indices_buf = 0;
    ptr->vertices_buf = 0;
    ptr->normals_buf = 0;
    ptr->uv1_buf = 0;
    ptr->uv2_buf = 0;

    return h;
}

static boolean mesh_set_buffer(
        RenderServerMeshHandle hdl, RenderServerBufferType target, RenderServerBufferHandle buffer
) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    ERROR_RANGE_CHECK(target, 0, RENDER_SERVER_BUFFER_TYPE_COUNT, { return false; });

    struct Mesh* ptr = chunk_memory_allocator_get_real_ptr(&g_meshes, hdl);
    if (!ptr)
        return false;

    switch (target) {
        case RENDER_SERVER_BUFFER_TYPE_VERTEX:
            ptr->vertices_buf = buffer;
            break;
        case RENDER_SERVER_BUFFER_TYPE_INDEX:
            ptr->indices_buf = buffer;
            break;
        case RENDER_SERVER_BUFFER_TYPE_NORMAL:
            ptr->normals_buf = buffer;
            break;
        case RENDER_SERVER_BUFFER_TYPE_UV1:
            ptr->uv1_buf = buffer;
            break;
        case RENDER_SERVER_BUFFER_TYPE_UV2:
            ptr->uv2_buf = buffer;
            break;
    }

    return true;
}

static boolean mesh_destroy(RenderServerMeshHandle hdl) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    return chunk_memory_allocator_free_mem(&g_meshes, hdl);
}


#define METHOD_NAME "buffer_create"
static RenderServerBufferHandle buffer_create(
        RenderServerBufferType type, RenderServerBufferUsageHint usage_hint
) {
    ERROR_RANGE_CHECK(usage_hint, 0, RENDER_SERVER_BUFFER_USAGE_HINT_COUNT, { return 0; });
    ERROR_RANGE_CHECK(type, 0, RENDER_SERVER_BUFFER_TYPE_COUNT, { return 0; });

    struct Buffer* ptr = NULL;
    RenderServerBufferHandle h = chunk_memory_allocator_alloc_mem(&g_buffers, (void**) &ptr);
    ERROR_ALLOC_CHECK(h, { return 0; });

    ptr->ptr = NULL;
    ptr->size = 0;
    ptr->own_mode = RENDER_SERVER_DATA_OWN_MODE_PTR;
    ptr->buffer_type = type;

    return h;
}
#undef METHOD_NAME

#define METHOD_NAME "buffer_set_data"
static boolean buffer_set_data(
        RenderServerBufferHandle hdl, const void* data, u64 data_size, RenderServerDataType data_type,
        RenderServerDataOwnMode data_own_mode
) {
    ERROR_ARGS_CHECK_3(hdl, data, data_size, { return false; });
    ERROR_RANGE_CHECK(data_type, 0, RENDER_SERVER_DATA_TYPE_COUNT, { return false; })
    ERROR_RANGE_CHECK(data_own_mode, 0, RENDER_SERVER_DATA_OWN_MODE_COUNT, { return false; })

    // In OpenGL 1.x you don't have any GPU buffers, all data stored in the RAM.
    // You must send all your data to the GPU immediatly at an each frame

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
#undef METHOD_NAME

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


static RenderServerMaterialHandle material_create(void) {
    struct Material* ptr = NULL;
    RenderServerMaterialHandle h = chunk_memory_allocator_alloc_mem(&g_materials, (void**) &ptr);
    ERROR_ALLOC_CHECK(h, { return 0; });

    ptr->albedo_texture = 0;
    return h;
}

#define METHOD_NAME "material_set_albedo_texture"
static boolean material_set_albedo_texture(
        RenderServerMaterialHandle hdl, RenderServerTextureHandle tex
) {
    // We are checks only hdl, so tex can be nulled
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    CMA_CHECK_LOAD(Material, g_materials, { return false; });

    ptr->albedo_texture = tex;

    return true;
}
#undef METHOD_NAME

static boolean material_destroy(RenderServerMaterialHandle ptr) {
    ERROR_ARGS_CHECK_1(ptr, { return false; });
    return chunk_memory_allocator_free_mem(&g_materials, ptr);
}

#define METHOD_NAME "texture_create"
static RenderServerTextureHandle texture_create(void) {
    struct Texture* ptr = NULL;
    RenderServerMaterialHandle h = chunk_memory_allocator_alloc_mem(&g_textures, (void**) &ptr);
    ERROR_ALLOC_CHECK(h, { return 0; });

    glGenTextures(1, &ptr->gl_tex_hdl);

    if (ptr->gl_tex_hdl == 0) {
        u32 code = glGetError();
        LOG_ERROR_OR_DEBUG_FATAL(LOG_HEADER " glGenTextures is failed with OpenGL error: %u", code);
        set_error(ANY_ERROR);
        return 0;
    }

    ptr->data_ptr = NULL;
    ptr->filter_min = RENDER_SERVER_TEXTURE_FILTER_LINEAR;
    ptr->filter_mag = RENDER_SERVER_TEXTURE_FILTER_LINEAR;
    ptr->mipmap_is_enabled = false;
    ptr->mipmap_filter_min = RENDER_SERVER_TEXTURE_MIPMAP_FILTER_LINEAR;
    ptr->wrap_s = RENDER_SERVER_TEXTURE_WRAP_MODE_REPEAT;
    ptr->wrap_t = RENDER_SERVER_TEXTURE_WRAP_MODE_REPEAT;

    return h;
}
#undef METHOD_NAME


#define METHOD_NAME "texture_set_data"
static boolean texture_set_data(
        RenderServerTextureHandle hdl, RenderServerTextureFormat format, const IVec2* const dimensions,
        RenderServerDataType data_type, const u8* const data
) {
    ERROR_ARGS_CHECK_3(hdl, data, dimensions, { return false; });
    ERROR_ARGS_CHECK_2(dimensions->x, dimensions->y, { return false; });
    //    ERROR_RANGE_CHECK(format, 0, RENDER_SERVER_TEXTURE_FORMAT_COUNT, { return false; });
    if (!(data_type == RENDER_SERVER_DATA_TYPE_U8 || data_type == RENDER_SERVER_DATA_TYPE_F32)) {
        LOG_ERROR_OR_DEBUG_FATAL(
                LOG_HEADER "Texture format must be only u8 of f32 (RENDER_SERVER_DATA_TYPE_U8 or "
                           "RENDER_SERVER_DATA_TYPE_F32)"
        );
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    if (dimensions->x % 2 != 0 || dimensions->y % 2 != 0) {
        LOG_ERROR_OR_DEBUG_FATAL(LOG_HEADER "Texture must be power of two (16x16, 256x256, etc.)")
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    CMA_CHECK_LOAD(Texture, g_textures, { return false; });

    ptr->data_ptr = data;
    ptr->data_type = data_type;
    ptr->dimensions = *dimensions;
    ptr->format = format;

    return true;
}

#undef METHOD_NAME


#define METHOD_NAME "texture_set_filter"
static boolean texture_set_filter(
        RenderServerTextureHandle hdl, RenderServerTextureFilter min, RenderServerTextureFilter mag
) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    ERROR_RANGE_CHECK(min, 0, RENDER_SERVER_TEXTURE_FILTER_COUNT, { return false; });
    ERROR_RANGE_CHECK(mag, 0, RENDER_SERVER_TEXTURE_FILTER_COUNT, { return false; });

    CMA_CHECK_LOAD(Texture, g_textures, { return false; });

    ptr->filter_min = min;
    ptr->filter_mag = mag;

    return true;
}
#undef METHOD_NAME

#define METHOD_NAME "texture_set_mipmap"
static boolean texture_set_mipmap(
        RenderServerTextureHandle hdl, boolean enable, RenderServerTextureMipmapFilter min
) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    ERROR_RANGE_CHECK(min, 0, RENDER_SERVER_TEXTURE_MIPMAP_FILTER_COUNT, { return false; });

    CMA_CHECK_LOAD(Texture, g_textures, { return false; });

    ptr->mipmap_is_enabled = enable;
    ptr->mipmap_filter_min = min;
    return true;
}
#undef METHOD_NAME

#define METHOD_NAME "texture_set_wrap"
static boolean texture_set_wrap(
        RenderServerTextureHandle hdl, RenderServerTextureWrapMode s, RenderServerTextureWrapMode t
) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    ERROR_RANGE_CHECK(s, 0, RENDER_SERVER_TEXTURE_WRAP_MODE_COUNT, { return false; });
    ERROR_RANGE_CHECK(t, 0, RENDER_SERVER_TEXTURE_WRAP_MODE_COUNT, { return false; });

    if (s == RENDER_SERVER_TEXTURE_WRAP_MODE_MIRRORED_REPEAT ||
        t == RENDER_SERVER_TEXTURE_WRAP_MODE_MIRRORED_REPEAT) {
        LOG_WARN(
                LOG_HEADER "'RENDER_SERVER_TEXTURE_WRAP_MODE_MIRRORED_REPEAT' doesn't supported in "
                           "OpenGL 1.3. Required OpenGL >= 1.4. Will be used "
                           "'RENDER_SERVER_TEXTURE_WRAP_MODE_REPEAT'"
        )
    }

    CMA_CHECK_LOAD(Texture, g_textures, { return false; });

    ptr->wrap_s = s;
    ptr->wrap_t = t;

    return true;
}
#undef METHOD_NAME


#define METHOD_NAME "texture_update"
static boolean texture_update(RenderServerTextureHandle hdl) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });

    CMA_CHECK_LOAD(Texture, g_textures, { return false; });

    if (ptr->data_ptr == NULL) {
        LOG_ERROR_OR_DEBUG_FATAL(
                LOG_HEADER "Texture pointer is not setted. You must first call \"texture_set_data\"!"
        )
        set_error(ERROR_INVALID_STATE);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, ptr->gl_tex_hdl);

    // Filters
    GLenum min_filter;
    GLenum mag_filter;

    if (ptr->mipmap_is_enabled)
        min_filter = TEX_MIPMAP_FILTER_MAP[ptr->filter_min][ptr->mipmap_filter_min];
    else
        min_filter = TEX_FILTER_MAP[ptr->filter_min];

    mag_filter = TEX_FILTER_MAP[ptr->filter_mag];

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    // Wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TEX_WRAP_MODE_MAP[ptr->wrap_s]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TEX_WRAP_MODE_MAP[ptr->wrap_t]);

    // Format
    GLenum format = TEX_FORMAT_MAP[ptr->format].gl_value;
    GLenum data_type = DATA_TYPE_MAP[ptr->data_type];
    glTexImage2D(
            GL_TEXTURE_2D, 0, format, ptr->dimensions.x, ptr->dimensions.y, 0, format, data_type,
            ptr->data_ptr
    );

    // STB Image Resize MipMap generation
    u8 format_size = TEX_FORMAT_MAP[ptr->format].size;
    stbir_pixel_layout stbir_format = TEX_FORMAT_MAP[ptr->format].stbir_format;
    if (ptr->mipmap_is_enabled && false) {
        usize data_type_size =
                data_type == RENDER_SERVER_DATA_TYPE_F32 ? sizeof(f32) : sizeof(u8); // f32 or u8

        const usize data_size = data_type_size * format_size * ptr->dimensions.x * ptr->dimensions.y;

        // Temp texture X/2 x Y/2 for the largest level of MipMap
        void* tmp_buffer_1 = tmalloc(data_size / 4);
        ERROR_ALLOC_CHECK(tmp_buffer_1, { return false; });

        // Temp texture X/4 x Y/4 for the next level of the largest level of MipMap
        void* tmp_buffer_2 = tmalloc(data_size / 8);
        ERROR_ALLOC_CHECK(tmp_buffer_2, { return false; });

        const u8* input_ptr = ptr->data_ptr;
        u8* output_ptr = tmp_buffer_1;

        i32 input_w = ptr->dimensions.x;
        i32 input_h = ptr->dimensions.y;

        u32 level = 1;
        while (input_w > 1 && input_h > 1) {
            i32 output_w = input_w / 2;
            i32 output_h = input_h / 2;

            if (data_type == RENDER_SERVER_DATA_TYPE_F32) {
                stbir_resize_float_linear(
                        (const float*) input_ptr, input_w, input_h, 0, (float*) output_ptr, output_w,
                        output_h, 0, stbir_format
                );
            } else {
                stbir_resize_uint8_srgb(
                        input_ptr, input_w, input_h, 0, output_ptr, output_w, output_h, 0, stbir_format
                );
            }

            glTexImage2D(
                    GL_TEXTURE_2D, level, format, output_w, output_h, 0, format, data_type, output_ptr
            );

            // Swap buffers
            if (level % 2 == 0) { // 2, 4, 6, ... iters
                input_ptr = tmp_buffer_1;
                output_ptr = tmp_buffer_2;
            } else { // 1, 3, 5, ... iters
                input_ptr = tmp_buffer_2;
                output_ptr = tmp_buffer_1;
            }

            ++level;
            input_w = output_w;
            input_h = output_h;
        }

        tfree(tmp_buffer_1);
        tfree(tmp_buffer_2);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}
#undef METHOD_NAME


#define METHOD_NAME "texture_destroy"
static boolean texture_destroy(RenderServerTextureHandle hdl) {
    ERROR_ARGS_CHECK_1(hdl, { return false; });
    CMA_CHECK_LOAD(Texture, g_textures, { return false; });
    glDeleteTextures(1, &ptr->gl_tex_hdl);
    return chunk_memory_allocator_free_mem(&g_textures, hdl);
}
#undef METHOD_NAME

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
    REGISTER(mesh_set_buffer);
    REGISTER(mesh_destroy);

    REGISTER(buffer_create);
    REGISTER(buffer_set_data);
    REGISTER(buffer_destroy);

    REGISTER(material_create);
    REGISTER(material_set_albedo_texture);
    REGISTER(material_destroy);

    REGISTER(texture_create);
    REGISTER(texture_set_data);
    REGISTER(texture_set_filter);
    REGISTER(texture_set_mipmap);
    REGISTER(texture_set_wrap);
    REGISTER(texture_update);
    REGISTER(texture_destroy);

    render_server_register_backend("OpenGL 1.3", backend);
}
