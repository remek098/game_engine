#pragma once
#include "core/application.h"

// struct application_config;


typedef struct game {
    // application config
    application_config app_config;

    // function pointer to game's initialize function.
    b8 (*initialize)(struct game* game_inst);

    // function pointer to game's update function
    b8 (*update)(struct game* game_inst, f32 delta_time);

    // function pointer to game's render function
    b8 (*render)(struct game* game_inst, f32 delta_time);

    // function pointer to handle resizes
    void (*on_resize)(struct game* game_inst, u32 width, u32 height);

    // game-specific game state. Created and managed by the game.
    void* state;

    // game() : app_config{}, initialize(nullptr), update(nullptr),
    //          render(nullptr), on_resize(nullptr), state(nullptr) {}
} game;