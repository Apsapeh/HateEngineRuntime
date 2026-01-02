#include "game_loader.h"
#include "game_loader_environment.h"

#include "log.h"
#include "servers/render_context/render_context.h"
#include "servers/render_server/render_server.h"
#include "servers/platform_driver/platform_driver.h"
#include <string.h>


void* runtime_proc_loader(const char* name);

#define CHECK_SERVER_LOAD(server, msg)                                                                  \
    do {                                                                                                \
        if (!server##_is_loaded()) {                                                                    \
            LOG_FATAL(msg);                                                                             \
        }                                                                                               \
    } while (0);

GameFunctions load_game(void) {
    GameFunctions game_functions;

    GameLoaderEnvironment fn_env = load_environment();

    // Stage 1: Init var-functions
    fn_env._runtime_init(runtime_proc_loader);

    // Stage 2: Setup config
    // TODO: Add _setup logic
    fn_env._setup();
    render_context_load_backend("OpenGL 1.3", "SDL3");
    CHECK_SERVER_LOAD(render_context, "RenderContext isn't loaded")

    platform_driver_load_backend("SDL3");
    CHECK_SERVER_LOAD(platform_driver, "PlatformDriver isn't loaded")

    render_server_load_backend("OpenGL 1.3", RENDER_SERVER_THREAD_MODE_SYNC);
    CHECK_SERVER_LOAD(render_server, "Render Server isn't loaded")

    RenderContext._init();
    PlatformDriver._init();
    RenderServer._init();

    // Stage 3: Init servers
    fn_env._render_context_init(&RenderContext);
    fn_env._platform_driver_init(&PlatformDriver);
    fn_env._render_server_init(&RenderServer);

    game_functions._ready = fn_env._ready;
    game_functions._process = fn_env._process;
    game_functions._physics_process = fn_env._physics_process;
    game_functions._render = fn_env._render;

    return game_functions;
}

#include <api_sym_lookup_table.h.gen>
// void* _runtime_proc_loader(const char* name) {
//     for (usize i = 0; i < sizeof(g_apiFunctionLookupTable) / sizeof(g_apiFunctionLookupTable[0]); i++)
//     {
//         if (strcmp(g_apiFunctionLookupTable[i].name, name) == 0) {
//             return g_apiFunctionLookupTable[i].ptr;
//         }
//     }
//     return NULL;
// }

void* runtime_proc_loader(const char* name) {
    usize left = 0;
    usize right = sizeof(g_apiFunctionLookupTable) / sizeof(g_apiFunctionLookupTable[0]) - 1;

    while (left <= right) {
        usize mid = left + (right - left) / 2;
        int cmp = strcmp(g_apiFunctionLookupTable[mid].name, name);

        if (cmp == 0)
            return g_apiFunctionLookupTable[mid].ptr;
        else if (cmp < 0)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return NULL;
}
