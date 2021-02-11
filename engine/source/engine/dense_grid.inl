template<typename T>
void Dense_Grid<T>::set_dimensions(u32 isize_x, u32 isize_y){
    data = (T*)malloc(isize_x * isize_y * sizeof(T));

    for(u32 iT = 0u; iT != isize_x * isize_y; ++iT){
        new((void*)&data[iT]) T{};
    }

    assert(data);
    size_x = isize_x;
    size_y = isize_y;
}

template<typename T>
void Dense_Grid<T>::extend_to_fit(ivec2 coord){
    ivec2 data_coord = coord - origin;

    if(data_coord.x < 0 || data_coord.x >= size_x
    || data_coord.y < 0 || data_coord.y >= size_y){

        ivec2 new_origin = {min(origin.x, coord.x), min(origin.y, coord.y)};

        // NOTE(hugo):
        // - if data_coord <  0    then extend towards the negatives to size - data_coord
        // - if data_coord >= size then extend towards the positives to data_coord
        u32 new_size_x = (u32)max(max((s32)size_x, (s32)size_x - data_coord.x), data_coord.x + 1);
        u32 new_size_y = (u32)max(max((s32)size_y, (s32)size_y - data_coord.y), data_coord.y + 1);

        T* new_data = (T*)malloc(new_size_x * new_size_y * sizeof(T));
        assert(new_data);

        ivec2 dorigin = origin - new_origin;
        // NOTE(hugo): bottom
        for(u32 iy = 0u; iy != dorigin.y; ++iy){
            for(u32 ix = 0u; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }
        // NOTE(hugo): middle-left & middle-right
        for(u32 iy = dorigin.y; iy != size_y + dorigin.y; ++iy){
            for(u32 ix = 0u; ix != dorigin.x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
            for(u32 ix = dorigin.x + size_x; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }
        // NOTE(hugo): top
        for(u32 iy = dorigin.y + size_y; iy != new_size_y; ++iy){
            for(u32 ix = 0u; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }

        for(u32 irow = 0u; irow != size_y; ++irow){
            u32 src_offset = irow * size_x;
            u32 dst_offset = (origin.y + irow - new_origin.y) * new_size_x
                + (origin.x - new_origin.x);

            memcpy(new_data + dst_offset,
                    data + src_offset,
                    size_x * sizeof(T));
        }
        ::free(data);

        data = new_data;
        size_x = new_size_x;
        size_y = new_size_y;
        origin = new_origin;
    }
}

template<typename T>
T& Dense_Grid<T>::at(s32 coord_x, s32 coord_y){
    s32 data_x = (s32)coord_x - (s32)origin.x;
    s32 data_y = (s32)coord_y - (s32)origin.y;

    assert(data_x >= 0u && data_x < size_x
    && data_y >= 0u && data_y < size_y);

    return data[data_y * size_x + data_x];
}

template<typename T>
void Dense_Grid<T>::free(){
    ::free(data);
}
