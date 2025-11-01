#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include "gui/window.h"
#include "apps/app_manager.h"

class Launcher {
public:
    Launcher();
    ~Launcher();

    bool init();
    void run();
    void cleanup();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::vector<Window*> windows;
    AppManager appManager;

    void render();
    void handleEvents();
};
