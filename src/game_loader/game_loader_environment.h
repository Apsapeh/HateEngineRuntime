#pragma once

#include "servers/platform_driver/platform_driver.h"
#include "servers/render_context/render_context.h"
#include "servers/render_server/render_server.h"

typedef void (*SetupFn)(void);
typedef void (*ReadyFn)(void);
typedef void (*ExitFn)(void);
typedef void (*ProcessFn)(double);
typedef void (*PhysicsProcessFn)(double);
typedef void (*RenderFn)(double);

typedef void (*RuntimeInitFn)(void* (*proc_addr)(const char* name));
typedef void (*PlatformDriverInitFn)(PlatformDriverBackend* backend);
typedef void (*RenderContextInitFn)(RenderContextBackend* backend);
typedef void (*RenderServerIninFn)(RenderServerBackend* backend);

typedef struct GameLoaderEnvironment {
    SetupFn _setup;
    ReadyFn _ready;
    ExitFn _exit;
    ProcessFn _process;
    PhysicsProcessFn _physics_process;
    RenderFn _render;

    // Init functions
    RuntimeInitFn _runtime_init;
    PlatformDriverInitFn _platform_driver_init;
    RenderContextInitFn _render_context_init;
    RenderServerIninFn _render_server_init;
} GameLoaderEnvironment;

GameLoaderEnvironment load_environment(void);
