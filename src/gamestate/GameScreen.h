#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <lua.hpp>
#include "lanarts_defines.h"
#include "GameView.h"

#include "interface/GameChat.h"
#include "interface/GameHud.h"

class GameInst;
class GameMapState;

struct PlayerDataEntry;

struct GameScreen {
    GameHud hud;
    GameView view;
    BBox window_region;
    int focus_player_id;
};

// Represents all active screens being drawn
// Multiple screens are used in split screen, while no screens are used
// when lanarts is being run headlessly.
struct GameScreenSet {
    void add(GameScreen screen) {
        screens.push_back(screen);
    }

    GameScreen& screen();
    GameView& view();
    PlayerInst* focus_object(GameState* gs);
    PlayerDataEntry& local_player_data(GameState* gs);
    GameMapState* map(GameState* gs);
    GameHud& hud();
    BBox& window_region();

    void clear() {
        screens.clear();
    }
    template <typename Func>
    void for_each_screen(Func&& f, bool allow_nesting = false) {
        // Do not allow nesting of for_each_screen;
        LANARTS_ASSERT(allow_nesting || current_screen == -1);
	int prev_screen = current_screen;
        for (current_screen = 0; current_screen < screens.size(); current_screen++) {
            f();
        }
        current_screen = prev_screen;
    };
private:
    int current_screen = -1;
    std::vector<GameScreen> screens;
};

#endif //GAMESCREEN_H
