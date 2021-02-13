static_assert(sizeof(Asset_Catalog_Bucket) % 8u == 0u);

static void joint_asset_bucket_malloc(size_t asset_bytesize,
        Asset_Catalog_Bucket*& bucket_ptr, void*& asset_ptr){

    void* shared_mem = malloc(sizeof(Asset_Catalog_Bucket) + asset_bytesize);
    bucket_ptr = (Asset_Catalog_Bucket*)shared_mem;
    asset_ptr = (u8*)shared_mem + sizeof(Asset_Catalog_Bucket);
}

static inline void* asset_from_bucket(Asset_Catalog_Bucket* bucket_ptr){
    return (void*)((u8*)bucket_ptr + sizeof(Asset_Catalog_Bucket));
}

template<typename T>
void Asset_Catalog<T>::terminate(){
    Asset_Catalog_Bucket* ptr = head;
    while(ptr){
        void* to_free = (void*)ptr;
        ptr = ptr->next;
        ::free(ptr);
    }

    tag_to_bucket.free();
}

template<typename T>
T* Asset_Catalog<T>::create(const Asset_Tag& tag){
    // NOTE(hugo): malloc
    Asset_Catalog_Bucket* bucket;
    void* asset;
    joint_asset_bucket_malloc(sizeof(T), bucket, asset);

    // NOTE(hugo): link
    bucket->prev = nullptr;
    bucket->next = head;
    if(head) head->prev = bucket;
    head = bucket;

    // NOTE(hugo): register
    bool no_collision;
    Asset_Catalog_Bucket** bucket_in_table = tag_to_bucket.get(tag, no_collision);
    assert(no_collision);
    *bucket_in_table = bucket;

    return (T*)asset;
}

template<typename T>
T* Asset_Catalog<T>::search(const Asset_Tag& tag){
    Asset_Catalog_Bucket** bucket_in_table = tag_to_bucket.search(tag);
    assert(bucket_in_table);

    return (T*)asset_from_bucket(*bucket_in_table);
}

template<typename T>
void Asset_Catalog<T>::remove(const Asset_Tag& tag){
    Asset_Catalog_Bucket** bucket_in_table = tag_to_bucket.search(tag);
    assert(bucket_in_table);

    if((*bucket_in_table)->prev)   (*bucket_in_table)->prev->next = (*bucket_in_table)->next;
    if((*bucket_in_table)->next)   (*bucket_in_table)->next->prev = (*bucket_in_table)->prev;

    ::free(*bucket_in_table);
}
