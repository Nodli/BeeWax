struct Entity_ID : indexmap_handle {};

struct Component{
    virtual void create() = 0;
    virtual void destroy() = 0;
};

// ----

template<typename Type_Registry>
struct Entity_Manager{
    struct Archetype{
        u32 ntypes;
        u32 type[64u];
    };

    struct Entity_Data{
        Archetype archetype;
        indexmap_handle handle;
        Component* data[64u];
    };

    constexpr Type_Registry type_registry;
    template<typename T>
    u32 type_ID(){
        constexpr size_t ID = type_registry::type_index<T>();
        static_assert(ID < type_registry::type_count());
        return ID;
    }

    // ----

    Entity_ID create_entity(){
        Entity_Data* entity = bw_malloc(sizeof(Entity_Data));
        memset(entity, 0x00, sizeof(Entity_Data));

        u32 entity_index = entities.size();
        entities.push_back(entity);

        indexmap_handle handle = entity_map.borrow_handle();
        *entity_map.search(handle) = entity_index;

        *entity.handle = handle;

        return (Entity_ID)handle;
    }

    void destroy_entity(Entity_ID entity_ID){
        u32 entity_index = entity_map.search(entity_ID);
        entity_map.return_handle(entity_D);

        Entity_Data* entity = entities[entity_index];
        if(entities.size()){
            Entity_Data* swap = entities[entities.size() - 1u];
            entities[entity_index] = swap;
            *entity_map.search(*swap.handle) = entity_index;
        }

        for(u32 icomp = 0u; icomp != *entity.archetype.ntypes; ++icomp){
            Component* comp = *entity.data[icomp]
            *comp.~T();
        }

        bw_free(entity);
    }

    template<typename T>
    void add_component(Entity_ID entity){
    }
    template<typename T>
    void remove_component(Entity_ID entity){
    }

    // ----

    indexmap<u32> entity_map
    std::vector<Entity_Data*> entities;
};
