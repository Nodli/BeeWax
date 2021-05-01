bool operator==(const Component_Reference& refA, const Component_Reference& refB){
    return refA.ID == refB.ID && refA.generation == refB.generation;
}

template<typename T>
void Component_Storage<T>::free(){
    indexing.free();
    free_ID.free();
    storage.free();
}

template<typename T>
T* Component_Storage<T>::create(Component_Reference& out_ref){
    // NOTE(hugo): acquire ID
    u32 ID;
    if(free_ID.size){
        ID = free_ID[free_ID.size - 1u];
        free_ID.pop();
    }else{
        ID = indexing.size;
        indexing.push_empty();
    }

    // NOTE(hugo): acquire storage
    u32 storage_index = storage.size;
    storage.push_empty();

    indexing[ID].storage_index = storage_index;
    storage[storage_index].indexing_index = ID;

    out_ref = {ID, indexing[ID].generation};
    return &storage[storage_index].data;
}

template<typename T>
T* Component_Storage<T>::search(const Component_Reference& ref){
    if(is_valid(ref)) return &storage[indexing[ref.ID].storage_index].data;
    return nullptr;
}

template<typename T>
void Component_Storage<T>::remove(const Component_Reference& ref){
    if(is_valid(ref)){
        // NOTE(hugo): free indexing info
        ++indexing[ref.ID].generation;
        free_ID.push(ref.ID);

        // NOTE(hugo): free storage & repack
        u32 storage_index = indexing[ref.ID].storage_index;
        indexing[storage[storage_index].indexing_index].storage_index = storage_index;
        storage.remove_swap(storage_index);
    }
}

template<typename T>
void Component_Storage<T>::remove_by_storage_index(u32 storage_index){
    assert(storage_index < storage.size);

    // NOTE(hugo): retrieve and free indexing info
    u32 indexing_index = storage[storage_index].indexing_index;
    ++indexing[indexing_index].generation;
    free_ID.push(indexing_index);

    // NOTE(hugo): free storage & repack
    indexing[storage[storage_index].indexing_index].storage_index = storage_index;
    storage.remove_swap(storage_index);
}

template<typename T>
Component_Reference Component_Storage<T>::reference_from_storage_index(u32 storage_index){
    assert(storage_index < storage.size);

    u32 indexing_index = storage[storage_index].indexing_index;
    return {indexing_index, indexing[indexing_index].generation};
}

template<typename T>
bool Component_Storage<T>::is_valid(const Component_Reference& ref){
    return (ref.ID < indexing.size) && (ref.generation == indexing[ref.ID].generation);
}
