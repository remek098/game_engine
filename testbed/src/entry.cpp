#include "game.h"

#include <entry.h>

// TODO: Remove this once we have a better way of allocating
// #include <platform/platform.h>
#include <core/kmemory.h>

b8 create_game(game* out_game) {
    
    out_game->app_config.start_pos_x = 100;
    out_game->app_config.start_pos_y = 100;
    out_game->app_config.start_width = 1280;
    out_game->app_config.start_height = 720;
    out_game->app_config.name = "game engine";
    out_game->update = testbed_game::update;
    out_game->render = testbed_game::render;
    out_game->initialize = testbed_game::initialize;
    out_game->on_resize = testbed_game::on_resize;
    
    // create game state
    out_game->state = kmemory::allocate(sizeof(game_state), MEMORY_TAG_GAME);
    
    return true;
}