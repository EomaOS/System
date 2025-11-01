#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define WIN_WIDTH  1024
#define WIN_HEIGHT 600

// Estrutura de botão
typedef struct {
    SDL_Rect rect;
    const char *label;
} Button;

// Função para renderizar texto
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Verifica se o mouse clicou em um botão
bool isMouseOver(SDL_Point mouse, Button b) {
    return (mouse.x > b.rect.x && mouse.x < b.rect.x + b.rect.w &&
            mouse.y > b.rect.y && mouse.y < b.rect.y + b.rect.h);
}

// Efeito de digitação
void printTyping(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, int delay) {
    char buffer[256];
    for (int i = 0; i < strlen(text); i++) {
        buffer[i] = text[i];
        buffer[i+1] = '\0';
        renderText(renderer, font, buffer, x, y, color);
        SDL_RenderPresent(renderer);
        usleep(delay);
    }
}

// Tela de boot animada
void bootScreen(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color cyan = {0, 255, 255};
    SDL_Color white = {255, 255, 255};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    printTyping(renderer, font, "EomaOS v0.1 Booting...", 300, 250, cyan, 50000);
    SDL_Delay(500);

    printTyping(renderer, font, "Loading modules...", 300, 300, white, 50000);
    SDL_Delay(500);

    printTyping(renderer, font, "Initializing interface...", 300, 350, white, 50000);
    SDL_Delay(500);
}

// Função principal
int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("EomaOS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Fontes
    TTF_Font *fontLarge = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48);
    TTF_Font *fontSmall = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (!fontLarge || !fontSmall) {
        printf("Erro ao carregar fonte.\n");
        return 1;
    }

    SDL_Color white = {255,255,255};
    SDL_Color cyan = {0,255,255};
    SDL_Color gray = {30,30,30};

    // Botões estilo Android
    Button buttons[5] = {
        {{50, 500, 150, 50}, "Home"},
        {{220, 500, 150, 50}, "Voltar"},
        {{390, 500, 150, 50}, "Terminal"},
        {{560, 500, 150, 50}, "Configurações"},
        {{730, 500, 150, 50}, "Desligar"},
    };

    // Tela de boot
    bootScreen(renderer, fontSmall);
    SDL_Delay(500);

    bool running = true;
    SDL_Event event;
    int bgShift = 0;

    while (running) {
        SDL_Point mouse;
        SDL_GetMouseState(&mouse.x, &mouse.y);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (isMouseOver(mouse, buttons[0])) printf("[EomaOS] Home clicado\n");
                else if (isMouseOver(mouse, buttons[1])) printf("[EomaOS] Voltar clicado\n");
                else if (isMouseOver(mouse, buttons[2])) printf("[EomaOS] Terminal aberto\n");
                else if (isMouseOver(mouse, buttons[3])) printf("[EomaOS] Configurações abertas\n");
                else if (isMouseOver(mouse, buttons[4])) {
                    printf("[EomaOS] Desligando...\n");
                    running = false;
                }
            }
        }

        // Fundo animado
        for (int y = 0; y < WIN_HEIGHT; y++) {
            int r = (20 + (y + bgShift) % 60);
            int g = (40 + (y + bgShift) % 80);
            int b = (100 + (y + bgShift) % 120);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, WIN_WIDTH, y);
        }
        bgShift++;

        // Barra superior estilo Android
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect bar = {0,0,WIN_WIDTH,40};
        SDL_RenderFillRect(renderer, &bar);

        // Relógio
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timeStr[9];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", t);
        renderText(renderer, fontSmall, timeStr, WIN_WIDTH - 100, 10, white);

        // Logo
        renderText(renderer, fontLarge, "EomaOS", 400, 150, cyan);
        renderText(renderer, fontSmall, "Sistema Operacional em C", 390, 220, white);

        // Renderizar botões
        for (int i = 0; i < 5; i++) {
            if (isMouseOver(mouse, buttons[i]))
                SDL_SetRenderDrawColor(renderer, 70,70,70,255);
            else
                SDL_SetRenderDrawColor(renderer, 50,50,50,255);
            SDL_RenderFillRect(renderer, &buttons[i].rect);
            SDL_SetRenderDrawColor(renderer, 0,255,255,255);
            SDL_RenderDrawRect(renderer, &buttons[i].rect);
            renderText(renderer, fontSmall, buttons[i].label, buttons[i].rect.x + 20, buttons[i].rect.y + 15, white);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    TTF_CloseFont(fontLarge);
    TTF_CloseFont(fontSmall);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
