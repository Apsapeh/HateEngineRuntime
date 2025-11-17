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
    g_win = window_server_create_window("Hello", IVEC2_NEW_M(800, 600), NULL);
    if (!g_win) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_surface = render_context_create_surface(g_win);
    if (!g_surface) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_win2 = window_server_create_window("Hello2", IVEC2_NEW_M(800, 600), NULL);
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
}

static int g_count = 0;
double g_time = 0;
PUBLIC void _process(double delta) {
    // printf("Process %d\n", count);
    g_time += delta;
    g_count++;

    render_context_surface_make_current(g_surface);
    render_server_frame_begin();

    render_server_frame_end();
    render_context_surface_present(g_surface);


    return;

    if (g_count == 20000000) {
        printf("Exit\n");
        exit(0);
    }
}
