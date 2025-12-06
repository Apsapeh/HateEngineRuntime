#include "platform_driver.h"
#include "error.h"
#include "log.h"
#include <types/vector.h>
#include "types/types.h"

/* ==> Backends <== */
#include "sdl3/platform_driver_sdl3.h"
/* ================ */

PlatformDriverBackend PlatformDriver;

struct PlatformDriverBackendPair {
    const char* name;
    PlatformDriverBackend* backend;
};


// clang-format off
vector_template_def_static(PlatformDriverBackendPair, struct PlatformDriverBackendPair)
vector_template_impl_static(PlatformDriverBackendPair, struct PlatformDriverBackendPair)

static vec_PlatformDriverBackendPair g_registredBackends;
static boolean g_isLoaded = false;
// clang-format on

void platform_driver_init(void) {
    g_registredBackends = vec_PlatformDriverBackendPair_init();

    // Register default backend
    platform_driver_sdl3_backend_register();
}

void platform_driver_exit(void) {
    for (usize i = 0; i < g_registredBackends.size; i++)
        platform_driver_backend_free(g_registredBackends.data[i].backend);

    vec_PlatformDriverBackendPair_free(&g_registredBackends);
}

boolean platform_driver_register_backend(const char* name, PlatformDriverBackend* backend) {
    ERROR_ARGS_CHECK_2(name, backend, { return false; });

    for (usize i = 0; i < g_registredBackends.size; i++) {
        if (strcmp(g_registredBackends.data[i].name, name) == 0) {
            LOG_ERROR("Backend with name '%s' already registered", name);
            set_error(ERROR_ALREADY_EXISTS);
            return false;
        }
    }

    vec_PlatformDriverBackendPair_push_back(
            &g_registredBackends, (struct PlatformDriverBackendPair) {name, backend}
    );

    return true;
}

boolean platform_driver_load_backend(const char* name) {
    ERROR_ARGS_CHECK_1(name, { return false; });

    if (g_isLoaded) {
        LOG_ERROR("(platform_driver_load_backend) Window server already loaded");
        set_error(ERROR_INVALID_STATE);
        return false;
    }

    for (usize i = 0; i < g_registredBackends.size; i++) {
        if (strcmp(g_registredBackends.data[i].name, name) == 0) {
            PlatformDriver = *g_registredBackends.data[i].backend;
            g_isLoaded = true;
            return true;
        }
    }

    set_error(ERROR_NOT_FOUND);
    return false;
}

boolean platform_driver_is_loaded(void) {
    return g_isLoaded;
}


/* ====================> PlatformDriverBackend functions <==================== */
static boolean backend_set_get(
        PlatformDriverBackend* backend, const char* name, void (**fn)(void), unsigned char is_set
);

PlatformDriverBackend* platform_driver_backend_new(void) {
    // FIXME: Add tmalloc check
    PlatformDriverBackend* backend = tmalloc(sizeof(PlatformDriverBackend));

    // TODO: Add default functions

    return backend;
}

boolean platform_driver_backend_free(PlatformDriverBackend* backend) {
    ERROR_ARG_CHECK(backend, { return false; });
    tfree(backend);
    return true;
}

boolean platform_driver_backend_set_function(
        PlatformDriverBackend* backend, const char* name, fptr function
) {
    ERROR_ARGS_CHECK_3(backend, name, function, { return false; });
    return backend_set_get(backend, name, (void (**)(void)) function, 1);
}

fptr platform_driver_backend_get_function(PlatformDriverBackend* backend, const char* name) {
    ERROR_ARGS_CHECK_2(backend, name, { return false; });
    fptr function = NULL;
    backend_set_get(backend, name, &function, 0);
    return function;
}


static boolean backend_set_get(
        PlatformDriverBackend* backend, const char* name, void (**fn)(void), unsigned char is_set
) {
#include "setget-pairs-table.h.gen"

    for (usize i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
        if (!strcmp(name, pairs[i].name)) {
            if (is_set)
                *pairs[i].function = (void (*)(void)) fn;
            else
                *fn = *pairs[i].function;
            return true;
        }
    }

    LOG_ERROR("Unknown function name: %s", name);
    set_error(ERROR_NOT_FOUND);
    return false;
}
