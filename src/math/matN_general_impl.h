// #define MATDEV
#ifdef MATDEV
    #include <math/mat3.h>
    #define MAT_TYPE Mat3
    #define MAT_FN_NAME mat3
    #define MAT_SIZE 3
#endif


#include <types/types.h>
#include <string.h>
#include <error.h>

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define MFN(mfn_name) CONCAT(CONCAT(MAT_FN_NAME, _), mfn_name)

boolean MFN(from_array)(const float* const array, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_2(array, out, { return false; });
    memcpy(out, array, sizeof(MAT_TYPE));
    return true;
}

boolean MFN(clone)(const MAT_TYPE* const a, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    memcpy(out, a, sizeof(MAT_TYPE));
    return true;
}

boolean MFN(add)(const MAT_TYPE* const a, const MAT_TYPE* const b, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_3(a, b, out, { return false; });
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            out->m[i][j] = a->m[i][j] + b->m[i][j];
        }
    }
    return true;
}

boolean MFN(sub)(const MAT_TYPE* const a, const MAT_TYPE* const b, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_3(a, b, out, { return false; });
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            out->m[i][j] = a->m[i][j] - b->m[i][j];
        }
    }
    return true;
}

boolean MFN(mul)(const MAT_TYPE* const a, const MAT_TYPE* const b, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_3(a, b, out, { return false; });
    for (usize i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        ((float*) out->m)[i] = 0.0f;
    }
    for (usize i = 0; i < MAT_SIZE; i++) {
        for (usize k = 0; k < MAT_SIZE; k++) {
            for (usize j = 0; j < MAT_SIZE; j++) {
                out->m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
    return true;
}

// MAT_TYPE mat3_mul_vec

boolean MFN(factor)(const MAT_TYPE* const a, const float factor, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            out->m[i][j] = a->m[i][j] * factor;
        }
    }
    return true;
}

boolean MFN(transpose)(const MAT_TYPE* const a, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });
    for (int row = 0; row < MAT_SIZE; row++) {
        for (int col = 0; col < MAT_SIZE; col++) {
            out->m[row][col] = a->m[col][row];
        }
    }
    return true;
}

#define M_TO_S_IMPL(m) #m
#define M_TO_S(m) M_TO_S_IMPL(m)

boolean MFN(inverse)(const MAT_TYPE* const a, MAT_TYPE* const out) {
    ERROR_ARGS_CHECK_2(a, out, { return false; });

    // TOOD: Need a determinant function

    set_error(ERROR_NOT_IMPLEMENTED);
    LOG_ERROR_OR_DEBUG_FATAL("'%s' is not implemented", M_TO_S(MFN(inverse)));
    return false;
}


// booleaan MFN(rotate)

// boolean MFN(translat)

// TODO: Add tests
