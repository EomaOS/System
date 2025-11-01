/* eoma_store.c
   EomaOS App Store: SDL2 UI + libcurl download + cJSON parse + install .oema packages
   Compile: see instructions after the file
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

/* --- CONFIG --- */
#define WIN_W 1024
#define WIN_H 600
#define CATALOG_URL "https://your-server.example.com/apps.json" // troque para seu servidor
#define DOWNLOAD_DIR "./downloads"
#define APPS_DIR "./apps"
#define ICONS_DIR "./icons"
#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" // ajuste conforme seu sistema

/* --- ESTRUTURAS --- */
typedef struct {
    char *id;
    char *name;
    char *desc;
    char *icon_url;
    char *package_url;
    char *version;
    bool installed;
} AppInfo;

typedef struct {
    char *data;
    size_t size;
} MemBuf;

/* --- AUX: cria diretório se não existir --- */
static void ensure_dir(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

/* --- CURL write callback --- */
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    MemBuf *buf = (MemBuf*)stream;
    size_t realsize = size * nmemb;
    char *ptr_new = realloc(buf->data, buf->size + realsize + 1);
    if (!ptr_new) return 0;
    buf->data = ptr_new;
    memcpy(&(buf->data[buf->size]), ptr, realsize);
    buf->size += realsize;
    buf->data[buf->size] = 0;
    return realsize;
}

/* --- Download para memória (usado para pegar catalog JSON) --- */
static bool curl_download_mem(const char *url, MemBuf *out) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    out->data = malloc(1); out->size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "EomaOS-Store/0.1");
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK && out->size > 0);
}

/* --- Download para arquivo (usado para baixar .oema ou ícone) --- */
static size_t write_file_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    FILE *f = (FILE*)stream;
    return fwrite(ptr, size, nmemb, f);
}

static bool curl_download_file(const char *url, const char *out_path) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    FILE *fp = fopen(out_path, "wb");
    if (!fp) { curl_easy_cleanup(curl); return false; }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "EomaOS-Store/0.1");
    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

/* --- Parse do catalog JSON --- */
static AppInfo* parse_catalog(const char *json_text, int *out_count) {
    cJSON *root = cJSON_Parse(json_text);
    if (!root) return NULL;
    int n = cJSON_GetArraySize(root);
    AppInfo *apps = calloc(n, sizeof(AppInfo));
    for (int i = 0; i < n; ++i) {
        cJSON *it = cJSON_GetArrayItem(root, i);
        cJSON *j_id = cJSON_GetObjectItem(it, "id");
        cJSON *j_name = cJSON_GetObjectItem(it, "name");
        cJSON *j_desc = cJSON_GetObjectItem(it, "description");
        cJSON *j_icon = cJSON_GetObjectItem(it, "icon_url");
        cJSON *j_pkg = cJSON_GetObjectItem(it, "package_url");
        cJSON *j_ver = cJSON_GetObjectItem(it, "version");
        apps[i].id = strdup(cJSON_IsString(j_id) ? j_id->valuestring : "unknown");
        apps[i].name = strdup(cJSON_IsString(j_name) ? j_name->valuestring : "NoName");
        apps[i].desc = strdup(cJSON_IsString(j_desc) ? j_desc->valuestring : "");
        apps[i].icon_url = strdup(cJSON_IsString(j_icon) ? j_icon->valuestring : "");
        apps[i].package_url = strdup(cJSON_IsString(j_pkg) ? j_pkg->valuestring : "");
        apps[i].version = strdup(cJSON_IsString(j_ver) ? j_ver->valuestring : "0.0");
        apps[i].installed = false;
    }
    cJSON_Delete(root);
    *out_count = n;
    return apps;
}

/* --- Le manifest local para saber se app está instalado --- */
static bool check_installed(const char *appid) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s/manifest.json", APPS_DIR, appid);
    struct stat st;
    return (stat(path, &st) == 0);
}

/* --- Faz install: salva .oema, unzip para apps/<id>/ --- */
static bool install_package(const char *pkg_url, const char *appid) {
    ensure_dir(DOWNLOAD_DIR);
    ensure_dir(APPS_DIR);
    char outfile[1024];
    snprintf(outfile, sizeof(outfile), "%s/%s.oema", DOWNLOAD_DIR, appid);
    if (!curl_download_file(pkg_url, outfile)) {
        printf("Erro ao baixar pacote %s\n", pkg_url);
        return false;
    }
    // cria pasta do app
    char appdir[1024];
    snprintf(appdir, sizeof(appdir), "%s/%s", APPS_DIR, appid);
    ensure_dir(appdir);
    // usa unzip system (simples). Requer 'unzip' instalado.
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "unzip -o \"%s\" -d \"%s\" >/dev/null 2>&1", outfile, appdir);
    int r = system(cmd);
    if (r != 0) {
        printf("Falha ao descompactar pacote (cmd='%s')\n", cmd);
        return false;
    }
    printf("Instalado em %s\n", appdir);
    return true;
}

/* --- Lança o app (executa app.jar com java - jar) --- */
static bool launch_app(const char *appid) {
    char appdir[1024];
    snprintf(appdir, sizeof(appdir), "%s/%s", APPS_DIR, appid);
    char jarpath[1024];
    snprintf(jarpath, sizeof(jarpath), "%s/app.jar", appdir);
    struct stat st;
    if (stat(jarpath, &st) != 0) {
        printf("app.jar não encontrado para %s\n", appid);
        return false;
    }
    // Executa em background (não bloqueante) e redireciona saída
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "java -jar \"%s\" >/tmp/eoma_app_%s.log 2>&1 &", jarpath, appid);
    int r = system(cmd);
    return (r == 0);
}

/* --- Carregar textura do disco via SDL_image seria ideal, mas mantemos SDL2_ttf - apenas um placeholder icon --- */
static SDL_Texture* load_icon_placeholder(SDL_Renderer *rend, TTF_Font *font, const char *label) {
    SDL_Color black = {0,0,0,255}, white = {255,255,255,255};
    SDL_Surface *s = TTF_RenderText_Shaded(font, label, white, black);
    SDL_Texture *t = SDL_CreateTextureFromSurface(rend, s);
    SDL_FreeSurface(s);
    return t;
}

/* --- Função main (UI/loop) --- */
int main(int argc, char *argv[]) {
    /* Init libs */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);

    ensure_dir(DOWNLOAD_DIR);
    ensure_dir(APPS_DIR);
    ensure_dir(ICONS_DIR);

    SDL_Window *win = SDL_CreateWindow("Eoma Store", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font *fontL = TTF_OpenFont(FONT_PATH, 32);
    TTF_Font *fontS = TTF_OpenFont(FONT_PATH, 16);
    if (!fontL || !fontS) {
        printf("Erro ao abrir fonte. Ajuste FONT_PATH.\n");
        return 1;
    }

    /* Baixa catalog */
    MemBuf buf = {0};
    bool ok = curl_download_mem(CATALOG_URL, &buf);
    if (!ok) {
        printf("Erro ao baixar catalogo em %s\n", CATALOG_URL);
        return 1;
    }

    int appcount = 0;
    AppInfo *apps = parse_catalog(buf.data, &appcount);
    free(buf.data);
    if (!apps) {
        printf("Erro no parse do catalog\n");
        return 1;
    }

    // marca instalados
    for (int i = 0; i < appcount; ++i) {
        apps[i].installed = check_installed(apps[i].id);
    }

    /* UI layout grid */
    int cols = 4;
    int padding = 20;
    int icon_w = 150, icon_h = 150;
    int offset_x = 40, offset_y = 80;

    bool running = true;
    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            else if (ev.type == SDL_MOUSEBUTTONDOWN) {
                int mx = ev.button.x, my = ev.button.y;
                // identificar item clicado
                for (int i = 0; i < appcount; ++i) {
                    int row = i / cols;
                    int col = i % cols;
                    int x = offset_x + col * (icon_w + padding);
                    int y = offset_y + row * (icon_h + 80);
                    SDL_Rect r = {x, y, icon_w, icon_h};
                    if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
                        // clicou ícone -> abrir detalhe / instalar / abrir
                        // Simples: se instalado -> launch, se não -> instalar
                        if (apps[i].installed) {
                            printf("Abrindo app %s\n", apps[i].id);
                            launch_app(apps[i].id);
                        } else {
                            printf("Instalando %s...\n", apps[i].id);
                            // feedback simples: baixar e unzip
                            bool res = install_package(apps[i].package_url, apps[i].id);
                            if (res) {
                                apps[i].installed = true;
                                printf("%s instalado com sucesso.\n", apps[i].id);
                            } else {
                                printf("Falha ao instalar %s\n", apps[i].id);
                            }
                        }
                    }
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(rend, 30, 30, 30, 255);
        SDL_RenderClear(rend);

        // Header
        SDL_Color white = {255,255,255,255};
        SDL_RenderDrawLine(rend, 0, 60, WIN_W, 60);
        SDL_Texture *title = NULL;
        {
            SDL_Surface *s = TTF_RenderText_Solid(fontL, "Eoma Store", white);
            title = SDL_CreateTextureFromSurface(rend, s);
            SDL_Rect tr = {20,10, s->w, s->h};
            SDL_RenderCopy(rend, title, NULL, &tr);
            SDL_FreeSurface(s);
            SDL_DestroyTexture(title);
        }

        // Grid apps
        for (int i = 0; i < appcount; ++i) {
            int row = i / cols;
            int col = i % cols;
            int x = offset_x + col * (icon_w + padding);
            int y = offset_y + row * (icon_h + 80);

            // icon placeholder
            SDL_Rect iconr = {x, y, icon_w, icon_h};
            SDL_SetRenderDrawColor(rend, 50, 50, 50, 255);
            SDL_RenderFillRect(rend, &iconr);

            // nome
            render_text: ;
            // desenha nome com fonte menor (simples)
            renderText(rend, fontS, apps[i].name, x, y + icon_h + 8, white);

            // se instalado, desenhar "open" tag
            if (apps[i].installed) {
                SDL_Rect t = {x + icon_w - 60, y + icon_h - 28, 56, 24};
                SDL_SetRenderDrawColor(rend, 0, 150, 0, 255);
                SDL_RenderFillRect(rend, &t);
                renderText(rend, fontS, "Open", x + icon_w - 52, y + icon_h - 24, (SDL_Color){255,255,255,255});
            } else {
                SDL_Rect t = {x + icon_w - 80, y + icon_h - 28, 76, 24};
                SDL_SetRenderDrawColor(rend, 50, 50, 120, 255);
                SDL_RenderFillRect(rend, &t);
                renderText(rend, fontS, "Install", x + icon_w - 72, y + icon_h - 24, (SDL_Color){255,255,255,255});
            }
        }

        SDL_RenderPresent(rend);
        SDL_Delay(16);
    }

    // limpa memoria
    for (int i = 0; i < appcount; ++i) {
        free(apps[i].id); free(apps[i].name); free(apps[i].desc);
        free(apps[i].icon_url); free(apps[i].package_url); free(apps[i].version);
    }
    free(apps);

    TTF_CloseFont(fontL);
    TTF_CloseFont(fontS);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    curl_global_cleanup();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
