#include "platform_driver_sdl3.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "error.h"
#include "log.h"
#include "math/ivec2.h"
#include "math/vec2.h"
#include "servers/platform_driver/platform_driver.h"
#include "types/types.h"
#include <events/input_event/input_event.h>

/* ====> Errors <==== */
#define NOT_MAIN_THREAD_ERROR "SDL3NotMainThread"
#define ANY_ERROR "SDL3AnyError"
/* ================== */


#define MAIN_THREAD_CHECK(function, end_block)                                                          \
    do {                                                                                                \
        if (!SDL_IsMainThread()) {                                                                      \
            LOG_ERROR(                                                                                  \
                    "PlatformDriver(SDL3)::" #function " must be called only from main thread or "      \
                    "via call_deferred/call_deferred_async"                                             \
            )                                                                                           \
            set_error(NOT_MAIN_THREAD_ERROR);                                                           \
            end_block                                                                                   \
        }                                                                                               \
    } while (0)

const static u32 INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
static u8 g_isInit = 0;

static InputEvent g_inputEvent;

static boolean _init(void) {
    if (!SDL_WasInit(INIT_FLAGS)) {
        if (!SDL_Init(INIT_FLAGS)) {
            set_error(ANY_ERROR);
            return false;
        }
        g_isInit = 1;
    }

    input_event_constructor(&g_inputEvent);
    return true;
}

static boolean _quit(void) {
    if (g_isInit) {
        SDL_QuitSubSystem(INIT_FLAGS);
    }
    input_event_destructor(&g_inputEvent);
    return true;
}

static SDL_Event g_event;
static boolean _poll_events(void) {
    while (SDL_PollEvent(&g_event)) {
        if (g_event.type == SDL_EVENT_QUIT || g_event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            return false;
        } else {
            switch (g_event.type) {
                // Key up/down
                case SDL_EVENT_KEY_UP:
                case SDL_EVENT_KEY_DOWN:
                    LOG_WARN("%c", g_event.key.key);
                    input_event_set_type(&g_inputEvent, INPUT_EVENT_TYPE_KEY);
                    input_event_key_set_key(&g_inputEvent, g_event.key.scancode);
                    input_event_key_set_is_pressed(&g_inputEvent, g_event.key.down);
                    input_event_key_set_is_repeat(&g_inputEvent, g_event.key.repeat);
                    input_event_emit(&g_inputEvent);
                    break;
                // MouseButton up/down
                case SDL_EVENT_MOUSE_BUTTON_UP:
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    input_event_set_type(&g_inputEvent, INPUT_EVENT_TYPE_MOUSE_BUTTON);
                    input_event_mouse_button_set_button(&g_inputEvent, g_event.button.button);
                    input_event_mouse_button_set_is_pressed(&g_inputEvent, g_event.button.down);
                    input_event_mouse_button_set_clicks(&g_inputEvent, g_event.button.clicks);
                    Vec2 mouse_button_coords = VEC2_NEW_M(g_event.button.x, g_event.button.y);
                    input_event_mouse_button_set_coords(&g_inputEvent, &mouse_button_coords);
                    input_event_emit(&g_inputEvent);
                    break;
                // MouseMotion
                case SDL_EVENT_MOUSE_MOTION:
                    input_event_set_type(&g_inputEvent, INPUT_EVENT_TYPE_MOUSE_MOTION);
                    Vec2 mouse_motion_offset = VEC2_NEW_M(g_event.motion.xrel, g_event.motion.yrel);
                    Vec2 mouse_motion_coords = VEC2_NEW_M(g_event.motion.x, g_event.motion.y);
                    input_event_mouse_motion_set_offset(&g_inputEvent, &mouse_motion_offset);
                    input_event_mouse_motion_set_coords(&g_inputEvent, &mouse_motion_coords);
                    input_event_emit(&g_inputEvent);
                    break;
            }
        }
    };

    return true;
}


static PlatformDriverWindow* create_window(const char* title, IVec2 size, PlatformDriverWindow* parent) {
    ERROR_ARGS_CHECK_1(title, { return NULL; });
    MAIN_THREAD_CHECK(create_window, { return NULL; });

    SDL_Window* window = SDL_CreateWindow(title, size.x, size.y, SDL_WINDOW_OPENGL);
    if (!window) {
        LOG_ERROR("Failed to create windowv SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return NULL;
    }
    return (PlatformDriverWindow*) window;
}

static boolean destroy_window(PlatformDriverWindow* this) {
    ERROR_ARGS_CHECK_1(this, { return false; });
    MAIN_THREAD_CHECK(destroy_window, { return false; });

    SDL_DestroyWindow((SDL_Window*) this);
    return true;
}


static boolean window_set_title(PlatformDriverWindow* this, const char* title) {
    ERROR_ARGS_CHECK_2(this, title, { return false; });
    MAIN_THREAD_CHECK(window_set_title, { return false; });

    if (!SDL_SetWindowTitle((SDL_Window*) this, title)) {
        LOG_ERROR("Failed to set window title. SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return false;
    }
    return true;
}

static c_str window_get_title(PlatformDriverWindow* this) {
    ERROR_ARG_CHECK(this, { return NULL; });
    MAIN_THREAD_CHECK(window_get_title, { return NULL; });

    return SDL_GetWindowTitle((SDL_Window*) this);
}


static boolean window_set_mode(PlatformDriverWindow* this, PlatformDriverWindowMode mode) {
    ERROR_ARGS_CHECK_1(this, { return false; });
    MAIN_THREAD_CHECK(window_set_mode, { return false; });

    // TODO: Implement this function
    set_error(ERROR_NOT_IMPLEMENTED);
    return false;

    return true;
}

static PlatformDriverWindowMode window_get_mode(PlatformDriverWindow* this) {
    ERROR_ARGS_CHECK_1(this, { return PLATFORM_DRIVER_WINDOW_MODE_UNKNOWN; });
    MAIN_THREAD_CHECK(window_get_mode, { return PLATFORM_DRIVER_WINDOW_MODE_UNKNOWN; });

    // TODO: Implement this function
    set_error(ERROR_NOT_IMPLEMENTED);
    return PLATFORM_DRIVER_WINDOW_MODE_UNKNOWN;

    return PLATFORM_DRIVER_WINDOW_MODE_UNKNOWN;
}


static boolean window_set_size(PlatformDriverWindow* this, IVec2 dimensions) {
    ERROR_ARGS_CHECK_1(this, { return false; });
    MAIN_THREAD_CHECK(window_set_size, { return false; });

    if (!SDL_SetWindowSize((SDL_Window*) this, dimensions.x, dimensions.y)) {
        LOG_ERROR("Failed to set window size. SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return false;
    }
    return true;
}

static boolean window_get_size(PlatformDriverWindow* this, IVec2* out) {
    ERROR_ARGS_CHECK_2(this, out, { return false; });
    MAIN_THREAD_CHECK(window_get_size, { return false; });

    int out_w, out_h;
    if (!SDL_GetWindowSize((SDL_Window*) this, &out_w, &out_h)) {
        LOG_ERROR("Failed to get window size. SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return false;
    }

    out->x = (i32) out_w;
    out->y = (i32) out_h;
    return true;
}


static boolean window_set_position(PlatformDriverWindow* this, IVec2 dimensions) {
    ERROR_ARGS_CHECK_1(this, { return false; });
    MAIN_THREAD_CHECK(window_set_position, { return false; });

    if (!SDL_SetWindowPosition((SDL_Window*) this, dimensions.x, dimensions.y)) {
        LOG_ERROR("Failed to set window position. SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return false;
    }
    return true;
}

static boolean window_get_position(PlatformDriverWindow* this, IVec2* out) {
    ERROR_ARGS_CHECK_2(this, out, { return false; });
    MAIN_THREAD_CHECK(window_get_position, { return false; });

    int out_x, out_y;
    if (!SDL_GetWindowPosition((SDL_Window*) this, &out_x, &out_y)) {
        LOG_ERROR("Failed to get window position. SDL Error: %s", SDL_GetError());
        set_error(ANY_ERROR);
        return false;
    }

    out->x = (i32) out_x;
    out->y = (i32) out_y;
    return true;
}


#define REGISTER(fn) platform_driver_backend_set_function(ws, #fn, (void (*)(void)) fn)

void platform_driver_sdl3_backend_register(void) {
    PlatformDriverBackend* ws = platform_driver_backend_new();

    REGISTER(_init);
    REGISTER(_quit);
    REGISTER(_poll_events);

    REGISTER(create_window);
    REGISTER(destroy_window);

    REGISTER(window_set_title);
    REGISTER(window_get_title);

    REGISTER(window_set_mode);
    REGISTER(window_get_mode);

    REGISTER(window_set_size);
    REGISTER(window_get_size);

    REGISTER(window_set_position);
    REGISTER(window_get_position);

    // REGISTER(window_set_fullscreen_display);

    platform_driver_register_backend("SDL3", ws);
}
