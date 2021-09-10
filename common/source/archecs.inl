namespace archecs{
    template<typename T>
    constexpr Type_Metadata metadataof(){
        return (Type_Metadata){sizeof(T), alignof(T), nullptr, nullptr};
    }

    template<typename ... Types>
    void Entity_Manager<Types...>::create(){
        // NOTE(hugo): extract type metadata
        constexpr Type_Metadata temp[] = { metadataof<Entity_Handle>(), (metadataof<Types>()) ... };
        memcpy(type_metadata, temp, sizeof(temp));

        entity_map.create();
        create_manager(ecs_manager);
    }

    template<typename ... Types>
    void Entity_Manager<Types...>::destroy(){
        entity_map.destroy();
        destroy_manager(ecs_manager);
    }

    template<typename ... Types>
    template<typename ... Archetype_Types>
    Archetype Entity_Manager<Types...>::create_archetype(){
        Archetype archetype;
        archetype.ntypes = 1u + sizeof...(Archetype_Types);

        // NOTE(hugo): extract type ID
        constexpr u16 temp[] = { type_index_Entity_Handle, ((u16)type_index<Archetype_Types, Entity_Handle, Types...>()) ... };
        memcpy(archetype.type_IDs, temp, sizeof(temp));

        update_archetype_metadata(ecs_manager, type_metadata, archetype);

        return archetype;
    }

    template<typename ... Types>
    Entity_Handle Entity_Manager<Types...>::create_entity(const Archetype& archetype){
        Entity entity = allocate_entity(ecs_manager, archetype);

        indexmap_handle handle = entity_map.borrow_handle();
        *(entity_map.search(handle)) = entity;

        Entity_Handle* storage_handle = (Entity_Handle*)type_memory(ecs_manager, type_metadata, entity, type_index_Entity_Handle);
        *storage_handle = (Entity_Handle){handle};

        return (Entity_Handle){handle};
    }

    template<typename ... Types>
    void Entity_Manager<Types...>::destroy_entity(const Entity_Handle entity_handle){
        Entity* entity = entity_map.search(entity_handle);

        if(entity){
            u32 moved_index = deallocate_entity(ecs_manager, type_metadata, *entity);

            if(moved_index != entity->index){
                Entity_Handle* moved_entity_handle = (Entity_Handle*)type_memory(ecs_manager, type_metadata, *entity, type_index_Entity_Handle);
                assert(moved_entity_handle);

                Entity* moved_entity = entity_map.search(*moved_entity_handle);
                assert(moved_entity);

                moved_entity->index = entity->index;
            }

            entity_map.return_handle(entity_handle);
        }
    }

    template<typename ... Types>
    template<typename T>
    void Entity_Manager<Types...>::attach_data(const Entity_Handle entity_handle){
        constexpr u32 index = type_index<T, Entity_Handle, Types...>();
        static_assert(index < 1u + sizeof...(Types));

        Entity* entity = entity_map.search(entity_handle);
        if(entity)  *entity = archecs::attach_type(ecs_manager, type_metadata, *entity, index);
        else        LOG_WARNING("attach_type (type_ID: %d) on unknown entity", index);
    }

    template<typename ... Types>
    template<typename T>
    void Entity_Manager<Types...>::detach_data(const Entity_Handle entity_handle){
        constexpr u32 index = type_index<T, Entity_Handle, Types...>();
        static_assert(index < 1u + sizeof...(Types));

        Entity* entity = entity_map.search(entity_handle);
        if(entity)  *entity = archecs::detach_type(ecs_manager, type_metadata, *entity, index);
        else        LOG_WARNING("detach_type (type_ID: %d) on unknown entity", index);
    }

    template<typename ... Types>
    template<typename T>
    T* Entity_Manager<Types...>::get_data(const Entity_Handle entity_handle){
        constexpr u32 index = type_index<T, Entity_Handle, Types...>();
        static_assert(index < 1u + sizeof...(Types));

        Entity* entity = entity_map.search(entity_handle);
        if(entity){
            return (T*)type_memory(ecs_manager, type_metadata, *entity, index);
        }else{
            LOG_WARNING("get_type (type_ID: %d) on unknown entity", index);
            return nullptr;
        }
    }

    template<typename ... Types>
    bool Entity_Manager<Types...>::available_entity(const Entity_Handle entity_handle){
        return entity_map.search(entity_handle);
    }

    template<typename ... Types>
    template<typename T>
    bool Entity_Manager<Types...>::available_data(const Entity_Handle entity_handle){
        constexpr u32 index = type_index<T, Entity_Handle, Types...>();
        static_assert(index < 1u + sizeof...(Types));

        Entity* entity = entity_map.search(entity_handle);
        if(entity)  return search_type(ecs_manager.archetypes[entity->archetype_index], index) != -1;
        else        return false;
    }

    template<typename ... Types>
    template<typename ... System_Types>
    System Entity_Manager<Types...>::create_system(){
        System system;
        system.ntypes = sizeof...(System_Types);

        // NOTE(hugo): extract type ID
        constexpr u16 temp[] = { ((u16)type_index<System_Types, Entity_Handle, Types...>()) ... };
        memcpy(system.type_IDs, temp, sizeof(temp));

        isort(system.type_IDs, system.ntypes);

        return system;
    }

    template<typename ... Types>
    void Entity_Manager<Types...>::execute_system(const System& system){
        archecs::execute_system(ecs_manager, system);
    }
}
