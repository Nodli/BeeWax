#ifndef H_ASSET_CATALOG
#define H_ASSET_CATALOG

constexpr u32 asset_string_capacity = 60u;
typedef sstring<asset_string_capacity> Asset_Type_Tag;
typedef sstring<asset_string_capacity> Asset_Tag;

template<typename T>
struct Asset_Catalog{
    void create();
    void destroy();

    // NOTE(hugo): asset resources are not destroyed
    T& create_runtime_asset(const Asset_Tag& tag);
    void remove_runtime_asset(const Asset_Tag& tag);

    const T* search_asset(const Asset_Tag& tag) const;

    // ---- data

    hashmap<Asset_Tag, void*> map;
};

#include "asset_catalog.inl"

#endif
