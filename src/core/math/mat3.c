#include "mat3.h"
#include <core/error.h>
#include <core/types/types.h>

boolean mat3_init(
        float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
        float m22, Mat3* const out
) {
    ERROR_ARGS_CHECK_1(out, { return false; });
    out->m[0][0] = m00;
    out->m[0][1] = m01;
    out->m[0][2] = m02;
    out->m[1][0] = m10;
    out->m[1][1] = m11;
    out->m[1][2] = m12;
    out->m[2][0] = m20;
    out->m[2][1] = m21;
    out->m[2][2] = m22;
    return true;
}

#define MAT_TYPE Mat3
#define MAT_FN_NAME mat3
#define MAT_SIZE 3

#include "matN_general_impl.h"
