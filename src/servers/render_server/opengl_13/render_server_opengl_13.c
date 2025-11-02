#include "render_server_opengl_13.h"
#include "servers/render_server/render_server.h"
#include "error.h"
#include "log.h"
#include "types/signal.h"
#include "types/types.h"
#include <servers/render_context/render_context.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad_ogl13/gl.h>

/* ====> Errors <==== */

/* ================== */

static boolean g_gloadLoaded = false;

static void init_glad_cb(void* args, void* ctx) {
    if (!gladLoadGL(RenderContext.get_proc_addr)) {
        LOG_FATAL("OpenGL load error (glad)")
    }
    g_gloadLoaded = true;
}

static boolean _init(void) {
    SignalCallbackHandler h = RenderContext.signal_connect("gl_context_created", init_glad_cb, NULL);
    if (h == 0) {
        return false;
    }

    return true;
}

static boolean _quit(void) {
    return true;
}

// static SignalCallback


#define REGISTER(fn) render_server_backend_set_function(backend, #fn, (void (*)(void)) fn)

void render_server_opengl_13_backend_register(void) {
    RenderServerBackend* backend = render_server_backend_new();

    REGISTER(_init);
    REGISTER(_quit);

    render_server_register_backend("OpenGL 1.3", backend);
}
