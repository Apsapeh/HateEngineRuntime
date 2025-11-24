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

RenderServerInstanceHandle instance;
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
    render_server_viewport_set_size(viewport2, IVEC2_NEW_M(1280, 720));

    RenderServerRenderTask* task2 = render_server_render_task_create();
    render_server_render_task_set_world(task2, world);
    render_server_render_task_set_viewport(task2, viewport2);

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
    //clang-format on

    RenderServerBufferHandle vertices_buffer = render_server_buffer_create(
            RENDER_SERVER_BUFFER_TYPE_VERTEX, RENDER_SERVER_BUFFER_USAGE_HINT_STATIC
    );
    render_server_buffer_set_data(
            vertices_buffer, vertices, sizeof(vertices), RENDER_SERVER_DATA_TYPE_F32,
            RENDER_SERVER_DATA_OWN_MODE_COPY
    );

    RenderServerBufferHandle indices_buffer = render_server_buffer_create(
            RENDER_SERVER_BUFFER_TYPE_INDEX, RENDER_SERVER_BUFFER_USAGE_HINT_STATIC
    );
    render_server_buffer_set_data(
            indices_buffer, indices, sizeof(indices), RENDER_SERVER_DATA_TYPE_U32,
            RENDER_SERVER_DATA_OWN_MODE_COPY
    );

    RenderServerMeshHandle mesh = render_server_mesh_create();
    render_server_mesh_set_vertices_buffer(mesh, vertices_buffer);
    render_server_mesh_set_indices_buffer(mesh, indices_buffer);

    instance = render_server_instance_create();
    render_server_instance_set_mesh(instance, mesh);

    render_server_world_add_instance(world, instance);
}



// Умножение двух матриц 4x4
Mat4 mat4_multiply(Mat4 a, Mat4 b) {
    Mat4 result = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

// Единичная матрица
Mat4 mat4_identity() {
    Mat4 result = {0};
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

// Поворот вокруг оси X (в радианах)
Mat4 mat4_rotate_x(float angle) {
    Mat4 result = mat4_identity();
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    result.m[1][1] = cos_a;
    result.m[1][2] = -sin_a;
    result.m[2][1] = sin_a;
    result.m[2][2] = cos_a;

    return result;
}

// Поворот вокруг оси Y (в радианах)
Mat4 mat4_rotate_y(float angle) {
    Mat4 result = mat4_identity();
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    result.m[0][0] = cos_a;
    result.m[0][2] = sin_a;
    result.m[2][0] = -sin_a;
    result.m[2][2] = cos_a;

    return result;
}

// Поворот вокруг оси Z (в радианах)
Mat4 mat4_rotate_z(float angle) {
    Mat4 result = mat4_identity();
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    result.m[0][0] = cos_a;
    result.m[0][1] = -sin_a;
    result.m[1][0] = sin_a;
    result.m[1][1] = cos_a;

    return result;
}

// Применить несколько поворотов к матрице
Mat4 mat4_apply_rotations(Mat4 matrix, float angle_x, float angle_y, float angle_z) {
    Mat4 rot_x = mat4_rotate_x(angle_x);
    Mat4 rot_y = mat4_rotate_y(angle_y);
    Mat4 rot_z = mat4_rotate_z(angle_z);

    // Порядок важен: обычно Z -> Y -> X (или X -> Y -> Z в зависимости от нужного эффекта)
    Mat4 combined = mat4_multiply(rot_y, rot_x);
    combined = mat4_multiply(rot_z, combined);

    return mat4_multiply(matrix, combined);
}

static Mat4 g_transform = MAT4_ONE_M;
PUBLIC void _render(double delta) {
    g_transform = mat4_apply_rotations(g_transform, delta * 0.1, delta * 0.05, 0);
    render_server_instance_set_transform(instance, g_transform);

}

PUBLIC void _process(double delta) {
}
