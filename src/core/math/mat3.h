#pragma once

#include <core/types/types.h>

/**
 * @brief Primitive 3x3 matrix
 *
 * Raw data - m
 *
 * @api forward
 */
typedef struct {
    float m[3][3];
} Mat3;

// MACROS API START
// clang-format off
#define MAT3_NEW_M(                                                                                     \
        v_m00, v_m01, v_m02, v_m10, v_m11, v_m12, v_m20, v_m21, v_m22                                   \
)                                                                                                       \
    (Mat3) {                                                                                            \
        .m = {                                                                                          \
            v_m00,                                                                                      \
            v_m01,                                                                                      \
            v_m02,                                                                                      \
            v_m10,                                                                                      \
            v_m11,                                                                                      \
            v_m12,                                                                                      \
            v_m20,                                                                                      \
            v_m21,                                                                                      \
            v_m22,                                                                                      \
        }                                                                                               \
    }

#define MAT3_ZERO_M                                                                                     \
    MAT3_NEW_M(                                                                                         \
        0.0f, 0.0f, 0.0f,                                                                               \
        0.0f, 0.0f, 0.0f,                                                                               \
        0.0f, 0.0f, 0.0f,                                                                               \
    )
    
#define MAT3_ONE_M                                                                                      \
    MAT3_NEW_M(                                                                                         \
        1.0f, 0.0f, 0.0f                                                                                \
        0.0f, 1.0f, 0.0f                                                                                \
        0.0f, 0.0f, 1.0f                                                                                \
    )
// clang-format on

// MACROS API END

/**
 * @brief
 *
 * @param m00
 * @param m01
 * @param m02
 * @param m10
 * @param m11
 * @param m12
 * @param m20
 * @param m21
 * @param m22
 * @param [out] Result Mat3
 *
 * @api
 */
boolean mat3_init(
        float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
        float m22, Mat3* const out
);

/**
 * @brief
 *
 * @param array
 * @return Mat3
 *
 * @api
 */
boolean mat3_from_array(const float* const array, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @return Mat3
 *
 * @api
 */
boolean mat3_clone(const Mat3* const a, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @param b
 * @return Mat3
 *
 * @api
 */
boolean mat3_add(const Mat3* const a, const Mat3* const b, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @param b
 * @return Mat3
 *
 * @api
 */
boolean mat3_sub(const Mat3* const a, const Mat3* const b, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @param b
 * @return Mat3
 *
 * @api
 */
boolean mat3_mul(const Mat3* const a, const Mat3* const b, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @param factor
 * @return Mat3
 *
 * @api
 */
boolean mat3_factor(const Mat3* const a, const float factor, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @return Mat3
 *
 * @api
 */
boolean mat3_transpose(const Mat3* const a, Mat3* const out);

/**
 * @brief
 *
 * @param a
 * @return Mat3
 *
 * @api
 */
boolean mat3_inverse(const Mat3* const a, Mat3* const out);
