#ifndef H_HANDLE_TABLE
#define H_HANDLE_TABLE

struct indexmap_handle{
    u32 virtual_index;
    u32 generation;
};

constexpr u32 indexmap_null_virtual_index = 0u;
constexpr u32 indexmap_null_generation = 0u;

// ----

template<typename T>
struct indexmap{
    void create();
    void destroy();

    indexmap_handle borrow_handle();
    T* search(indexmap_handle handle);
    void return_handle(indexmap_handle handle);

    // ---- data

    union mapping{
        struct {
            u32 generation;
            T type;
        } active;
        struct {
            u32 generation;
            u32 next;
        } inactive;
    };

    // NOTE(hugo): inactive_head is a virtual index
    u32 inactive_head;
    array<mapping> map;
};

#include "indexmap.inl"

#endif
