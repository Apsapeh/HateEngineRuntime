#pragma once

#include <error.h>
#include <types/types.h>
#include <types/signal.h>
#include "math/ivec2.h"
#include "math/mat4.h"
#include "math/vec4.h"
#include "ex_alloc/chunk_allocator.h"


/*
API ENUM {
        "name": "RenderServerBufferUsageHint",
        "type": "u8",
        "values": [
                ["Static", 0],
                ["Dynamic", 1],
                ["Stream", 2]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_BUFFER_USAGE_HINT_STATIC 0
#define RENDER_SERVER_BUFFER_USAGE_HINT_DYNAMIC 1
#define RENDER_SERVER_BUFFER_USAGE_HINT_STREAM 2
#define RENDER_SERVER_BUFFER_USAGE_HINT_FIRST RENDER_SERVER_BUFFER_USAGE_HINT_STATIC
#define RENDER_SERVER_BUFFER_USAGE_HINT_LAST RENDER_SERVER_BUFFER_USAGE_HINT_STREAM
// clang-format on

/**
 * 0 - Buffer will be setted once and naver changed.
 *     Use for static meshes like level, level environment, etc.
 *
 * 1 - Buffer will be sometimes change
 *     Use for maybe minecraft-like chunks, idk
 *
 * 2 - Buffer will be changed every few frames
 *     Use for particles
 *
 * @api
 */
typedef char RenderServerBufferUsageHint;


/*
API ENUM {
        "name": "RenderServerBufferType",
        "type": "u8",
        "values": [
                ["Vertex", 0],
                ["Index", 1],
                ["Normal", 2]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_BUFFER_TYPE_VERTEX 0
#define RENDER_SERVER_BUFFER_TYPE_INDEX 1
#define RENDER_SERVER_BUFFER_TYPE_NORMAL 2
#define RENDER_SERVER_BUFFER_TYPE_FIRST RENDER_SERVER_BUFFER_USAGE_HINT_VERTEX
#define RENDER_SERVER_BUFFER_TYPE_LAST RENDER_SERVER_BUFFER_USAGE_HINT_NORMAL
// clang-format on

/**
 * 0 - Vertices. Float types
 *
 * 1 - Indices. Indeger types
 *
 * 2 - Normals. Float types. Must be normalized
 *
 * @api
 */
typedef char RenderServerBufferType;


/*
API ENUM {
        "name": "RenderServerDataType",
        "type": "u8",
        "values": [
                ["F32", 0],
                ["F64", 1],
                ["I8",  2],
                ["I16", 3],
                ["I32", 4],
                ["U8",  5],
                ["U16", 6],
                ["U32", 7]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_DATA_TYPE_F32 0
#define RENDER_SERVER_DATA_TYPE_F64 1
#define RENDER_SERVER_DATA_TYPE_I8  2
#define RENDER_SERVER_DATA_TYPE_I16 3
#define RENDER_SERVER_DATA_TYPE_I32 4
#define RENDER_SERVER_DATA_TYPE_U8  5
#define RENDER_SERVER_DATA_TYPE_U16 6
#define RENDER_SERVER_DATA_TYPE_U32 7
#define RENDER_SERVER_DATA_TYPE_FIRST RENDER_SERVER_DATA_TYPE_F32
#define RENDER_SERVER_DATA_TYPE_LAST  RENDER_SERVER_DATA_TYPE_U32
// clang-format on

/**
 * 0 - f32, 4 bytes float point
 * 1 - f64, 8 bytes float point
 * 2 - i8,  1 byte signed integer
 * 3 - i16, 2 byte signed integer
 * 4 - i32, 4 byte signed integer
 * 5 - u8,  1 byte unsigned integer
 * 6 - u16, 2 byte unsigned integer
 * 7 - u32, 4 byte unsigned integer
 *
 * @api
 */
typedef u8 RenderServerDataType;


/*
API ENUM {
        "name": "RenderServerDataOwnMode",
        "type": "u8",
        "values": [
                ["Copy", 0],
                ["Borrow", 1],
                ["Ptr", 2]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_DATA_OWN_MODE_COPY   0
#define RENDER_SERVER_DATA_OWN_MODE_BORROW 1
#define RENDER_SERVER_DATA_OWN_MODE_PTR    2
#define RENDER_SERVER_DATA_OWN_MODE_FIRST RENDER_SERVER_DATA_OWN_MODE_COPY
#define RENDER_SERVER_DATA_OWN_MODE_LAST  RENDER_SERVER_DATA_OWN_MODE_PTR
// clang-format on

/**
 * 0 - Data may be copied, may be not, depending on the backend implementation.
 * RenderServer will own this data
 *
 * 1 - Data will be borrowed. RenderServer will own this data
 *
 * 2 - Data can be stored as a pointer without copying. Render server is not ownes data
 *
 * @api
 */
typedef u8 RenderServerDataOwnMode;


/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerWorldHandle;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerInstanceHandle;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerMeshHandle;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerBufferHandle;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerMaterialHandle;

/**
 * @brief
 *
 * @api
 */
typedef chunk_allocator_ptr RenderServerTextureHandle;

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

    boolean (*frame_begin)(void);
    boolean (*frame_end)(void);

    // Environment
    // RID (*environment_create)(void);
    // boolean (*environment_set_ambient_color)(RID rid, Vec4 color);
    // boolean (*environment_destroy)(RID rid);
    //

    // World (contains all data to draw)
    RenderServerWorldHandle (*world_create)(void);
    boolean (*world_add_instace)(RenderServerWorldHandle world, RenderServerInstanceHandle instance);
    boolean (*world_del_instace)(RenderServerWorldHandle world, RenderServerInstanceHandle instance);
    boolean (*world_set_ambient_color)(RenderServerWorldHandle world, Vec4 color);
    boolean (*world_destroy)(RenderServerWorldHandle world);

    // Instance (contains mesh, material, transform)
    RenderServerInstanceHandle (*instance_create)(void);
    boolean (*instance_set_mesh)(RenderServerInstanceHandle instance, RenderServerMeshHandle mesh);
    boolean (*instance_set_material)(
            RenderServerInstanceHandle instance, RenderServerMeshHandle material
    );
    boolean (*instance_set_transform)(RenderServerInstanceHandle instance, Mat4 trasform);
    boolean (*instance_destroy)(RenderServerInstanceHandle instance);

    // Mesh
    RenderServerMeshHandle (*mesh_create)(void);
    /**
     * @param type contains only float types 'fd'
     */
    boolean (*mesh_set_vertices_buffer)(RenderServerMeshHandle ptr, RenderServerBufferHandle buffer);
    /**
     * @param type contains only integer types 'bsiBSI'
     */
    boolean (*mesh_set_indices_buffer)(RenderServerMeshHandle ptr, RenderServerBufferHandle buffer);
    boolean (*mesh_destroy)(RenderServerMeshHandle ptr);

    // Buffer
    RenderServerBufferHandle (*buffer_create)(void);
    boolean (*buffer_set_data)(
            RenderServerBufferHandle ptr, const void* data, u64 data_size_in_bytes,
            RenderServerDataType data_type, RenderServerDataOwnMode data_own_mode,
            RenderServerBufferUsageHint usage_hint
    );
    boolean (*buffer_destroy)(RenderServerBufferHandle ptr);

    // Material
    RenderServerMaterialHandle (*material_create)(void);
    boolean (*material_set_albedo_texture)(
            RenderServerMaterialHandle ptr, RenderServerTextureHandle texture_rid
    );
    boolean (*material_destroy)(RenderServerMaterialHandle ptr);

    // Texture
    RenderServerTextureHandle (*texture_create)(void);
    boolean (*texture_set_data)(
            RenderServerTextureHandle ptr, const u8* const data, IVec2 dimensions,
            RenderServerDataOwnMode data_own_mode
    );
    boolean (*texture_destroy)(RenderServerTextureHandle ptr);

} RenderServerBackend;


// API START

// clang-format off
#define RENDER_SERVER_THREAD_MODE_SYNC   0
#define RENDER_SERVER_THREDA_MODE_ASYNC  1
#define RENDER_SERVER_THREAD_MODE_FIRST RENDER_SERVER_THREAD_MODE_SYNC
#define RENDER_SERVER_THREAD_MODE_LAST  RENDER_SERVER_THREDA_MODE_ASYNC
// clang-format on

// API END

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


/* ====================> RenderServer Async functions <==================== */
/**
 * @brief Deffered Function template
 *
 * @api
 */
typedef void (*CallDefferedRenderThreadFunctionPointer)(void* ctx);

void render_server_begin_frame(void);

void render_server_end_frame(void);

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
boolean call_deferred_render_thread(CallDefferedRenderThreadFunctionPointer function, void* ctx);


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
