#include "mat4.h"
#include <core/error.h>
#include <math.h>

boolean mat4_init(
        float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33,
        Mat4* const out
) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    out->m[0][0] = m00;
    out->m[0][1] = m01;
    out->m[0][2] = m02;
    out->m[0][3] = m03;
    out->m[1][0] = m10;
    out->m[1][1] = m11;
    out->m[1][2] = m12;
    out->m[1][3] = m13;
    out->m[2][0] = m20;
    out->m[2][1] = m21;
    out->m[2][2] = m22;
    out->m[2][3] = m23;
    out->m[3][0] = m30;
    out->m[3][1] = m31;
    out->m[3][2] = m32;
    out->m[3][3] = m33;
    return true;
}

#define MAT_TYPE Mat4
#define MAT_FN_NAME mat4
#define MAT_SIZE 4

#include "matN_general_impl.h"


/* ===========================> Afine <============================ */

boolean mat4_translate_make(float tx, float ty, float tz, Mat4* const out) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    *out = MAT4_ONE_M;
    out->m[3][0] = tx;
    out->m[3][1] = ty;
    out->m[3][2] = tz;
    return true;
}

boolean mat4_translate(const Mat4* const a, float tx, float ty, float tz, Mat4* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    *out = *a;
    return mat4_translate_in(out, tx, ty, tz);
}

boolean mat4_translate_in(Mat4* const m, float tx, float ty, float tz) {
    ERROR_ARGS_CHECK_1(m, { return false; });
    m->m[3][0] += (tx * m->m[0][0] + ty * m->m[1][0] + tz * m->m[2][0]);
    m->m[3][1] += (tx * m->m[0][1] + ty * m->m[1][1] + tz * m->m[2][1]);
    m->m[3][2] += (tx * m->m[0][2] + ty * m->m[1][2] + tz * m->m[2][2]);
    return true;
}

boolean mat4_translate_local(const Mat4* const a, float tx, float ty, float tz, Mat4* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    *out = *a;
    return mat4_translate_local_in(out, tx, ty, tz);
}

boolean mat4_translate_local_in(Mat4* const a, float tx, float ty, float tz) {
    ERROR_ARGS_CHECK_1(a, { return false; });
    a->m[3][0] += tx;
    a->m[3][1] += ty;
    a->m[3][2] += tz;
    return true;
}


boolean mat4_scale_make(float sx, float sy, float sz, Mat4* const out) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    *out = MAT4_ONE_M;
    out->m[0][0] = sx;
    out->m[1][1] = sy;
    out->m[2][2] = sz;
    return true;
}

boolean mat4_scale(const Mat4* const m, float sx, float sy, float sz, Mat4* const out) {
    ERROR_ARGS_CHECK_2(m, out, { return false; });
    Mat4 scale_mat;
    mat4_scale_make(sx, sy, sz, &scale_mat);
    mat4_mul(&scale_mat, m, out);
    return true;
}

boolean mat4_scale_in(Mat4* const m, float sx, float sy, float sz) {
    ERROR_ARGS_CHECK_1(m, { return false; });
    Mat4 temp;
    mat4_scale(m, sx, sy, sz, &temp);
    *m = temp;
    return true;
}

boolean mat4_scale_local(const Mat4* const a, float sx, float sy, float sz, Mat4* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    *out = *a;
    return mat4_scale_local_in(out, sx, sy, sz);
}

boolean mat4_scale_local_in(Mat4* const m, float sx, float sy, float sz) {
    ERROR_ARGS_CHECK_1(m, { return false; });
    // clang-format off
    m->m[0][0] *= sx; m->m[0][1] *= sx; m->m[0][2] *= sx;
    m->m[1][0] *= sy; m->m[1][1] *= sy; m->m[1][2] *= sy;
    m->m[2][0] *= sz; m->m[2][1] *= sz; m->m[2][2] *= sz;
    // clang-format on
    return true;
}


boolean mat4_rotate_make(float angle, float ax, float ay, float az, Mat4* const out) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    const float c = cosf(-angle);
    const float s = sinf(-angle);
    const float ic = 1.0f - c;

    const float xy = ax * ay, xz = ax * az, yz = ay * az;

    *out = MAT4_ONE_M;

    out->m[0][0] = c + ax * ax * ic;
    out->m[0][1] = xy * ic - az * s;
    out->m[0][2] = xz * ic + ay * s;

    out->m[1][0] = xy * ic + az * s;
    out->m[1][1] = c + ay * ay * ic;
    out->m[1][2] = yz * ic - ax * s;

    out->m[2][0] = xz * ic - ay * s;
    out->m[2][1] = yz * ic + ax * s;
    out->m[2][2] = c + az * az * ic;

    return true;
}

boolean mat4_rotate(const Mat4* const m, float angle, float ax, float ay, float az, Mat4* const out) {
    ERROR_ARGS_CHECK_2(m, out, { return false; });
    Mat4 rot;
    mat4_rotate_make(angle, ax, ay, az, &rot);
    mat4_mul(m, &rot, out);
    return true;
}

boolean mat4_rotate_in(Mat4* const m, float angle, float ax, float ay, float az) {
    ERROR_ARGS_CHECK_1(m, { return false; });
    Mat4 temp;
    mat4_rotate(m, angle, ax, ay, az, &temp);
    *m = temp;
    return true;
}

boolean mat4_rotate_local(
        const Mat4* const m, float angle, float ax, float ay, float az, Mat4* const out
) {
    ERROR_ARGS_CHECK_2(m, out, { return false; });
    Mat4 rot;
    mat4_rotate_make(angle, ax, ay, az, &rot);
    mat4_mul(&rot, m, out);
    return true;
}

boolean mat4_rotate_local_in(Mat4* const m, float angle, float ax, float ay, float az) {
    ERROR_ARGS_CHECK_1(m, { return false; });
    Mat4 temp;
    mat4_rotate_local(m, angle, ax, ay, az, &temp);
    *m = temp;
    return true;
}
