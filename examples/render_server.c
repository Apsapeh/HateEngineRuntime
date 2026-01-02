#include <math.h>
#define HEAPI_LOAD_IMPL
// #define HEAPI_FULL_TRACE
#include <HateEngineAPI.h>

#include <stdio.h>
#include <stdlib.h>

#include <teapot.h>

static WindowServerWindow* g_win;
static RenderContextSurface* g_surface;

static WindowServerWindow* g_win2;
static RenderContextSurface* g_surface2;

static RenderServerCamera* g_camera;
Mat4 g_cameraView = MAT4_ONE_M;
static RenderServerCamera* g_camera2;

static RenderServerInstanceHandle g_instance;
static Mat4 g_transform = MAT4_ONE_M;
PUBLIC void _ready(void) {
    LOG_INFO("Hello ma boys!");
    g_win = window_server_create_window("Hello", IVEC2_NEW_M(1280, 720), NULL);
    ERROR_CATCH(g_win);

    g_surface = render_context_create_surface(g_win);
    ERROR_CATCH(g_surface);

    g_win2 = window_server_create_window("Hello2", IVEC2_NEW_M(1280, 720), NULL);
    ERROR_CATCH(g_win2);

    g_surface2 = render_context_create_surface(g_win2);
    ERROR_CATCH(g_surface2);

    const IVec2 viewport_size = IVEC2_NEW_M(1280, 720);

    RenderServerWorld* world = render_server_world_create();

    RenderServerViewport* viewport = render_server_viewport_surface_create(g_surface);
    render_server_viewport_set_size(viewport, &viewport_size);

    g_camera = render_server_camera_create();
    mat4_translate_in(&g_cameraView, 0, 0, 200);
    render_server_camera_view_set(g_camera, &g_cameraView);
    Mat4 proj_mat;
    float aspect = (float) viewport_size.x / (float) viewport_size.y;
    math_camera_perspective(DEG_TO_RAD_M(60), aspect, 0.1, 1000, &proj_mat);
    render_server_camera_projection_set(g_camera, &proj_mat);

    RenderServerRenderTask* task = render_server_render_task_create();
    render_server_render_task_set_world(task, world);
    render_server_render_task_set_viewport(task, viewport);
    render_server_render_task_set_camera(task, g_camera);

    RenderServerViewport* viewport1 = render_server_viewport_surface_create(g_surface);
    const IVec2 viewport1_size = IVEC2_NEW_M(640, 360);
    render_server_viewport_set_size(viewport1, &viewport1_size);

    RenderServerCamera* camera1 = render_server_camera_create();

    RenderServerRenderTask* task1 = render_server_render_task_create();
    render_server_render_task_set_world(task1, world);
    render_server_render_task_set_viewport(task1, viewport1);
    render_server_render_task_set_camera(task1, camera1);
    render_server_render_task_set_priority(task1, 100000);
    // render_server_render_task_set_state(task1, RENDER_SERVER_RENDER_TASK_STATE_DISABLED);

    RenderServerViewport* viewport2 = render_server_viewport_surface_create(g_surface2);
    render_server_viewport_set_size(viewport2, &viewport_size);

    g_camera2 = render_server_camera_create();

    RenderServerRenderTask* task2 = render_server_render_task_create();
    render_server_render_task_set_world(task2, world);
    render_server_render_task_set_viewport(task2, viewport2);
    render_server_render_task_set_camera(task2, g_camera2);
    // render_server_render_task_set_state(task2, RENDER_SERVER_RENDER_TASK_STATE_DISABLED)

    // clang-format off
    // Вершины куба(координаты x, y, z)
    float vertices[] = {
            // Передняя грань
            -0.5f, -0.5f, 0.5f, // 0: нижний левый перед
             0.5f, -0.5f, 0.5f, // 1: нижний правый перед
             0.5f,  0.5f, 0.5f, // 2: верхний правый перед
            -0.5f, 0.5f, 0.5f, // 3: верхний левый перед

            // Задняя грань
            -0.5f, -0.5f, -0.5f, // 4: нижний левый зад
            0.5f, -0.5f, -0.5f, // 5: нижний правый зад
            0.5f, 0.5f, -0.5f, // 6: верхний правый зад
            -0.5f, 0.5f, -0.5f // 7: верхний левый зад
    };

    // Индексы для треугольников (по 2 треугольника на грань)
    u32 indices[] = {
            // Передняя грань
            0, 1, 2, 2, 3, 0,
            // Задняя грань
            5, 4, 7, 7, 6, 5,
            // Верхняя грань
            3, 2, 6, 6, 7, 3,
            // Нижняя грань
            4, 5, 1, 1, 0, 4,
            // Правая грань
            1, 5, 6, 6, 2, 1,
            // Левая грань
            4, 0, 3, 3, 7, 4
    };
    // clang-format on

    RenderServerBufferHandle vertices_buffer = render_server_buffer_create(
            RENDER_SERVER_BUFFER_TYPE_VERTEX, RENDER_SERVER_BUFFER_USAGE_HINT_STATIC
    );
    render_server_buffer_set_data(
            vertices_buffer, VD, sizeof(VD), RENDER_SERVER_DATA_TYPE_F32, RENDER_SERVER_DATA_OWN_MODE_PTR
    );

    RenderServerBufferHandle indices_buffer = render_server_buffer_create(
            RENDER_SERVER_BUFFER_TYPE_INDEX, RENDER_SERVER_BUFFER_USAGE_HINT_STATIC
    );
    render_server_buffer_set_data(
            indices_buffer, ID, sizeof(ID), RENDER_SERVER_DATA_TYPE_U32, RENDER_SERVER_DATA_OWN_MODE_PTR
    );

    RenderServerMeshHandle mesh = render_server_mesh_create();
    render_server_mesh_set_vertices_buffer(mesh, vertices_buffer);
    render_server_mesh_set_indices_buffer(mesh, indices_buffer);

    g_instance = render_server_instance_create();
    render_server_instance_set_mesh(g_instance, mesh);

    render_server_world_add_instance(world, g_instance);

    // mat4_rotate_local_in(&g_transform, 0.2, 0, 0, 1);
    // mat4_translate_in(&g_transform, 0, 0, 3);
}


PUBLIC void _render(double delta) {
    // g_transform = mat4_apply_rotations(g_transform, delta * 0.1, delta * 0.05, 0);
    mat4_rotate_in(&g_transform, 0.1, 0, 1, 0);
    render_server_instance_set_transform(g_instance, &g_transform);

    // mat4_rotate_in(&g_cameraView, 0.03, 0, 1, 0);
    Vec3 v = VEC3_NEW_M(1, 2, 3);
    vec3_normalize_in(&v);
    mat4_rotate_local_in(&g_cameraView, 0.03, v.x, v.y, v.z);
    //    render_server_camera_view_set(g_camera, &g_cameraView);
}

PUBLIC void _process(double delta) {
}
