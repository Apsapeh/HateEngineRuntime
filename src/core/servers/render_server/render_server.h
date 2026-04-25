#pragma once

#include <core/error.h>
#include <core/types/types.h>
#include <core/types/signal.h>
#include <core/types/string.h>
#include <core/math/ivec2.h>
#include <core/math/mat3.h>
#include <core/math/mat4.h>
#include <core/math/vec3.h>
#include <core/math/vec4.h>
#include <core/ex_alloc/chunk_allocator.h>
#include <core/servers/render_context/render_context.h>


// MACROS API START

// clang-format off
#define RENDER_SERVER_RENDER_TASK_STATE_ENABLED   0
#define RENDER_SERVER_RENDER_TASK_STATE_DISABLED  1
#define RENDER_SERVER_RENDER_TASK_STATE_FIRST RENDER_SERVER_RENDER_TASK_STATE_ENABLED
#define RENDER_SERVER_RENDER_TAST_STATE_LAST  RENDER_SERVER_RENDER_TASK_STATE_DISABLED
// clang-format on

// MACROS API END

/**
 * 0 - Enabled. Task will be rendered
 *
 * 1 - Disabled. Task will be skipped
 *
 * @api
 */
typedef u8 RenderServerRenderTaskState;


/* ============> Buffer Usage Hint Enum <============ */
// clang-format off
/*
u8 RenderServerBufferUsageHint

Static // Buffer will be set once and never changed. Use for static meshes like level, level environment, etc.
Dynamic // Buffer will be sometimes change. Use for maybe minecraft-like chunks, idk
Stream // Buffer will be changed every few frames.Use for particles
Count
 */
// clang-format on

/**
 * @api
 */
enum {
    // Buffer will be set once and never changed. Use for static meshes like level, level environment,
    // etc.
    RENDER_SERVER_BUFFER_USAGE_HINT_STATIC = 0,
    // Buffer will be sometimes change. Use for maybe minecraft-like chunks, idk
    RENDER_SERVER_BUFFER_USAGE_HINT_DYNAMIC = 1,
    // Buffer will be changed every few frames.Use for particles
    RENDER_SERVER_BUFFER_USAGE_HINT_STREAM = 2,
    RENDER_SERVER_BUFFER_USAGE_HINT_COUNT = 3,
};

/**
 * 0 - Static: Buffer will be set once and never changed. Use for static meshes like level, level
 environment, etc.

 * 1 - Dynamic: Buffer will be sometimes change. Use for maybe minecraft-like chunks, idk

 * 2 - Stream: Buffer will be changed every few frames.Use for particles

 * 3 - Count

 * @api
 */
typedef u8 RenderServerBufferUsageHint;


/* ============> Buffer Type Enum <============ */
// clang-format off
/*
u8 RenderServerBufferType

Vertex // Float types
Index // Integer types
Normal // Float types. Must be normalized
UV1 
UV2
Count
 */
// clang-format on

/**
 * @api
 */
enum {
    // Float types
    RENDER_SERVER_BUFFER_TYPE_VERTEX = 0,
    // Integer types
    RENDER_SERVER_BUFFER_TYPE_INDEX = 1,
    // Float types. Must be normalized
    RENDER_SERVER_BUFFER_TYPE_NORMAL = 2,
    RENDER_SERVER_BUFFER_TYPE_UV1 = 3,
    RENDER_SERVER_BUFFER_TYPE_UV2 = 4,
    RENDER_SERVER_BUFFER_TYPE_COUNT = 5,
};

/**
 * 0 - Vertex: Float types

 * 1 - Index: Integer types

 * 2 - Normal: Float types. Must be normalized

 * 3 - UV1

 * 4 - UV2

 * 5 - Count

 * @api
 */
typedef u8 RenderServerBufferType;


/* ============> Texture Format Enum <============ */
/*
u8 RenderServerDataType

F32  // 4 bytes float point
I8   // 1 byte signed integer
I16  // 2 byte signed integer
I32  // 4 byte signed integer
U8   // 1 byte unsigned integer
U16  // 2 byte unsigned integer
U32  // 4 byte unsigned integer
Count
 */


/**
 * @api
 */
enum {
    // 4 bytes float point
    RENDER_SERVER_DATA_TYPE_F32 = 0,
    // 1 byte signed integer
    RENDER_SERVER_DATA_TYPE_I8 = 1,
    // 2 byte signed integer
    RENDER_SERVER_DATA_TYPE_I16 = 2,
    // 4 byte signed integer
    RENDER_SERVER_DATA_TYPE_I32 = 3,
    // 1 byte unsigned integer
    RENDER_SERVER_DATA_TYPE_U8 = 4,
    // 2 byte unsigned integer
    RENDER_SERVER_DATA_TYPE_U16 = 5,
    // 4 byte unsigned integer
    RENDER_SERVER_DATA_TYPE_U32 = 6,
    RENDER_SERVER_DATA_TYPE_COUNT = 7,
};

/**
 * 0 - F32: 4 bytes float point

 * 1 - I8: 1 byte signed integer

 * 2 - I16: 2 byte signed integer

 * 3 - I32: 4 byte signed integer

 * 4 - U8: 1 byte unsigned integer

 * 5 - U16: 2 byte unsigned integer

 * 6 - U32: 4 byte unsigned integer

 * 7 - Count

 * @api
 */
typedef u8 RenderServerDataType;


/* ============> Texture Format Enum <============ */
// clang-format off
/*
u8 RenderServerDataOwnMode

Copy     // Data may be copied, may be not, depending on the backend implementation. RenderServer will own this data
Borrow   // Data will be borrowed. RenderServer will own this data
Ptr      // Data can be stored as a pointer without copying. Render server is not ownes data
Count        
*/
// clang-format on

/**
 * @api
 */
enum {
    // Data may be copied, may be not, depending on the backend implementation. RenderServer will own
    // this data
    RENDER_SERVER_DATA_OWN_MODE_COPY = 0,
    // Data will be borrowed. RenderServer will own this data
    RENDER_SERVER_DATA_OWN_MODE_BORROW = 1,
    // Data can be stored as a pointer without copying. Render server is not ownes data
    RENDER_SERVER_DATA_OWN_MODE_PTR = 2,
    RENDER_SERVER_DATA_OWN_MODE_COUNT = 3,
};

/**
 * 0 - Copy: Data may be copied, may be not, depending on the backend implementation. RenderServer will
 own this data

 * 1 - Borrow: Data will be borrowed. RenderServer will own this data

 * 2 - Ptr: Data can be stored as a pointer without copying. Render server is not ownes data

 * 3 - Count

 * @api
 */
typedef u8 RenderServerDataOwnMode;


/* ============> Texture Format Enum <============ */
/*
u8 RenderServerTextureFormat

RGB
RGBA
BGR
BGRA
Count
 */

/**
 * @api
 */
enum {
    RENDER_SERVER_TEXTURE_FORMAT_RGB = 0,
    RENDER_SERVER_TEXTURE_FORMAT_RGBA = 1,
    RENDER_SERVER_TEXTURE_FORMAT_BGR = 2,
    RENDER_SERVER_TEXTURE_FORMAT_BGRA = 3,
    RENDER_SERVER_TEXTURE_FORMAT_COUNT = 4,
};

/**
 * 0 - RGB

 * 1 - RGBA

 * 2 - BGR

 * 3 - BGRA

 * 4 - Count

 * @api
 */
typedef u8 RenderServerTextureFormat;


/* ============> Texture Filter Enum <============ */
/*
u8 RenderServerTextureFilter

Nearest
Linear
Count
 */

/**
 * @api
 */
enum {
    RENDER_SERVER_TEXTURE_FILTER_NEAREST = 0,
    RENDER_SERVER_TEXTURE_FILTER_LINEAR = 1,
    RENDER_SERVER_TEXTURE_FILTER_COUNT = 2,
};

/**
 * 0 - Nearest

 * 1 - Linear

 * 2 - Count

 * @api
 */
typedef u8 RenderServerTextureFilter;


/* ============> Texture MipMap Filter Enum <============ */
/*
u8 RenderServerTextureMipmapFilter

Nearest
Linear
Count
 */

/**
 * @api
 */
enum {
    RENDER_SERVER_TEXTURE_MIPMAP_FILTER_NEAREST = 0,
    RENDER_SERVER_TEXTURE_MIPMAP_FILTER_LINEAR = 1,
    RENDER_SERVER_TEXTURE_MIPMAP_FILTER_COUNT = 2,
};

/**
 * 0 - Nearest

 * 1 - Linear

 * 2 - Count

 * @api
 */
typedef u8 RenderServerTextureMipmapFilter;


/* ============> Texture Wrap Mode Enum <============ */
/*
u8 RenderServerTextureWrapMode

Repeat
MirroredRepeat
ClampToEdge
Count
 */

/**
 * @api
 */
enum {
    RENDER_SERVER_TEXTURE_WRAP_MODE_REPEAT = 0,
    RENDER_SERVER_TEXTURE_WRAP_MODE_MIRRORED_REPEAT = 1,
    RENDER_SERVER_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE = 2,
    RENDER_SERVER_TEXTURE_WRAP_MODE_COUNT = 3,
};

/**
 * 0 - Repeat

 * 1 - MirroredRepeat

 * 2 - ClampToEdge

 * 3 - Count

 * @api
 */
typedef u8 RenderServerTextureWrapMode;


/**
 * @api
 */
typedef struct RenderServerRenderTask RenderServerRenderTask;

/**
 * @api
 */
typedef struct RenderServerViewport RenderServerViewport;

/**
 * @api
 */
typedef struct RenderServerCamera RenderServerCamera;

/**
 * @api
 */
typedef struct RenderServerWorld RenderServerWorld;

/**
 * @api
 */
typedef struct RenderServerInstance RenderServerInstance;

/**
 * @api
 */
typedef struct RenderServerMesh RenderServerMesh;

/**
 * @api
 */
typedef struct RenderServerBuffer RenderServerBuffer;

/**
 * @api
 */
typedef struct RenderServerMaterial RenderServerMaterial;


/**
 * @brief
 *
 * First you should configure texture params. After - call texture_update()
 *
 * @api
 */
typedef struct RenderServerTexture RenderServerTexture;

/**
 * @api server
 * @api_config {
 *     "fn_prefix": "render_server_",
 *     "init_method": "___hate_engine_runtime_init_render_server"
 * }
 */
typedef struct {
    boolean (*_init)(void);
    boolean (*_quit)(void);

    // TODO:  change to StringSlice when fix/string PR is will be allowed
    // Signal* (*get_signal)(c_str name);
    // SignalCallbackHandler (*signal_connect)(c_str name, SignalCallbackFunc func, void* ctx);
    // boolean (*signal_disconnect)(c_str name, SignalCallbackHandler);

    boolean (*_draw)(double delta);

    RenderServerRenderTask* (*render_task_create)(void);
    boolean (*render_task_set_world)(RenderServerRenderTask* task, RenderServerWorld* world);
    boolean (*render_task_set_viewport)(RenderServerRenderTask* tast, RenderServerViewport* viewport);
    boolean (*render_task_set_camera)(RenderServerRenderTask* task, RenderServerCamera* camera);
    boolean (*render_task_set_state)(RenderServerRenderTask* task, RenderServerRenderTaskState state);
    boolean (*render_task_set_priority)(RenderServerRenderTask* task, i32 priority);
    boolean (*render_task_destroy)(RenderServerRenderTask* task);


    RenderServerViewport* (*viewport_surface_create)(RenderContextSurface* surface);
    // RenderServerViewport* (*viewport_texture_create)
    boolean (*viewport_set_position)(RenderServerViewport* viewport, const IVec2* const pos);
    boolean (*viewport_set_size)(RenderServerViewport* viewport, const IVec2* const size);
    boolean (*viewport_destroy)(RenderServerViewport* viewport);

    RenderServerCamera* (*camera_create)(void);
    boolean (*camera_projection_set)(RenderServerCamera* camera, const Mat4* const matrix);
    boolean (*camera_view_set)(RenderServerCamera* camera, const Mat4* const matrix);
    boolean (*camera_destroy)(RenderServerCamera* camera);

    // Environment
    // RID (*environment_create)(void);
    // boolean (*environment_set_ambient_color)(RID rid, Vec4 color);
    // boolean (*environment_destroy)(RID rid);
    //

    // World (contains all data to draw)
    RenderServerWorld* (*world_create)(void);
    boolean (*world_add_instance)(RenderServerWorld* world, RenderServerInstance* instance);
    boolean (*world_del_instance)(RenderServerWorld* world, RenderServerInstance* instance);
    boolean (*world_set_ambient_color)(RenderServerWorld* world, const Vec4* const color);
    boolean (*world_destroy)(RenderServerWorld* world);

    // Instance (contains mesh, material, transform)
    RenderServerInstance* (*instance_create)(void);
    boolean (*instance_set_mesh)(RenderServerInstance* instance, RenderServerMesh* mesh);
    boolean (*instance_set_material)(RenderServerInstance* instance, RenderServerMaterial* material);
    boolean (*instance_set_transform)(RenderServerInstance* instance, const Mat4* const trasform);
    boolean (*instance_destroy)(RenderServerInstance* instance);

    // Mesh
    RenderServerMesh* (*mesh_create)(void);
    boolean (*mesh_set_buffer)(
            RenderServerMesh* self, RenderServerBufferType target, RenderServerBuffer* buffer
    );
    boolean (*mesh_destroy)(RenderServerMesh* self);

    // Buffer
    RenderServerBuffer* (*buffer_create)(
            RenderServerBufferType type, RenderServerBufferUsageHint usage_hint
    );
    boolean (*buffer_set_data)(
            RenderServerBuffer* hdl, const void* data, u64 data_size_in_bytes,
            RenderServerDataType data_type, RenderServerDataOwnMode data_own_mode
    );
    boolean (*buffer_destroy)(RenderServerBuffer* hdl);

    // Material
    RenderServerMaterial* (*material_create)(void);
    boolean (*material_set_albedo_texture)(RenderServerMaterial* material, RenderServerTexture* texture);
    boolean (*material_destroy)(RenderServerMaterial* hdl);

    // Texture
    RenderServerTexture* (*texture_create)(void);
    /**
     * @param data_type Must be only u8 or f32 (RENDER_SERVER_DATA_TYPE_U8 or
     * RENDER_SERVER_DATA_TYPE_F32)
     * @param data Pointer must be valid until texture_update is called
     */
    boolean (*texture_set_data)(
            RenderServerTexture* hdl, RenderServerTextureFormat format, const IVec2* const dimensions,
            RenderServerDataType data_type, const u8* const data
    );
    boolean (*texture_set_filter)(
            RenderServerTexture* hdl, RenderServerTextureFilter min, RenderServerTextureFilter mag
    );
    boolean (*texture_set_mipmap)(
            RenderServerTexture* hdl, boolean enable, RenderServerTextureMipmapFilter min
    );
    boolean (*texture_set_wrap)(
            RenderServerTexture* hdl, RenderServerTextureWrapMode s, RenderServerTextureWrapMode t
    );
    boolean (*texture_update)(RenderServerTexture* hdl);
    boolean (*texture_destroy)(RenderServerTexture* hdl);

} RenderServerBackend;


// MACROS API START

// clang-format off
#define RENDER_SERVER_THREAD_MODE_SYNC   0
#define RENDER_SERVER_THREDA_MODE_ASYNC  1
#define RENDER_SERVER_THREAD_MODE_FIRST RENDER_SERVER_THREAD_MODE_SYNC
#define RENDER_SERVER_THREAD_MODE_LAST  RENDER_SERVER_THREDA_MODE_ASYNC
// clang-format on

// MACROS API END

/**
 * 0 - Rendering and game logic runs in one thread
 *
 * 1 - Rengering and game logic runs in separated threads
 *
 * @api
 */
typedef u8 RenderServerThreadMode;


// Static global RenderServer
extern RenderServerBackend RenderServer;

/**
 * @brief Initialize the static variables and default backends
 */
void render_server_init(void);

void render_server_exit(void);

/**
 * @brief Register a backend
 * @return "InvalidArgument" if name is NULL or backend is NULL
 * @return "AlreadyExists" if a backend with the same name is already registered
 *
 * @api
 */
boolean render_server_register_backend(const char* name, RenderServerBackend* backend);

/**
 * @brief Load a backend. First you should register them via render_server_register_backend
 * @warning If the backend is already loaded, this function does nothing.
 * @return "InvalidArgument" if name is NULL or th_mode is unknown
 * @return "NotFound" if a backend with the given name is not registered
 * @return "InvalidState" if the backend is already loaded
 *
 * @api
 */
boolean render_server_load_backend(const char* name, RenderServerThreadMode th_mode);

/**
 * @brief If backend was loaded
 */
boolean render_server_is_loaded(void);

/**
 * @brief Return thread mode of render server. Render server should be loaded.
 *
 * @api
 */
RenderServerThreadMode render_server_get_thread_mode(void);


/* ====================> Frame pipeline functions <==================== */
void render_server_begin_frame(void);

void render_server_end_frame(void);


/* ====================> RenderServer Async functions <==================== */
/**
 * @brief Deffered Function template
 *
 * @api
 */
typedef void (*RenderServerCallDefferedFunc)(void* ctx);


/**
 * @brief Call some function in the begin of next iteration of the render thread
 * @warning If used 'sync' mode, function will be called immediatly
 *
 * @param[in] function - your function to call
 * @param[in] ctx - your data. Just raw pointer, you can pass what you want
 *
 * Puts your function to the render's thread call queue.
 *
 * Example:
 *  You have a three functions:
 *      load_texture() - which loads a texture from HDD to VRAM
 *      load_mesh() - which loads mesh data (some buffers, like Vertices, indices, etc.) to VRAM
 *      create_instance() - which create new render instance with loaded texture and mesh data
 *
 *  In your logic code, when somebody (player) press key 'A', you call this functions in right order:
 *      call_deferred_render_thread(load_texture, &ctx);
 *      call_deferred_render_thread(load_mesh, &ctx);
 *      call_deferred_render_thread(create_instance, &ctx);
 *
 *  These functions will be added to the queue and will be executed with next render thread iteration
 *      load_texture -> load_mesh -> create_instance
 *
 *  And, yeah, it's manual memory management. You should manually create and destroy context
 *
 * @error InvalidArgument
 *
 * @api
 */
boolean call_deferred_render_thread(RenderServerCallDefferedFunc function, void* ctx);


/* ====================> RenderServerBackend functions <==================== */

/**
 * @brief Create a new RenderServerBackend instance
 * @return NULL if memory allocation fails
 *
 * @error "AllocationFailed"
 * @api
 */
RenderServerBackend* render_server_backend_new(void);

/**
 * @brieif Free a RenderServerBackend instance
 * @return "InvalidArgument" if backned is NULL
 *
 * @api
 */
boolean render_server_backend_free(RenderServerBackend* backend);

/**
 * @brief Set a function pointer for a backend
 * @return "InvalidArgument" if name is NULL or func is NULL
 * @return "NotFound" if a function with the given name does not exist in the backend
 *
 * @api
 */
boolean render_server_backend_set_function(
        RenderServerBackend* backend, const char* name, fptr function
);


/**
 * @brief Get a function pointer for a backend
 * @return "InvalidArgument" if backend is NULL or name is NULL or function is NULL
 * @return "NotFound" if a function with the given name is not registered
 *
 * @api
 */
fptr render_server_backend_get_function(RenderServerBackend* backend, const char* name);
