#include "Engine.h"

int main(int argc, char* argv[]) {
    Engine engine;
    
    if (!engine.Initialize(1080, 720, "Forge Engine V0.1")) {
        SDL_Log("Failed to initialize engine\n");
        return -1;
    }
    
    engine.Run();
    engine.Shutdown();
    
    return 0;
}