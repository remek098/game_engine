#include "game.h"

#include <core/logger.h>

b8 testbed_game::initialize(game* game_inst) {
    KDEBUG("testbed_game::initialize() called");
    return true;
}

b8 testbed_game::update(game* game_inst, f32 delta_time) {
    // KDEBUG("testbed_game::update() called");
    return true;
}

b8 testbed_game::render(game* game_inst, f32 delta_time) {
    // KDEBUG("testbed_game::render() called");
    return true;
}

void testbed_game::on_resize(game* game_inst, u32 width, u32 height) {
    KDEBUG("testbed_game::on_resize() called with width:%d, height: %d ", width, height);
}