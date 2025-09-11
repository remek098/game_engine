#pragma once

#include <defines.h>
#include <game_types.h>


typedef struct game_state {
    f32 delta_time;
} game_state;


class testbed_game {
public:
    static b8 initialize(game* game_inst);
    static b8 update(game* game_inst, f32 delta_time);
    static b8 render(game* game_inst, f32 delta_time);
    static void on_resize(game* game_inst, u32 width, u32 height);
};