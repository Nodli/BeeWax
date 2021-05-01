template<typename T>
void Dense_Grid<T>::create(){
    data = nullptr;
    size_x = 0u;
    size_y = 0u;
    origin_x = 0u;
    origin_y = 0u;
}

template<typename T>
void Dense_Grid<T>::destroy(){
    bw_free(data);
}

template<typename T>
void Dense_Grid<T>::set_dimensions(u32 isize_x, u32 isize_y){
    data = (T*)bw_malloc(isize_x * isize_y * sizeof(T));

    for(u32 iT = 0u; iT != isize_x * isize_y; ++iT){
        new((void*)&data[iT]) T{};
    }

    assert(data);
    size_x = isize_x;
    size_y = isize_y;
}

template<typename T>
void Dense_Grid<T>::extend_to_fit(s32 coord_x, s32 coord_y){
    s32 data_coord_x = coord_x - origin_x;
    s32 data_coord_y = coord_y - origin_y;

    if(data_coord_x < 0 || data_coord_x >= size_x
    || data_coord_y < 0 || data_coord_y >= size_y){

        s32 new_origin_x = min(origin_x, coord_x);
        s32 new_origin_y = min(origin_y, coord_y);

        // NOTE(hugo):
        // - if data_coord <  0    then extend towards the negatives to size - data_coord
        // - if data_coord >= size then extend towards the positives to data_coord
        u32 new_size_x = (u32)max(max((s32)size_x, (s32)size_x - data_coord_x), data_coord_x + 1);
        u32 new_size_y = (u32)max(max((s32)size_y, (s32)size_y - data_coord_y), data_coord_y + 1);

        T* new_data = (T*)bw_malloc(new_size_x * new_size_y * sizeof(T));
        assert(new_data);

        s32 dorigin_x = origin_x - new_origin_x;
        s32 dorigin_y = origin_y - new_origin_y;

        // NOTE(hugo): bottom
        for(u32 iy = 0u; iy != dorigin_y; ++iy){
            for(u32 ix = 0u; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }
        // NOTE(hugo): middle-left & middle-right
        for(u32 iy = dorigin_y; iy != size_y + dorigin_y; ++iy){
            for(u32 ix = 0u; ix != dorigin_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
            for(u32 ix = dorigin_x + size_x; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }
        // NOTE(hugo): top
        for(u32 iy = dorigin_y + size_y; iy != new_size_y; ++iy){
            for(u32 ix = 0u; ix != new_size_x; ++ix){
                new((void*)&new_data[iy * new_size_x + ix]) T{};
            }
        }

        for(u32 irow = 0u; irow != size_y; ++irow){
            u32 src_offset = irow * size_x;
            u32 dst_offset = (origin_y + irow - new_origin_y) * new_size_x
                + (origin_x - new_origin_x);

            memcpy(new_data + dst_offset,
                    data + src_offset,
                    size_x * sizeof(T));
        }
        bw_free(data);

        data = new_data;
        size_x = new_size_x;
        size_y = new_size_y;
        origin_x = new_origin_x;
        origin_y = new_origin_y;
    }
}

template<typename T>
T& Dense_Grid<T>::at(s32 coord_x, s32 coord_y){
    s32 data_x = (s32)coord_x - (s32)origin_x;
    s32 data_y = (s32)coord_y - (s32)origin_y;

    assert(data_x >= 0u && data_x < size_x
    && data_y >= 0u && data_y < size_y);

    return data[data_y * size_x + data_x];
}
