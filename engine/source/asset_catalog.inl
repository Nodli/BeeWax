template<typename T>
void Asset_Catalog<T>::create(){
    map.create();
}

template<typename T>
void Asset_Catalog<T>::destroy(){
    map.destroy();
}

template<typename T>
T& Asset_Catalog<T>::create_runtime_asset(const Asset_Tag& tag){
    void** map_ptr;
    map.get(tag, map_ptr);
    assert(map_ptr);

    void* memory = bw_malloc(sizeof(T));
    assert(memory);
    *map_ptr = memory;

    return *(T*)memory;
}

template<typename T>
void Asset_Catalog<T>::remove_runtime_asset(const Asset_Tag& tag){
    auto destroy_T = [](T& t){
        bw_free(&t);
    };
    map.remove_func(tag, destroy_T);
}

template<typename T>
const T* Asset_Catalog<T>::search_asset(const Asset_Tag& tag) const{
    void** map_ptr;
    map.search(tag, map_ptr);
    assert(map_ptr);

    return (T*)*map_ptr;
}

