#pragma once

#include"core/application.h"
#include "core/logger.h"
#include "core/kmemory.h"
#include "game_types.h"

extern b8 create_game(game* out_game);


/*
The main entry point of the application.
*/
int main() {
    kmemory::initialize();
    
    // request game instance from application
    game game_inst = {};
    if(!create_game(&game_inst)) {
        KFATAL("Could not create game!");
        return -1;
    }

    if(!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
        KFATAL("The game's function pointers must be assigned!");
        return -2;
    }

    
    application* app = new application();
    
    // initialization
    if(!app->create(&game_inst)) {
        KINFO("Application failed to create!");
        return 1;
    }

    // Begin the game loop.
    if(!app->run()) {
        KINFO("Application did not shutdown gracefully!");
        return 2;
    }

    delete app;
    // delete game_inst;
    kmemory::shutdown();
    // KINFO("From main():")
    // KINFO(kmemory::get_memory_usage_string());
    return 0;
}