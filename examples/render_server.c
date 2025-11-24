#include <math.h>
#define HEAPI_LOAD_IMPL
// #define HEAPI_FULL_TRACE
#include <HateEngineAPI.h>

#include <stdio.h>
#include <stdlib.h>

WindowServerWindow* g_win;
RenderContextSurface* g_surface;

WindowServerWindow* g_win2;
RenderContextSurface* g_surface2;
PUBLIC void _ready(void) {
    g_win = window_server_create_window("Hello", IVEC2_NEW_M(1280, 720), NULL);
    if (!g_win) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_surface = render_context_create_surface(g_win);
    if (!g_surface) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_win2 = window_server_create_window("Hello2", IVEC2_NEW_M(1280, 720), NULL);
    if (!g_win2) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_surface2 = render_context_create_surface(g_win2);
    if (!g_surface2) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }
    // window_server_destroy_window(win);*/


    RenderServerWorld* world = render_server_world_create();

    RenderServerViewport* viewport = render_server_viewport_surface_create(g_surface);
    render_server_viewport_set_size(viewport, IVEC2_NEW_M(1280, 720));

    RenderServerRenderTask* task = render_server_render_task_create();
    render_server_render_task_set_world(task, world);
    render_server_render_task_set_viewport(task, viewport);
    // render_server_render_task_set_state(task, RENDER_SERVER_RENDER_TASK_STATE_DISABLED);

    RenderServerViewport* viewport2 = render_server_viewport_surface_create(g_surface2);
    render_server_viewport_set_size(viewport, IVEC2_NEW_M(1280, 720));

    RenderServerRenderTask* task2 = render_server_render_task_create();
    render_server_render_task_set_world(task2, world);
    render_server_render_task_set_viewport(task2, viewport2);
}

PUBLIC void _render(double delta) {
}

PUBLIC void _process(double delta) {
}
