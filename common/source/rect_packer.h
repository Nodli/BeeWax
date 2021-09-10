// REF(hugo):
// http://pds25.egloos.com/pds/201504/21/98/RectangleBinPack.pdf
// https://www.cs.princeton.edu/~chazelle/pubs/blbinpacking.pdf (Potentially more efficient)

// NOTE(hugo):
// * ~ 100% faster than stb_rect_pack with the default bottom-left heuristic

//#define RECT_PACKER_HEURISTIC_WITH_AREA

struct Rect_Packer{
    void create();
    void destroy();

    void set_packing_area(u32 new_width, u32 new_height);

    // NOTE(hugo):
    // * {UINT32_MAX, UINT32_MAX} in case of failure to pack
    // * coordinates of bottom-left corner in case of success
    uivec2 insert_rect(u32 width, u32 height);

    // ----

    struct Segment{
        u32 next_segment;
        u32 width;
        u32 altitude;
    };
    u32 skyline_head;
    u32 free_head;
    array<Segment> segments;

    u32 area_width;
    u32 area_height;
};
