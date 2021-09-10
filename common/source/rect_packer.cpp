void Rect_Packer::create(){
    skyline_head = 0u;
    segments.create();
    segments.push({UINT32_MAX, 0u, 0u});

    // NOTE(hugo): min. capacity of array (cf. data_structure.inl : BEEWAX_INTERNAL::array_next_capacity)
    segments.resize(16u);
    free_head = 1u;
    for(u32 ifree = free_head; ifree < segments.size - 1u; ++ifree) segments[ifree].next_segment = ifree + 1u;
    segments[segments.size - 1u].next_segment = UINT32_MAX;

    area_width = 0u;
    area_height = 0u;
}

void Rect_Packer::destroy(){
    segments.destroy();
}

void Rect_Packer::set_packing_area(u32 new_width, u32 new_height){
    assert(new_width >= area_width && new_height >= area_height);

    if(new_width > area_width){
        // NOTE(hugo): find the end of the skyline
        u32 skyline_tail = skyline_head;
        while(segments[skyline_tail].next_segment != UINT32_MAX) skyline_tail = segments[skyline_tail].next_segment;

        if(segments[skyline_tail].altitude == 0u){
            // NOTE(hugo): extend the tail segment
            segments[skyline_tail].width += new_width - area_width;

        }else{
            // NOTE(hugo): allocate new segments
            if(free_head == UINT32_MAX){
                free_head = segments.size;
                segments.resize(2u * segments.capacity);

                for(u32 ifree = free_head; ifree < segments.size - 1u; ++ifree) segments[ifree].next_segment = ifree + 1u;
                segments[segments.size - 1u].next_segment = UINT32_MAX;
            }

            // NOTE(hugo): insert the new segment
            u32 new_segment = free_head;

            segments[skyline_tail].next_segment = free_head;
            free_head = segments[free_head].next_segment;

            segments[new_segment].width = new_width - area_width;
            segments[new_segment].altitude = 0u;
            segments[new_segment].next_segment = UINT32_MAX;
        }
    }

    area_width = new_width;
    area_height = new_height;
}

uivec2 Rect_Packer::insert_rect(u32 width, u32 height){
    assert(width > 0u && height > 0u);

    u32* to_insert = nullptr;

    u32 insert_x = UINT32_MAX;
    u32 insert_altitude = UINT32_MAX;
    u32 insert_cost = UINT32_MAX;

    // NOTE(hugo): search best insertion point
    u32* to_current = &skyline_head;
    u32 current_x = 0u;

    while((current_x + width) <= area_width){
        u32* to_iter = to_current;

        u32 iter_x = current_x;
        u32 iter_end = current_x + width;

        u32 iter_altitude = segments[*to_iter].altitude;

#if defined(RECT_PACKER_HEURISTIC_WITH_AREA)
        u32 iter_cost = 0u;
#endif

        // NOTE(hugo): find the iter_altitude and iter_cost for a rectangle starting at /current_x/
        while(iter_x < iter_end){
            // NOTE(hugo): cost of the current segment
            u32 occluded_width = min(iter_end - iter_x, segments[*to_iter].width);

            // NOTE(hugo): cost of increasing the segment altitude
            if(iter_altitude < segments[*to_iter].altitude){
#if defined(RECT_PACKER_HEURISTIC_WITH_AREA)
                iter_cost += (iter_x - current_x) * (segments[*to_iter].altitude - iter_altitude);
#endif
                iter_altitude = segments[*to_iter].altitude;
            }else{
#if defined(RECT_PACKER_HEURISTIC_WITH_AREA)
                iter_cost += occluded_width * (iter_altitude - segments[*to_iter].altitude);
#endif
            }

            iter_x += segments[*to_iter].width;
            to_iter = &segments[*to_iter].next_segment;
        }

        u32 new_altitude = iter_altitude + height;

#if defined(RECT_PACKER_HEURISTIC_WITH_AREA)
        // NOTE(hugo): bottom-left heuristic and then area heuristic
        if((iter_altitude < insert_altitude || (iter_altitude == insert_altitude && iter_cost < insert_cost))
        && !(new_altitude > area_height)){
#else
        // NOTE(hugo): bottom-left heuristic
        if(iter_altitude < insert_altitude
        && !(new_altitude > area_height)){
#endif
            to_insert = to_current;
            insert_x = current_x;
            insert_altitude = iter_altitude;
#if defined(RECT_PACKER_HEURISTIC_WITH_AREA)
            insert_cost = iter_cost;
#endif
        }

        current_x += segments[*to_current].width;
        to_current = &segments[*to_current].next_segment;
    }

    if(to_insert){
        // NOTE(hugo): remove occluded segments
        u32 iter = *to_insert;
        u32 iter_x = insert_x;
        u32 iter_end = insert_x + width;

        while(iter != UINT32_MAX && (iter_x + segments[iter].width) <= iter_end){
            u32 next_iter = segments[iter].next_segment;
            iter_x += segments[iter].width;

            segments[iter].next_segment = free_head;
            free_head = iter;

            iter = next_iter;
        }

        // NOTE(hugo): reduce partially occluded segment
        if(iter_x != iter_end) segments[iter].width = iter_x + segments[iter].width - iter_end;

        // NOTE(hugo): allocate new segments
        if(free_head == UINT32_MAX){
            uintptr_t to_offset = (uintptr_t)to_insert - (uintptr_t)segments.data;

            free_head = segments.size;
            segments.resize(2u * segments.capacity);

            for(u32 ifree = free_head; ifree < segments.size - 1u; ++ifree) segments[ifree].next_segment = ifree + 1u;
            segments[segments.size - 1u].next_segment = UINT32_MAX;

            if(insert_x != 0u) to_insert = (u32*)((uintptr_t)segments.data + to_offset);
        }

        // NOTE(hugo): insert new segment
        u32 new_segment = free_head;
        free_head = segments[free_head].next_segment;

        segments[new_segment].width = width;
        segments[new_segment].altitude = insert_altitude + height;
        segments[new_segment].next_segment = iter;
        *to_insert = new_segment;

        // NOTE(hugo): merge with next segment of same altitude
        u32 next_segment = segments[*to_insert].next_segment;
        if(next_segment != UINT32_MAX && segments[next_segment].altitude == segments[new_segment].altitude){
            segments[new_segment].width += segments[next_segment].width;
            segments[new_segment].next_segment = segments[next_segment].next_segment;
            segments[next_segment].next_segment = free_head;
            free_head = next_segment;
        }

        // NOTE(hugo): merge previous segment of same altitude
        Segment* previous_segment = (Segment*)to_insert;
        if(insert_x != 0u && (*previous_segment).altitude == segments[new_segment].altitude){
            assert(to_insert != &skyline_head);
            (*previous_segment).width += segments[new_segment].width;
            (*previous_segment).next_segment = segments[new_segment].next_segment;
            segments[new_segment].next_segment = free_head;
            free_head = new_segment;
        }
    }

    return {insert_x, insert_altitude};
}
