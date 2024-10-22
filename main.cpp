#include "kolosal.h"

int main() {
    if (!initializeGLFW())
        return 1;

    GLFWwindow* window = createWindow();
    if (window == nullptr)
        return 1;

    if (!initializeGLAD())
        return 1;

    setupImGui(window);
    mainLoop(window);
    cleanup(window);

    return 0;
}
