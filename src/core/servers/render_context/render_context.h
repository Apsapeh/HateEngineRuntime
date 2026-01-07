#pragma once

#include <core/error.h>
#include <core/types/types.h>
#include <core/types/signal.h>

#include <core/servers/platform_driver/platform_driver.h>

/**
 * @brief
 *
 * @api
 */
typedef struct RenderContextSurface RenderContextSurface;

/**
 * @api server
 * @api_config {
 *     "fn_prefix": "render_context_",
 *     "init_method": "___hate_engine_runtime_init_render_context"
 * }
 */
typedef struct {
    boolean (*_init)(void);
    boolean (*_quit)(void);

    // TODO:  change to StringSlice when fix/string PR is will be allowed
    SignalCallbackHandler (*signal_connect)(c_str name, SignalCallbackFunc func, void* ctx);
    boolean (*signal_disconnect)(c_str name, SignalCallbackHandler);
    i32 (*get_available_signals)(c_str* names_buff, c_str* descriptions_buff);


    RenderContextSurface* (*create_surface)(PlatformDriverWindow* window);
    boolean (*destroy_surface)(RenderContextSurface* surface);

    boolean (*surface_make_current)(RenderContextSurface* surface);
    boolean (*surface_present)(RenderContextSurface* surface);

    fptr (*get_proc_addr)(const char* proc);
} RenderContextBackend;


// Static global PlatformDriver
extern RenderContextBackend RenderContext;

/**
 * @brief Initialize the static variables and default backends
 */
void render_context_init(void);

void render_context_exit(void);

/**
 * @brief Register a backend
 * @return "InvalidArgument" if render_server_name is NULL or platform_driver_name is NULL or backend is
 * NULL
 * @return "AlreadyExists" if a backend with the same render_server_name and platform_driver_name is
 * already registered
 *
 * @api
 */
boolean render_context_register_backend(
        const char* render_server_name, const char* platform_driver_name, RenderContextBackend* backend
);

/**
 * @brief Load a backend. First you should register them via render_context_register_backend
 * @warning If the backend is already loaded, this function does nothing.
 * @return "InvalidArgument" if render_server_name is NULL or platform_driver_name
 * @return "NotFound" if a backend with the given names is not registered
 * @return "InvalidState" if the backend is already loaded
 *
 * @api
 */
boolean render_context_load_backend(const char* render_server_name, const char* platform_driver_name);

/**
 * @brief If backend was loaded
 */
boolean render_context_is_loaded(void);

/* ====================> RenderContextBackend functions <==================== */

/**
 * @brief Create a new RenderContextBackend instance
 * @return NULL if memory allocation fails
 *
 * @api
 */
RenderContextBackend* render_context_backend_new(void);

/**
 * @brieif Free a RenderContextBackend instance
 * @return "InvalidArgument" if backned is NULL
 *
 * @api
 */
boolean render_context_backend_free(RenderContextBackend* backend);

/**
 * @brief Set a function pointer for a backend
 * @return "InvalidArgument" if name is NULL or func is NULL
 * @return "NotFound" if a function with the given name does not exist in the backend
 *
 * @api
 */
boolean render_context_backend_set_function(
        RenderContextBackend* backend, const char* name, fptr function
);


/**
 * @brief Get a function pointer for a backend
 * @return "InvalidArgument" if backend is NULL or name is NULL or function is NULL
 * @return "NotFound" if a function with the given name is not registered
 *
 * @api
 */
fptr render_context_backend_get_function(RenderContextBackend* backend, const char* name);
