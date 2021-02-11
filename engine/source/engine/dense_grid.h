#ifndef H_DENSE_GRID
#define H_DENSE_GRID

// NOTE(hugo): represents a row major grid with the origin at the bottom left
template<typename T>
struct Dense_Grid{
    void set_dimensions(u32 isize_x, u32 isize_y);
    void extend_to_fit(ivec2 coord);

    T& at(s32 coord_x, s32 coord_y);

    void free();

    // ---- data

    T* data = nullptr;
    u32 size_x = 0u;
    u32 size_y = 0u;
    ivec2 origin = {};
};

#include "dense_grid.inl"

#endif
