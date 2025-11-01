void clearScreen() {
    printf("\033[2J\033[1;1H");
}
void setColor(const char *color) {
    printf("%s", color);
}
void printCentered(const char *text, int width) {
    int len = strlen(text);
    int padding = (width - len) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", text);
}
void drawBox(int x, int y, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0 || i == height - 1) printf("-");
            else if (j == 0 || j == width - 1) printf("|");
            else printf(" ");
        }
        printf("\n");
    }
}
void runCommand(const char *cmd) {
    if (strcmp(cmd, "info") == 0) systemInfo();
    else if (strcmp(cmd, "clear") == 0) clearScreen();
    else if (strcmp(cmd, "proc") == 0) listProcesses();
    else printf("Comando desconhecido: %s\n", cmd);
}
void loadingAnimation(int timeMs) {
    const char *frames = "|/-\\";
    for (int i = 0; i < timeMs / 100; i++) {
        printf("\rCarregando... %c", frames[i % 4]);
        fflush(stdout);
        usleep(100000);
    }
    printf("\rConcluído!     \n");
}
void showLogo() {
#include <stdio.h>
#include <string.h>
#include <unistd.h>  // para usleep

// Cores ANSI
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define CYAN    "\033[1;36m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"
#define CLEAR   "\033[2J\033[1;1H"

// Função para limpar a tela
void clearScreen() {
    printf(CLEAR);
}

// Função para imprimir texto com efeito de digitação
void printTyping(const char *text, useconds_t delay) {
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        usleep(delay);
    }
}

// Função para imprimir texto centralizado
void printCentered(const char *text, int width) {
    int len = strlen(text);
    int padding = (width - len) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", text);
}

// Animação de carregamento estilo spinner
void loadingAnimation(int timeMs) {
    const char *frames = "|/-\\";
    for (int i = 0; i < timeMs / 100; i++) {
        printf("\r%sIniciando EomaOS... %c%s", CYAN, frames[i % 4], RESET);
        fflush(stdout);
        usleep(100000);
    }
    printf("\r%sEomaOS iniciado com sucesso!    %s\n", GREEN, RESET);
}

// Função principal de exibir o logo
void showLogo() {
    clearScreen();
    printf("\n");
    printCentered(CYAN "====================================" RESET, 40);
    printCentered(GREEN "        ███████╗ ██████╗ ███╗   ███╗" RESET, 40);
    printCentered(GREEN "        ██╔════╝██╔═══██╗████╗ ████║" RESET, 40);
    printCentered(GREEN "        █████╗  ██║   ██║██╔████╔██║" RESET, 40);
    printCentered(GREEN "        ██╔══╝  ██║   ██║██║╚██╔╝██║" RESET, 40);
    printCentered(GREEN "        ██║     ╚██████╔╝██║ ╚═╝ ██║" RESET, 40);
    printCentered(GREEN "        ╚═╝      ╚═════╝ ╚═╝     ╚═╝" RESET, 40);
    printCentered(CYAN "====================================" RESET, 40);
    printf("\n");
    printCentered(YELLOW "           EomaOS v0.1" RESET, 40);
    printCentered("        Sistema Operacional em C", 40);
    printf("\n\n");
    loadingAnimation(2500);
    printf("\n");
    printTyping(GREEN "Bem-vindo ao EomaOS!\n", 40000);
}

int main() {
    showLogo();
    // Aqui você pode chamar seu shell, menu, etc.
    printTyping(YELLOW "Digite 'help' para ver os comandos disponíveis.\n" RESET, 40000);
    return 0;
}
}