#include "core/Engine.h"

int main()
{
    core::Engine engine;
    engine.init();

    engine.run();
    engine.cleanup();
    
    return 0;
}
