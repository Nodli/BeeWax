namespace BEEWAX_INTERNAL{
    inline u32 indexmap_devirtualize_index(u32 virtual_index){
        assert(virtual_index);
        return virtual_index - 1u;
    }
    inline u32 indexmap_virtualize_index(u32 index){
        return index + 1u;
    }
}

template<typename T>
void indexmap<T>::create(){
    inactive_head = 0u;
    map.create();
}

template<typename T>
void indexmap<T>::destroy(){
    map.destroy();
}

template<typename T>
indexmap_handle indexmap<T>::borrow_handle(){
    indexmap_handle handle;

    if(inactive_head){
        handle.virtual_index = inactive_head;

        u32 index = BEEWAX_INTERNAL::indexmap_devirtualize_index(inactive_head);
        inactive_head = map[index].inactive.next;
        handle.generation = map[index].inactive.generation;

    }else{
        u32 new_index = map.size;
        handle.virtual_index = BEEWAX_INTERNAL::indexmap_virtualize_index(map.size);
        handle.generation = indexmap_null_generation + 1u;

        mapping new_mapping;
        new_mapping.active.generation = indexmap_null_generation + 1u;
        map.push(new_mapping);
    }

    return handle;
}

template<typename T>
T* indexmap<T>::search(indexmap_handle handle){
    u32 index = BEEWAX_INTERNAL::indexmap_devirtualize_index(handle.virtual_index);
    if(handle.virtual_index && index < map.size && map[index].active.generation == handle.generation){
        return &map[index].active.type;
    }
    return nullptr;
}

template<typename T>
void indexmap<T>::return_handle(indexmap_handle handle){
    u32 index = BEEWAX_INTERNAL::indexmap_devirtualize_index(handle.virtual_index);
    if(handle.virtual_index && index < map.size && map[index].active.generation == handle.generation){
        ++map[index].inactive.generation;
        map[index].inactive.next = inactive_head;
        inactive_head = handle.virtual_index;
    }
}
