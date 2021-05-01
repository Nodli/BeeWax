namespace bw{
    #include "tracked_memory.cpp"
    #include "logprint.cpp"
    #include "error_handling.cpp"

    #include "type.cpp"
    #include "intrinsics.cpp"
    #include "utils.cpp"
    #include "hash.cpp"
    #include "vmemory.cpp"

    #include "filepath.cpp"
    #include "file.cpp"

    #include "developper_tools.cpp"

    #include "time_SDL.cpp"
    #include "frame_timing.cpp"

    #include "vec.cpp"
    #include "color.cpp"

    #include "random_custom.cpp"
    #include "noise.cpp"

    #include "geometry.cpp"

    #include "tween.cpp"

    #include "camera_math.cpp"
    #include "camera_2D.cpp"
    #include "camera_3D.cpp"

    // --

    #include "input_SDL.cpp"
    #include "cursor_SDL.cpp"

    #include "audio_SDL.cpp"

    #if defined(RENDERER_OPENGL3)
        #include "GL.cpp"

        #include "render_layer_GL3.cpp"

        #include "window_SDL_GL.cpp"
    #endif

    // ---- additional structures

    #include "texture.cpp"

    #include "scene_manager.cpp"

    #include "asset_import.cpp"
}
