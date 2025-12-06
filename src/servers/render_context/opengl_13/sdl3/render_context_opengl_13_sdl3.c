#include "SDL3/SDL_error.h"
#include "platform/memory.h"
#include "render_context_opengl_13_sdl3.h"
#include "log.h"
#include "error.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

#include "servers/render_context/render_context.h"
#include "servers/platform_driver/sdl3/platform_driver_sdl3.h"
#include "servers/platform_driver/platform_driver.h"
#include "types/signal.h"
#include "types/types.h"

/*struct RenderContextSurface {
    int a;
};*/

/* ====> Errors <==== */
#define NOT_MAIN_THREAD_ERROR "SDL3NotMainThread"
#define ANY_ERROR "SDL3AnyError"
/* ================== */


#define MAIN_THREAD_CHECK(function, end_block)                                                          \
    do {                                                                                                \
        if (!SDL_IsMainThread()) {                                                                      \
            LOG_ERROR(                                                                                  \
                    "RenderContext(OpenGL 1.3, SDL3)::" #function                                       \
                    " must be called only from main thread or "                                         \
                    "via call_deferred/call_deferred_async"                                             \
            )                                                                                           \
            set_error(NOT_MAIN_THREAD_ERROR);                                                           \
            end_block                                                                                   \
        }                                                                                               \
    } while (0)


const static u32 INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
static u8 g_isInit = 0;

struct ServerSignal {
    Signal* signal;
    c_str name;
    c_str description;
};

// clang-format off
static Signal g_signalGlContextCreated;

#define SERVER_SIGNAL(_signal, _name, _description) (struct ServerSignal) {.signal = _signal, .name = _name, .description = _description}
static struct ServerSignal g_signals[] = {
    SERVER_SIGNAL(&g_signalGlContextCreated, "gl_context_created", "")
};
// clang-format on

static boolean _init(void) {
    if (!SDL_WasInit(INIT_FLAGS)) {
        if (!SDL_Init(INIT_FLAGS)) {
            set_error(ANY_ERROR);
            return false;
        }
        g_isInit = 1;
    }

    for (usize i = 0; i < (sizeof(g_signals) / sizeof(g_signals[0])); ++i) {
        if (!signal_constructor(g_signals[i].signal)) {
            LOG_ERROR("RenderContext::_init: Signal initializaito failed");
            return false;
        }
    }

    return true;
}

static boolean _quit(void) {
    if (g_isInit) {
        SDL_QuitSubSystem(INIT_FLAGS);
    }

    for (usize i = 0; i < (sizeof(g_signals) / sizeof(g_signals[0])); ++i) {
        signal_destructor(g_signals[i].signal);
    }
    return true;
}


static SignalCallbackHandler impl_signal_connect(c_str name, SignalCallbackFunc func, void* ctx) {
    ERROR_ARGS_CHECK_2(name, func, { return 0; });

    for (usize i = 0; i < (sizeof(g_signals) / sizeof(g_signals[0])); ++i) {
        if (strcmp(name, g_signals[i].name) == 0) {
            return signal_connect(g_signals[i].signal, func, ctx);
        }
    }

    LOG_WARN("RenderContext(OpenGL 1.3, SDL3)::signal_connect: Signal '%s' not found", name);
    set_error(ERROR_NOT_FOUND);
    return 0;
}

static boolean impl_signal_disconnect(c_str name, SignalCallbackHandler handler) {
    ERROR_ARGS_CHECK_2(name, handler, { return false; });

    for (usize i = 0; i < (sizeof(g_signals) / sizeof(g_signals[0])); ++i) {
        if (strcmp(name, g_signals[i].name) == 0) {
            return signal_disconnect(g_signals[i].signal, handler);
        }
    }

    LOG_WARN("RenderContext.signal_disconnect: Signal '%s' not found", name);
    set_error(ERROR_NOT_FOUND);
    return false;
}

static i32 get_available_signals(c_str* names_buff, c_str* descriptions_buff) {
    const usize signals_count = sizeof(g_signals) / sizeof(g_signals[0]);

    if (names_buff) {
        names_buff = tmalloc(sizeof(c_str) * signals_count);
        ERROR_ALLOC_CHECK(names_buff, { return 0; });
    }

    if (descriptions_buff) {
        descriptions_buff = tmalloc(sizeof(c_str) * signals_count);
        ERROR_ALLOC_CHECK(descriptions_buff, {
            tfree(names_buff);
            return 0;
        });
    }

    for (usize i = 0; i < (sizeof(g_signals) / sizeof(g_signals[0])); ++i) {
        if (names_buff)
            names_buff[i] = g_signals[i].name;
        if (descriptions_buff)
            descriptions_buff[i] = g_signals[i].description;
    }

    return signals_count;
}


static SDL_GLContext g_openglSharedContext = NULL;
static RenderContextSurface* create_surface(PlatformDriverWindow* window) {
    ERROR_ARGS_CHECK_1(window, { return NULL; });
    MAIN_THREAD_CHECK(create_surface, { return NULL; });

    if (!g_openglSharedContext) {
        g_openglSharedContext = SDL_GL_CreateContext((SDL_Window*) window);

        if (!g_openglSharedContext) {
            LOG_FATAL(
                    "RenderContext.create_surface: Failed to create shared OpenGL context. SDL Error: "
                    "%s",
                    SDL_GetError()
            );
            set_error(ANY_ERROR);
            return NULL;
        }

        signal_emit(&g_signalGlContextCreated, NULL);
    }

    SDL_GL_SetSwapInterval(0);

    return (RenderContextSurface*) window;
}


static boolean destroy_surface(RenderContextSurface* surface) {
    ERROR_ARG_CHECK(surface, { return false; });
    return true;
}


static RenderContextSurface* g_currentSurface = NULL;
static boolean surface_make_current(RenderContextSurface* surface) {
    ERROR_ARG_CHECK(surface, { return false; });
    MAIN_THREAD_CHECK(create_surface, { return false; });

    if (surface == g_currentSurface) {
        return true;
    }

    if (!SDL_GL_MakeCurrent((SDL_Window*) surface, g_openglSharedContext)) {
        LOG_FATAL(
                "RenderContext.surface_make_current: Failed to change OpenGL context. SDL Error: %s",
                SDL_GetError()
        );
        set_error(ANY_ERROR);
        return false;
    }
    g_currentSurface = surface;

    return true;
}


static boolean surface_present(RenderContextSurface* surface) {
    ERROR_ARG_CHECK(surface, { return false; });
    MAIN_THREAD_CHECK(create_surface, { return false; });

    if (surface != g_currentSurface) {
        if (!surface_make_current(surface))
            return false;
    }
    if (!SDL_GL_SwapWindow((SDL_Window*) surface)) {
        LOG_FATAL(
                "RenderContext.surface_present: Failed to swap window's buffers. SDL Error: %s",
                SDL_GetError()
        );
        set_error(ANY_ERROR);
        return false;
    }
    return true;
}

#define REGISTER_WITH_NAME(name, fn) render_context_backend_set_function(backend, name, (fptr) fn)
#define REGISTER(fn) REGISTER_WITH_NAME(#fn, fn)

void render_context_opengl_13_sdl3_backend_register(void) {
    RenderContextBackend* backend = render_context_backend_new();

    REGISTER(_init);
    REGISTER(_quit);

    REGISTER_WITH_NAME("signal_connect", impl_signal_connect);
    REGISTER_WITH_NAME("signal_disconnect", impl_signal_disconnect);
    REGISTER(get_available_signals);

    REGISTER(create_surface);
    REGISTER(destroy_surface);
    REGISTER(surface_make_current);
    REGISTER(surface_present);

    REGISTER_WITH_NAME("get_proc_addr", SDL_GL_GetProcAddress);

    render_context_register_backend("OpenGL 1.3", "SDL3", backend);
}
