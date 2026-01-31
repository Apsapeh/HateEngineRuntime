#pragma once

#include <core/error.h>
#include <core/types/types.h>
#include <core/math/ivec2.h>

/* ============> Window VSync <============ */
/*
u8 PlatformDriverWindowVsync

Unknown
Disabled
Enabled
EnabledAsync
Count
 */

/**
 * @api
 */
enum {
    PLATFORM_DRIVER_WINDOW_VSYNC_UNKNOWN = 0,
    PLATFORM_DRIVER_WINDOW_VSYNC_DISABLED = 1,
    PLATFORM_DRIVER_WINDOW_VSYNC_ENABLED = 2,
    PLATFORM_DRIVER_WINDOW_VSYNC_ENABLED_ASYNC = 3,
    PLATFORM_DRIVER_WINDOW_VSYNC_COUNT = 4,
};

/**
 * 0 - Unknown

 * 1 - Disabled

 * 2 - Enabled

 * 3 - EnabledAsync

 * 4 - Count

 * @api
 */
typedef u8 PlatformDriverWindowVsync;


/* ============> Window Mode <============ */
/*
u8 PlatformDriverWindowMode

Unknown
Windowed
Fullscreen
BorderlessFullscreen
Count
 */

/**
 * @api
 */
enum {
    PLATFORM_DRIVER_WINDOW_MODE_UNKNOWN = 0,
    PLATFORM_DRIVER_WINDOW_MODE_WINDOWED = 1,
    PLATFORM_DRIVER_WINDOW_MODE_FULLSCREEN = 2,
    PLATFORM_DRIVER_WINDOW_MODE_BORDERLESS_FULLSCREEN = 3,
    PLATFORM_DRIVER_WINDOW_MODE_COUNT = 4,
};

/**
 * 0 - Unknown

 * 1 - Windowed

 * 2 - Fullscreen

 * 3 - BorderlessFullscreen

 * 4 - Count

 * @api
 */
typedef u8 PlatformDriverWindowMode;


/**
 * @brief
 *
 * @api
 */
typedef struct PlatformDriverWindow PlatformDriverWindow;

/**
 * @brief
 *
 * @api
 */
typedef struct PlatformDriverDisplay PlatformDriverDisplay;


/**
 * @api server
 * @api_config {
 *     "fn_prefix": "platform_driver_",
 *     "init_method": "___hate_engine_runtime_init_platform_driver"
 * }
 */
typedef struct {
    boolean (*_init)(void);
    boolean (*_quit)(void);

    boolean (*_poll_events)(void);

    PlatformDriverWindow* (*create_window)(const char* title, IVec2 size, PlatformDriverWindow* parent);
    boolean (*destroy_window)(PlatformDriverWindow* this);

    boolean (*window_set_title)(PlatformDriverWindow* this, const char* title);
    c_str (*window_get_title)(PlatformDriverWindow* this);

    boolean (*window_set_mode)(PlatformDriverWindow* this, PlatformDriverWindowMode mode);
    PlatformDriverWindowMode (*window_get_mode)(PlatformDriverWindow* this);

    boolean (*window_set_size)(PlatformDriverWindow* this, IVec2 dimensions);
    boolean (*window_get_size)(PlatformDriverWindow* this, IVec2* out);

    boolean (*window_set_position)(PlatformDriverWindow* this, IVec2 dimensions);
    boolean (*window_get_position)(PlatformDriverWindow* this, IVec2* out);

    // boolean (*window_set_fullscreen_display)(PlatformDriverWindow* this, PlatformDriverDisplay*
    // fullscreen);
} PlatformDriverBackend;


// Static global PlatformDriver
extern PlatformDriverBackend PlatformDriver;

/**
 * @brief Initialize the static variables and default backends
 */
void platform_driver_init(void);

void platform_driver_exit(void);

/**
 * @brief Register a backend
 * @return "InvalidArgument" if name is NULL or backend is NULL
 * @return "AlreadyExists" if a backend with the same name is already registered
 *
 * @api
 */
boolean platform_driver_register_backend(const char* name, PlatformDriverBackend* backend);

/**
 * @brief Load a backend. First you should register them via platform_driver_register_backend
 * @warning If the backend is already loaded, this function does nothing.
 * @return "InvalidArgument" if name is NULL
 * @return "NotFound" if a backend with the given name is not registered
 * @return "InvalidState" if the backend is already loaded
 *
 * @api
 */
boolean platform_driver_load_backend(const char* name);

/**
 * @brief If backend was loaded
 */
boolean platform_driver_is_loaded(void);


/* ====================> PlatformDriverBackend functions <==================== */

/**
 * @brief Create a new PlatformDriverBackend instance
 * @return NULL if memory allocation fails
 *
 * @api
 */
PlatformDriverBackend* platform_driver_backend_new(void);

/**
 * @brieif Free a PlatformDriverBackend instance
 * @return "InvalidArgument" if backned is NULL
 *
 * @api
 */
boolean platform_driver_backend_free(PlatformDriverBackend* backend);

/**
 * @brief Set a function pointer for a backend
 * @return "InvalidArgument" if name is NULL or func is NULL
 * @return "NotFound" if a function with the given name does not exist in the backend
 *
 * @api
 */
boolean platform_driver_backend_set_function(
        PlatformDriverBackend* backend, const char* name, fptr function
);


/**
 * @brief Get a function pointer for a backend
 * @return "InvalidArgument" if backend is NULL or name is NULL or function is NULL
 * @return "NotFound" if a function with the given name is not registered
 *
 * @api
 */
fptr platform_driver_backend_get_function(PlatformDriverBackend* backend, const char* name);
