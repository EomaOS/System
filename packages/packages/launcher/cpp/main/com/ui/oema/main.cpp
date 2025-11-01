#include "launcher.h"

int main(int argc, char* argv[]) {
    Launcher launcher;
    if (!launcher.init()) {
        return 1;
    }
    launcher.run();
    launcher.cleanup();
    return 0;
}
