#include "camera.h"
#include <math.h>
#include <math/mat4.h>
#include "error.h"
#include "types/types.h"

boolean math_camera_perspective(
        float fov, float aspect_ratio, float z_near, float z_far, Mat4* const out
) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    const float f = 1 / tanf(fov / 2);

    Mat4 m = MAT4_ZERO_M;
    m.m[0][0] = f / aspect_ratio;
    m.m[1][1] = f;
    m.m[2][2] = (z_far + z_near) / (z_near - z_far);
    m.m[3][2] = (2 * z_far * z_near) / (z_near - z_far);
    m.m[2][3] = -1;
    *out = m;
    return true;
}

boolean math_camera_ortho(
        float left, float right, float bottom, float top, float z_near, float z_far, Mat4* const out
) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    Mat4 m = MAT4_ONE_M;
    m.m[0][0] = 2 / (right - left);
    m.m[1][1] = 2 / (top - bottom);
    m.m[2][2] = -2 / (z_far - z_near);
    m.m[3][0] = -(right + left) / (right - left);
    m.m[3][1] = -(top + bottom) / (top - bottom);
    m.m[3][2] = -(z_far + z_near) / (z_far - z_near);
    *out = m;
    return true;
}
