#if defined(APPLICATION_SETTINGS)
    #include APPLICATION_SETTINGS
#endif

#include "unity_header.h"
#include "unity_source.cpp"

// ---- post-engine include

#if defined(APPLICATION_UNITY)
    #include APPLICATION_UNITY

#else

using namespace bw;

int main(int argc, char* argv[]){
    Engine_Config configuration;
    configuration.window_name = "Engine";
    configuration.window_width = 800u;
    configuration.window_height = 600u;
    configuration.render_target_samples = 1u;
    Engine engine;

    printf("-- starting\n");

    engine.create(configuration);

    printf("-- mainloop\n");

    engine.run();

    printf("-- engine destroy \n");

    engine.destroy();

    printf("-- memory leak detection\n");

    DEV_Memtracker_Leakcheck();

    printf("-- finished\n");

    return 0;
}

#endif
