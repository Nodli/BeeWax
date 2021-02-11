#ifndef H_ECS
#define H_ECS

struct Component_Reference{
    u32 ID;
    u32 generation;
};

template<typename T>
struct Component_Storage{
    void free();

    T* create(Component_Reference& out_ref);
    T* search(const Component_Reference& ref);
    void remove(const Component_Reference& ref);
    bool is_valid(const Component_Reference& ref);

    // NOTE(hugo): use this to remove a component while iterating on storage
    void remove_by_storage_index(u32 storage_index);

    // ---- data

    struct Indexing_Info{
        u32 storage_index = 0u;
        u32 generation = 0u;
    };
    array<Indexing_Info> indexing;
    array<u32> free_ID;

    struct Storage_Info{
        u32 indexing_index = 0u;
        T data = {};
    };
    array<Storage_Info> storage;
};

#include "component_storage.inl"

#endif
