uint index2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size){
    return minor_axis_coord * major_axis_size + major_axis_coord;
}

void coord2D(uint index, uint major_axis_size, uint& major_axis_coord, uint& minor_axis_coord){
    major_axis_coord = index % major_axis_size;
    minor_axis_coord = index / major_axis_size;
}

uint neigh4_index2D(uint index, uint major_axis_size, uint max_index, uint* out_neighbor){
    assert(major_axis_size > 0);

    uint ncount = 0;
    if(index > major_axis_size - 1){
        *(out_neighbor++) = index - major_axis_size;
        ++ncount;
    }
    if(index + major_axis_size < max_index){
        *(out_neighbor++) = index + major_axis_size;
        ++ncount;
    }
    uint major_coord_temp = index % major_axis_size;
    if(major_coord_temp < major_axis_size - 1){
        *(out_neighbor++) = index + 1;
        ++ncount;
    }
    if(major_coord_temp > 0){ // NOTE(hugo): implies index > 0
        *(out_neighbor++) = index - 1;
        ++ncount;
    }
    return ncount;
}

uint neigh4_coord2D(uint major_axis_coord, uint minor_axis_coord, uint major_axis_size, uint minor_axis_size, uint* out_neighbor){
    uint ncount = 0;
    if(major_axis_coord + 1 < major_axis_size){
        *(out_neighbor++) = major_axis_coord + 1;
        *(out_neighbor++) = minor_axis_coord;
        ++ncount;
    }
    if(major_axis_coord > 0){
        *(out_neighbor++) = major_axis_coord - 1;
        *(out_neighbor++) = minor_axis_coord;
        ++ncount;
    }
    if(minor_axis_coord + 1 < minor_axis_size){
        *(out_neighbor++) = major_axis_coord;
        *(out_neighbor++) = minor_axis_coord + 1;
        ++ncount;
    }
    if(minor_axis_coord > 0){
        *(out_neighbor++) = major_axis_coord;
        *(out_neighbor++) = minor_axis_coord - 1;
        ++ncount;
    }
    return ncount;
}
