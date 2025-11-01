#include "launcher.h"
#include <iostream>

Launcher::Launcher() : window(nullptr), renderer(nullptr), font(nullptr) {}

Launcher::~Launcher() {}

bool Launcher::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("EomaOS Launcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 600, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
    if (!font) return false;

    // Carrega apps
    appManager.loadApps();

    return true;
}

void Launcher::run() {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            // Aqui você pode propagar eventos para janelas, botões e apps
        }

        render();
        SDL_Delay(16);
    }
}

void Launcher::render() {
    SDL_SetRenderDrawColor(renderer, 30,30,30,255);
    SDL_RenderClear(renderer);

    // Renderiza apps
    appManager.render(renderer, font);

    SDL_RenderPresent(renderer);
}

void Launcher::cleanup() {
    for (auto win : windows) delete win;
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
