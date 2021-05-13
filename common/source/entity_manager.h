#ifndef H_ENTITY_MANAGER
#define H_ENTITY_MANAGER

// ----- USAGE -----
// - declare one or more archetype struct
// - define FOR_EACH_ENTITY_TYPE(FUNCTION) with all archetypes
// ex:  #define FOR_EACH_ENTITY_TYPE(FUNCTION)  \
//      FUNCTION(Entity)
// -----------------

// NOTE(hugo): archetypes are similar to a compile time collection of components

// REF(hugo):
// * Jonathan Blow's entity manager
// https://handmade.network/forums/t/1210-approaching_entity_systems#6924
// http://number-none.com/blow/blog//programming/2016/07/16/braid_code_cleanup_1.html
// * ECS with archetypes and collocated components
// https://www.youtube.com/watch?v=PmEeW9hjqrM                                  (Our Machinery)
// https://forum.unity.com/threads/memory-layout-for-ecs-components.590731/     (Unity)

// TODO(hugo): don't use virtual destructors and store a pointer to each archetype's destructor instead
// TODO(hugo): allocate in a memory pool per entity type

struct Entity_ID : indexmap_handle{};

struct Entity_Base{
    u32 type;
    u32 type_array_index;
    indexmap_handle entity_map_handle;
};

struct Entity_Manager{
    void create(){
        entity_map.create();
    }
    void destroy(){
        entity_map.destroy();
    }

    // ---- type indexing

#define ADD_TO_ENTITY_TYPE_ENUM(TYPE) CONCATENATE(_, TYPE),
    enum Entity_Type_ID : u32{
        FOR_EACH_ENTITY_TYPE(ADD_TO_ENTITY_TYPE_ENUM)
        NUMBER_OF_ENTITY_TYPES,
        ENTITY_NONE = NUMBER_OF_ENTITY_TYPES,
    };
#undef ADD_TO_ENTITY_TYPE_ENUM

    template<typename T>
    constexpr Entity_Type_ID get_Entity_Type_ID(){
        // NOTE(hugo): nullptr is required as a dummy because ADD_TO_ENUM appends a comma to each entry
        typedef Type_Indexer<FOR_EACH_ENTITY_TYPE(ADD_TO_ENUM) decltype(nullptr)> Indexer;
        constexpr size_t type_ID = Indexer::type_index<T>();
        static_assert(type_ID < NUMBER_OF_ENTITY_TYPES);
        return type_ID;
    }

    // ----

    template<typename T>
    Entity_ID create_entity(){
        // NOTE(hugo): allocation
        T* ptr = bw_malloc(sizeof(T));
        new (ptr) T();

        constexpr Entity_Type_ID type_ID = get_Entity_Type_ID<T>();
        std::vector<Entity_Base*>& type_array = type_arrays[type_ID];

        // NOTE(hugo): register as entity
        u32 entity_index = entity_array.size();
        entity_array.push_back(ptr);

        // NOTE(hugo): register as archetype
        u32 type_index = type_array.size();
        type_array.push_back(ptr);

        // NOTE(hugo): borrow handle
        indexmap_handle handle = entity_map.borrow_handle();
        (*entity_map.search(handle)) = entity_index;

        // NOTE(hugo): initialize Entity_Base members
        (*ptr).type = type_ID;
        (*ptr).type_array_index = type_index;
        (*ptr).entity_map_handle = handle;

        return (Entity_ID)handle;
    }

    template<typename T>
    T* get_entity(Entity_ID handle){
        constexpr Entity_Type_ID type_ID = get_Entity_Type_ID<T>();

        u32* index_Entity_Base = entity_map.search(handle);
        if(index_Entity_Base){
            Entity_Base* ptr = entity_array[*index_Entity_Base];
            assert((*ptr).type == get_Entity_Type_ID<T>());
            return (T*)ptr;
        }

        return nullptr;
    }

    void destroy_Entity(Entity_ID handle){
        u32* entity_index_ptr = entity_map.search((indexmap_handle)handle);
        if(entity_index_ptr){
            // NOTE(hugo): return handle
            entity_map.return_handle((indexmap_handle)handle);

            // NOTE(hugo): remove as entity
            u32 entity_index = *entity_index_ptr;
            {
                if(entity_array.size() > 1u){
                    Entity_Base* swap = entity_array[entity_array.size() - 1u];

                    entity_array[entity_index] = swap;
                    *entity_map.search((*swap).entity_map_handle) = entity_index;
                }

                entity_array.pop_back();
            }

            Entity_Base* ptr = entity_array[entity_index];

            // NOTE(hugo): remove as archetype
            u32 type_index = (*ptr).type_array_index;
            {
                std::vector<Entity_Base*>& type_array = type_arrays[(*ptr).type];

                if(type_array.size() > 1u){
                    Entity_Base* swap = type_array[type_array.size() - 1u];

                    type_array[type_index] = swap;
                    (*swap).type_array_index = type_index;
                }

                type_array.pop_back();
            }

            // NOTE(hugo): deallocattion
            (*ptr).~Entity_Base();
            bw_free(ptr);
        }
    }

    // ----

    // NOTE(hugo): maps an indexmap_handle to an index in entity_array
    indexmap<u32> entity_map;

    array<Entity_Base*> entity_array;
    array<Entity_Base*> type_arrays[NUMBER_OF_ENTITY_TYPES];
};

#endif

#endif
