#ifndef H_ASSET_MANAGER
#define H_ASSET_MANAGER

constexpr u32 asset_tag_capacity = 60u;
typedef sstring<asset_tag_capacity> Asset_Tag;

struct Asset_Catalog_Bucket{
    Asset_Catalog_Bucket* prev;
    Asset_Catalog_Bucket* next;
};

template<typename T>
struct Asset_Catalog{
    void terminate();

    T* create(const Asset_Tag& tag);
    T* search(const Asset_Tag& tag);
    void remove(const Asset_Tag& tag);

    // ---- data

    Asset_Catalog_Bucket* head = nullptr;
    hashmap<Asset_Tag, Asset_Catalog_Bucket*> tag_to_bucket;
};

#endif
