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
        "name": "RenderServerDataOwnMode",
        "type": "char",
        "values": [
                ["Copy", "'\\0'"],
                ["Borrow", "'w'"],
                ["Ptr", "'f'"]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_DATA_OWN_MODE_COPY   '\0'
#define RENDER_SERVER_DATA_OWN_MODE_BORROW 'w'
#define RENDER_SERVER_DATA_OWN_MODE_PTR    'f'
// clang-format on

/**
 * '\0' or 'c' - Data may be copied, may be not, depending on the backend implementation.
 * RenderServer will own this data
 *
 * 'b' - Data will be borrowed. RenderServer will own this data
 *
 * 'p' - Data can be stored as a pointer without copying. Render server is not ownes data
 *
 * @api
 */
typedef char RenderServerDataOwnMode;


/*
API ENUM {
        "name": "RenderServerDataType",
        "type": "char",
        "values": [
                ["F32", "'f'"],
                ["F64", "'d'"],
                ["I8",  "'b'"],
                ["I16", "'s'"],
                ["I32", "'i'"],
                ["U8",  "'B'"],
                ["U16", "'S'"],
                ["U32", "'I'"]
        ]
}
*/

// clang-format off
#define RENDER_SERVER_DATA_TYPE_F32 'f'
#define RENDER_SERVER_DATA_TYPE_F64 'd'
#define RENDER_SERVER_DATA_TYPE_I8  'b'
#define RENDER_SERVER_DATA_TYPE_I16 's'
#define RENDER_SERVER_DATA_TYPE_I32 'i'
#define RENDER_SERVER_DATA_TYPE_U8  'B'
#define RENDER_SERVER_DATA_TYPE_U16 'S'
#define RENDER_SERVER_DATA_TYPE_U32 'I'
// clang-format on

/**
 * 'f' - f32, 4 bytes float point
 * 'd' - f64, 8 bytes float point
 * 'b' - i8,  1 byte signed integer
 * 's' - i16, 2 byte signed integer
 * 'i' - i32, 4 byte signed integer
 * 'B' - u8,  1 byte unsigned integer
 * 'S' - u16, 2 byte unsigned integer
 * 'I' - u32, 4 byte unsigned integer
 *
 * @api
 */
typedef char RenderServerDataType;


/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerWorldCPtr;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerInstanceCPtr;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerMeshCPtr;

/**
 * @api
 */
typedef chunk_allocator_ptr RenderServerMaterialCPtr;

/**
 * @brief
 *
 * @api
 */
typedef chunk_allocator_ptr RenderServerTextureCPtr;

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
    Signal* (*get_signal)(c_str name);

    boolean (*frame_begin)(void);
    boolean (*frame_end)(void);

    // Environment
    // RID (*environment_create)(void);
    // boolean (*environment_set_ambient_color)(RID rid, Vec4 color);
    // boolean (*environment_destroy)(RID rid);
    //

    // World (contains all data to draw)
    RenderServerWorldCPtr (*world_create)(void);
    boolean (*world_add_instace)(RenderServerWorldCPtr world, RenderServerInstanceCPtr instance);
    boolean (*world_del_instace)(RenderServerWorldCPtr world, RenderServerInstanceCPtr instance);
    boolean (*world_set_ambient_color)(RenderServerWorldCPtr world, Vec4 color);
    boolean (*world_destroy)(RenderServerWorldCPtr world);

    // Instance (contains mesh, material, transform)
    RenderServerInstanceCPtr (*instance_create)(void);
    boolean (*instance_set_mesh)(RenderServerInstanceCPtr instance, RenderServerMeshCPtr mesh);
    boolean (*instance_set_material)(RenderServerInstanceCPtr instance, RenderServerMeshCPtr material);
    boolean (*instance_set_transform)(RenderServerInstanceCPtr instance, Mat4 trasform);
    boolean (*instance_destroy)(RenderServerInstanceCPtr instance);

    // Mesh
    RenderServerMeshCPtr (*mesh_create)(void);
    /**
     * @param type contains only integer types 'bsiBSI'
     */
    boolean (*mesh_set_vertices)(
            RenderServerMeshCPtr ptr, const u8* const data, u64 size, RenderServerDataType type,
            RenderServerDataOwnMode data_own_mode
    );
    /**
     * @param type contains only integer types 'bsiBSI'
     */
    boolean (*mesh_set_indices)(
            RenderServerMeshCPtr ptr, const u8* const data, u64 size, RenderServerDataType type,
            RenderServerDataOwnMode data_own_mode
    );
    boolean (*mesh_destroy)(RenderServerMeshCPtr ptr);

    // Material
    RenderServerMaterialCPtr (*material_create)(void);
    boolean (*material_set_albedo_texture)(
            RenderServerMaterialCPtr ptr, RenderServerTextureCPtr texture_rid
    );
    boolean (*material_destroy)(RenderServerMaterialCPtr ptr);

    // Texture
    RenderServerTextureCPtr (*texture_create)(void);
    boolean (*texture_set_data)(
            RenderServerTextureCPtr ptr, const u8* const data, IVec2 dimensions,
            RenderServerDataOwnMode data_own_mode
    );
    boolean (*texture_destroy)(RenderServerTextureCPtr ptr);

} RenderServerBackend;


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
 * @return "InvalidArgument" if name is NULL
 * @return "NotFound" if a backend with the given name is not registered
 * @return "InvalidState" if the backend is already loaded
 *
 * @api
 */
boolean render_server_load_backend(const char* name);

/**
 * @brief If backend was loaded
 */
boolean render_server_is_loaded(void);


/* ====================> RenderServerBackend functions <==================== */

/**
 * @brief Create a new RenderServerBackend instance
 * @return NULL if memory allocation fails
 *
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
