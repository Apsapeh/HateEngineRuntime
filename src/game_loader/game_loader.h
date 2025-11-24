#pragma once

#include "game_loader_environment.h"

typedef struct GameFunctions {
    ReadyFn _ready;
    ProcessFn _process;
    PhysicsProcessFn _physics_process;
    RenderFn _render;
} GameFunctions;

GameFunctions load_game(void);
