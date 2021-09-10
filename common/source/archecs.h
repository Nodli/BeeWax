// REF(hugo):
// **** Entity manager with base type
// * Jonathan Blow's - Braid & The Witness & Sokoban
// https://handmade.network/forums/t/1210-approaching_entity_systems#6924
// http://number-none.com/blow/blog//programming/2016/07/16/braid_code_cleanup_1.html
// **** ECS with archetypes and collocated components
// * Our Machinery
// https://www.youtube.com/watch?v=PmEeW9hjqrM
// https://ourmachinery.com/post/ecs-and-rendering/
// https://ourmachinery.com/post/making-the-move-rotate-scale-gizmos-work-with-any-component/
// * Unity DOTS
// https://forum.unity.com/threads/memory-layout-for-ecs-components.590731/
// https://mzaks.medium.com/how-to-not-loop-over-all-entities-in-unity-ecs-f17ecb50d054

namespace archecs{
    constexpr size_t chunk_bytesize = KILOBYTES(16u);
    constexpr u32 max_ntypes = 15u;
    constexpr u32 infinite_entities_per_chunk = UINT32_MAX;

    struct Type_Metadata{
        u16 bytesize;
        u16 alignment;
        void (*create)(void* context);
        void (*destroy)(void* context);
    };

    struct Entity{
        u32 archetype_index;
        u32 index;
    };

    struct Chunk{
        u8 data[chunk_bytesize];
    };

    struct Archetype_Storage{
        u32 nentities;
        u32 free;
        array<Chunk*> chunks;
    };

    struct Archetype{
        u16 ntypes;
        u16 type_IDs[max_ntypes];
        u16 type_offsets[max_ntypes];
        u32 entities_per_chunk;
    };

    struct Manager{
        array<Archetype> archetypes;
        array<Archetype_Storage> storage;
        Archetype* free_archetype_head;
        Chunk* free_chunk_head;
    };

    struct System_Param{
        Chunk* chunk;
        u32 nentities;
        u16 type_offsets[max_ntypes];
    };

    struct System{
        u16 ntypes;
        u16 type_IDs[max_ntypes];
        void* data;
        void (*update)(void* data, const System_Param& param);
    };

    // NOTE(hugo): returns the index of type_ID in the archetype
    s32 add_type(const Manager& man, const Type_Metadata* type_metadata, Archetype& archetype, u32 type_ID);
    // NOTE(hugo): returns the index of type_ID in the archetype
    s32 remove_type(const Manager& man, const Type_Metadata* type_metadata, Archetype& archetype, u32 type_ID);
    void update_archetype_metadata(const Manager& manager, const Type_Metadata* type_metadata, Archetype& archetype);

    s32 search_type(const Archetype& archetype, u32 type_ID);

    u32 allocate_archetype(Manager& man);
    void deallocate_archetype(Manager& man, u32 arch_index);

    // NOTE(hugo): returns the index of the entity
    u32 allocate_entity_storage(Manager& man, const Archetype& arch, Archetype_Storage& storage);
    // NOTE(hugo): returns the index of the entity that got moved to /index/ ; or /index/ if no entity was moved
    u32 deallocate_entity_storage(Manager& man, const Type_Metadata* metadata, const Archetype& arch, Archetype_Storage& storage, u32 index);

    Entity allocate_entity(Manager& manager, const Archetype& arch);
    // NOTE(hugo): returns the index of the entity that got moved to /index/ ; or /index/ if no entity was moved
    u32 deallocate_entity(Manager& manager, const Type_Metadata* metadata, Entity entity);

    Entity attach_type(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID);
    Entity detach_type(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID);
    void* type_memory(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID);

    void create_manager(Manager& manager);
    void destroy_manager(Manager& manager);

    u32 system_assess_archetype(const System& sys, const Archetype& arch, System_Param& param);
    void execute_system(Manager& man, const System& sys);

    // ---- template wrapper

    template<typename T>
    constexpr Type_Metadata metadataof();

    struct Entity_Handle : indexmap_handle {};

    template<typename ... Types>
    struct Entity_Manager{
        static constexpr u32 type_index_Entity_Handle = 0u;

        void create();
        void destroy();

        // --

        template<typename ... Archetype_Types>
        Archetype create_archetype();

        Entity_Handle create_entity(const Archetype& archetype);
        void destroy_entity(const Entity_Handle entity_handle);

        template<typename T>
        void attach_data(const Entity_Handle entity_handle);
        template<typename T>
        void detach_data(const Entity_Handle entity_handle);
        template<typename T>
        T* get_data(const Entity_Handle entity_handle);

        bool available_entity(const Entity_Handle entity_handle);
        template<typename T>
        bool available_data(const Entity_Handle entity_handle);

        // NOTE(hugo): system.data and system.update must be modified after creation
        template<typename ... System_Types>
        System create_system();

        void execute_system(const System& system);

        // ----

        indexmap<Entity> entity_map;

        Type_Metadata type_metadata[1u + sizeof...(Types)];
        Manager ecs_manager;
    };
}

void archecs_unit_test();

#include "archecs.inl"
