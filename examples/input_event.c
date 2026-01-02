#include <math.h>
#define HEAPI_LOAD_IMPL
// #define HEAPI_FULL_TRACE
#include <HateEngineAPI.h>

#include <stdio.h>
#include <stdlib.h>

static void event_callback(const InputEvent* const event) {
    printf("New event\n");
}

PlatformDriverWindow* g_win;
RenderContextSurface* g_surface;

PlatformDriverWindow* g_win2;
RenderContextSurface* g_surface2;
PUBLIC void _ready(void) {
    vfs_mount_rfs("/");
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);
    input_event_connect(&event_callback);

    printf("wscw: %p\n", (void*) raw_platform_driver_create_window);
    printf("wswss: %p\n", (void*) raw_platform_driver_window_set_size);
    g_win = platform_driver_create_window("Hello", IVEC2_NEW_M(800, 600), NULL);
    if (!g_win) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }


    g_surface = render_context_create_surface(g_win);
    if (!g_surface) {
        printf("СМЭРТЬ: %s\n", get_error());
        exit(1);
    }

    g_win2 = platform_driver_create_window("Hello2", IVEC2_NEW_M(800, 600), NULL);
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

    printf("Used mem: %zu\n", get_allocated_memory());
    u64 size;
    char* data = vfs_res_read_file("/assets/text.txt", &size);

    if (data) {
        printf("Data: %s\n", data);
        tfree(data);
    } else {
        printf("Failed to load data\n");
    }
    printf("Used mem: %zu\n", get_allocated_memory());

    printf("Ptr: %p\n", (void*) raw_node_new);
    Node* node = node_new("Jopa");
    const char* name = node_get_name(node);

    printf("%s\n", name);

    node_set_name(node, "Node 2");
    name = node_get_name(node);
    printf("%s\n", name);

    Vec3 pos = VEC3_NEW_M(1, 2, 3);
    vec3_add_in(&pos, &pos);
    printf("%f %f %f\n", pos.x, pos.y, pos.z);
    vec3_normalize_in(&pos);
    printf("%f %f %f\n", pos.x, pos.y, pos.z);


    auto_free((Object*) node);
    vfs_unmount_rfs();
}

static int g_count = 0;
double g_time = 0;
PUBLIC void _process(double delta) {
    g_time += delta;
    g_count++;

    render_context_surface_present(g_surface);
    render_context_surface_present(g_surface2);

    return;

    if (g_count == 20000000) {
        printf("Exit\n");
        exit(0);
    }
}
