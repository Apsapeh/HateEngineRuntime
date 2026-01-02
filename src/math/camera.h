#pragma once

#include "mat4.h"
#include "types/types.h"

// Mat4 math_camera_view(float fov, float aspect_ratio, float z_near, float z_far) {
// }

/**
 * @brief Calculate a perspective matrix
 *
 * @param[in] fov FOV in radians
 *
 * @api
 */
boolean math_camera_perspective(
        float fov, float aspect_ratio, float z_near, float z_far, Mat4* const out
);

/**
 * @brief Calculate a perspective matrix
 *
 * @param[in] fov FOV in radians
 *
 * @api
 */
boolean math_camera_ortho(
        float left, float right, float bottom, float top, float z_near, float z_far, Mat4* const out
);
