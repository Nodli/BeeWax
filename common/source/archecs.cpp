namespace archecs{
    // --

    s32 add_type(const Manager& manager, const Type_Metadata* type_metadata, Archetype& archetype, u32 type_ID){
        assert(archetype.ntypes != max_ntypes);

        // NOTE(hugo): find insertion index
        u32 index = 0u;
        while(index != archetype.ntypes && archetype.type_IDs[index] < type_ID) ++index;

        if(index < archetype.ntypes && archetype.type_IDs[index] == type_ID) return -1;

        // NOTE(hugo): insert type index
        for(u32 itype = archetype.ntypes; itype != index; --itype){
            archetype.type_IDs[itype] = archetype.type_IDs[itype - 1u];
        }
        archetype.type_IDs[index] = type_ID;
        ++archetype.ntypes;

        update_archetype_metadata(manager, type_metadata, archetype);

        return (s32)index;
    }

    s32 remove_type(const Manager& manager, const Type_Metadata* type_metadata, Archetype& archetype, u32 type_ID){
        // NOTE(hugo): find type index
        u32 index = 0u;
        while(index != archetype.ntypes && archetype.type_IDs[index] < type_ID) ++index;

        if(index == archetype.ntypes || archetype.type_IDs[index] != type_ID) return -1;

        // NOTE(hugo): remove type index
        for(u32 itype = index; itype != archetype.ntypes - 1u; ++itype){
            archetype.type_IDs[itype] = archetype.type_IDs[itype + 1u];
        }
        --archetype.ntypes;

        update_archetype_metadata(manager, type_metadata, archetype);

        return (s32)index;
    }

    void update_archetype_metadata(const Manager& manager, const Type_Metadata* type_metadata, Archetype& archetype){
        size_t sum_bytesize = 0u;
        for(u32 itype = 0u; itype != archetype.ntypes; ++itype) sum_bytesize += type_metadata[archetype.type_IDs[itype]].bytesize;

        u32 entities_per_chunk;

        if(sum_bytesize){
            entities_per_chunk = chunk_bytesize / sum_bytesize;

            size_t type_offset;

            auto compute_type_offset = [&](){
                type_offset = 0u;

                for(u32 itype = 0u; itype != archetype.ntypes; ++itype){
                    const Type_Metadata& meta = type_metadata[archetype.type_IDs[itype]];

                    type_offset += align_offset_next(type_offset, meta.alignment);
                    archetype.type_offsets[itype] = type_offset;
                    type_offset += meta.bytesize * entities_per_chunk;
                }
            };

            compute_type_offset();

            // NOTE(hugo): may happend because of alignment
            while(type_offset > chunk_bytesize){
                --entities_per_chunk;
                compute_type_offset();
            }

        }else{
            entities_per_chunk = infinite_entities_per_chunk;

        }

        archetype.entities_per_chunk = entities_per_chunk;
    }

    s32 search_type(const Archetype& archetype, u32 type_ID){
        // NOTE(hugo): find type index
        u32 index = 0u;
        while(index != archetype.ntypes && archetype.type_IDs[index] < type_ID) ++index;

        if(index == archetype.ntypes || archetype.type_IDs[index] != type_ID)   return -1;
        else                                                                    return index;
    }

    u32 allocate_archetype(Manager& manager){
        if(manager.free_archetype_head){
            Archetype* free_ptr = manager.free_archetype_head;
            manager.free_archetype_head = *(Archetype**)free_ptr;
            return free_ptr - manager.archetypes.data;
        }

        u32 index = manager.archetypes.size;

        Archetype archetype;
        manager.archetypes.push(archetype);

        Archetype_Storage storage;
        storage.nentities = 0u;
        storage.free = 0u;
        storage.chunks.create();
        manager.storage.push(storage);

        return index;
    }

    void deallocate_archetype(Manager& manager, u32 index){
        assert(index < manager.archetypes.size);
        assert(!manager.storage[index].chunks.size && !manager.storage[index].nentities);

        Archetype* free_ptr = &manager.archetypes[index];
        *(Archetype**)free_ptr = manager.free_archetype_head;
        manager.free_archetype_head = free_ptr;
    }

    u32 allocate_entity_storage(Manager& man, const Archetype& arch, Archetype_Storage& storage){
        // NOTE(hugo): nothing to allocate
        if(arch.entities_per_chunk == infinite_entities_per_chunk){
            ++storage.nentities;
            return UINT32_MAX;
        }

        // NOTE(hugo): chunk space available
        if(storage.free){
            --storage.free;
            return storage.nentities++;
        }

        // NOTE(hugo): no chunk space ; retrieve / allocate a chunk
        Chunk* new_chunk;
        if(man.free_chunk_head){
            new_chunk = man.free_chunk_head;
            man.free_chunk_head = *(Chunk**)man.free_chunk_head;

        }else{
            new_chunk = (Chunk*)bw_malloc(sizeof(Chunk));
        }

        storage.chunks.push(new_chunk);
        storage.free = arch.entities_per_chunk - 1u;

        return storage.nentities++;
    }

    u32 deallocate_entity_storage(Manager& man, const Type_Metadata* type_metadata, const Archetype& arch, Archetype_Storage& storage, u32 index){
        // NOTE(hugo): nothing to deallocate
        if(arch.entities_per_chunk == infinite_entities_per_chunk){
            --storage.nentities;
            return index;
        }

        u32 remove_chunk_index = index / arch.entities_per_chunk;
        Chunk* remove_chunk = storage.chunks[remove_chunk_index];
        u32 remove_sub_index = index - remove_chunk_index * arch.entities_per_chunk;

        u32 move_chunk_index = storage.chunks.size - 1u;
        Chunk* move_chunk = storage.chunks[move_chunk_index];
        u32 move_sub_index = storage.nentities - 1u - move_chunk_index * arch.entities_per_chunk;

        // NOTE(hugo): data copy
        if(remove_chunk_index != move_chunk_index || remove_sub_index != move_sub_index){
            for(u32 itype = 0u; itype != arch.ntypes; ++itype){
                size_t type_offset = arch.type_offsets[itype];
                size_t type_bytesize = type_metadata[arch.type_IDs[itype]].bytesize;

                void* src_ptr = move_chunk->data   + type_offset + move_sub_index   * type_bytesize;
                void* dst_ptr = remove_chunk->data + type_offset + remove_sub_index * type_bytesize;
                memcpy(dst_ptr, src_ptr, type_bytesize);
            }
        }

        ++storage.free;

        // NOTE(hugo): return chunk
        if(storage.free == arch.entities_per_chunk){
            *(Chunk**)move_chunk = man.free_chunk_head;
            man.free_chunk_head = move_chunk;
            storage.chunks.pop();
            storage.free = 0u;
        }

        --storage.nentities;
        return storage.nentities;
    }

    Entity allocate_entity(Manager& manager, const Archetype& arch){
        Entity entity;

        for(u32 iarch = 0u; iarch != manager.archetypes.size; ++iarch){
            Archetype& other_arch = manager.archetypes[iarch];

            if(other_arch.entities_per_chunk == arch.entities_per_chunk
            && other_arch.ntypes == arch.ntypes
            && memcmp(other_arch.type_IDs, arch.type_IDs, arch.ntypes * sizeof(u32)) == 0u){
                entity.archetype_index = iarch;
                entity.index = allocate_entity_storage(manager, arch, manager.storage[iarch]);

                return entity;
            }
        }

        entity.archetype_index = allocate_archetype(manager);
        manager.archetypes[entity.archetype_index] = arch;

        entity.index = allocate_entity_storage(manager, arch, manager.storage[entity.archetype_index]);

        return entity;
    }

    u32 deallocate_entity(Manager& manager, const Type_Metadata* type_metadata, Entity entity){
        assert(entity.archetype_index < manager.archetypes.size);

        Archetype& arch = manager.archetypes[entity.archetype_index];
        Archetype_Storage& storage = manager.storage[entity.archetype_index];
        u32 moved_entity = deallocate_entity_storage(manager, type_metadata, arch, storage, entity.index);

        if(!storage.nentities) deallocate_archetype(manager, entity.archetype_index);

        return moved_entity;
    }

    Entity attach_type(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID){
        assert(entity.archetype_index < manager.archetypes.size);

        Archetype& arch = manager.archetypes[entity.archetype_index];

        Archetype new_arch = arch;
        s32 type_insert = add_type(manager, type_metadata, new_arch, type_ID);

        if(type_insert != -1){
            Entity new_entity = allocate_entity(manager, new_arch);

            if(arch.entities_per_chunk != infinite_entities_per_chunk
            && new_arch.entities_per_chunk != infinite_entities_per_chunk){

                Archetype_Storage& storage = manager.storage[entity.archetype_index];
                u32 chunk_index = entity.index / arch.entities_per_chunk;
                u32 chunk_sub_index = entity.index - chunk_index * arch.entities_per_chunk;
                Chunk* chunk = storage.chunks[chunk_index];

                Archetype_Storage& new_storage = manager.storage[new_entity.archetype_index];
                u32 new_chunk_index = new_entity.index / new_arch.entities_per_chunk;
                u32 new_chunk_sub_index = new_entity.index - new_chunk_index * new_arch.entities_per_chunk;
                Chunk* new_chunk = new_storage.chunks[new_chunk_index];

                u32 type_index = (u32)type_insert;

                for(u32 itype = 0u; itype != type_index; ++itype){
                    size_t type_bytesize = type_metadata[arch.type_IDs[itype]].bytesize;

                    void* src_ptr = chunk->data     + arch.type_offsets[itype]      + chunk_sub_index     * type_bytesize;
                    void* dst_ptr = new_chunk->data + new_arch.type_offsets[itype]  + new_chunk_sub_index * type_bytesize;
                    memcpy(dst_ptr, src_ptr, type_bytesize);
                }
                for(u32 itype = type_index; itype != arch.ntypes; ++itype){
                    size_t type_bytesize = type_metadata[arch.type_IDs[itype]].bytesize;

                    void* src_ptr = chunk->data     + arch.type_offsets[itype]          + chunk_sub_index     * type_bytesize;
                    void* dst_ptr = new_chunk->data + new_arch.type_offsets[itype + 1u] + new_chunk_sub_index * type_bytesize;
                    memcpy(dst_ptr, src_ptr, type_bytesize);
                }
            }

            deallocate_entity(manager, type_metadata, entity);

            return new_entity;
        }

        return entity;
    }

    Entity detach_type(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID){
        assert(entity.archetype_index < manager.archetypes.size);

        Archetype& arch = manager.archetypes[entity.archetype_index];

        Archetype new_arch = arch;
        s32 type_insert = remove_type(manager, type_metadata, new_arch, type_ID);

        if(type_insert != -1){
            Entity new_entity = allocate_entity(manager, new_arch);

            if(arch.entities_per_chunk != infinite_entities_per_chunk
            && new_arch.entities_per_chunk != infinite_entities_per_chunk){

                Archetype_Storage& storage = manager.storage[entity.archetype_index];
                u32 chunk_index = entity.index / arch.entities_per_chunk;
                u32 chunk_sub_index = entity.index - chunk_index * arch.entities_per_chunk;
                Chunk* chunk = storage.chunks[chunk_index];

                Archetype_Storage& new_storage = manager.storage[new_entity.archetype_index];
                u32 new_chunk_index = new_entity.index / new_arch.entities_per_chunk;
                u32 new_chunk_sub_index = new_entity.index - new_chunk_index * new_arch.entities_per_chunk;
                Chunk* new_chunk = new_storage.chunks[new_chunk_index];

                u32 type_index = (u32)type_insert;
                for(u32 itype = 0u; itype != type_index; ++itype){
                    size_t type_bytesize = type_metadata[arch.type_IDs[itype]].bytesize;

                    void* src_ptr = chunk->data     + arch.type_offsets[itype]      + chunk_sub_index     * type_bytesize;
                    void* dst_ptr = new_chunk->data + new_arch.type_offsets[itype]  + new_chunk_sub_index * type_bytesize;
                    memcpy(dst_ptr, src_ptr, type_bytesize);
                }
                for(u32 itype = type_index + 1u; itype != arch.ntypes; ++itype){
                    size_t type_bytesize = type_metadata[arch.type_IDs[itype]].bytesize;

                    void* src_ptr = chunk->data     + arch.type_offsets[itype]  + chunk_sub_index     * type_bytesize;
                    void* dst_ptr = new_chunk->data + new_arch.type_offsets[itype - 1u]   + new_chunk_sub_index * type_bytesize;
                    memcpy(dst_ptr, src_ptr, type_bytesize);
                }
            }

            deallocate_entity(manager, type_metadata, entity);

            return new_entity;
        }

        return entity;
    }

    void* type_memory(Manager& manager, const Type_Metadata* type_metadata, Entity entity, u32 type_ID){
        Archetype& arch = manager.archetypes[entity.archetype_index];

        // NOTE(hugo): find index in arch
        u32 type_index = 0u;
        while(type_index != arch.ntypes && arch.type_IDs[type_index] < type_ID) ++type_index;

        if(type_index == arch.ntypes
        || arch.type_IDs[type_index] != type_ID
        || arch.entities_per_chunk == infinite_entities_per_chunk){
            return nullptr;
        }

        u32 chunk_index = entity.index / arch.entities_per_chunk;
        u32 chunk_sub_index = entity.index - chunk_index * arch.entities_per_chunk;
        Chunk* chunk = manager.storage[entity.archetype_index].chunks[chunk_index];

        return chunk->data + arch.type_offsets[type_index] + chunk_sub_index * type_metadata[type_ID].bytesize;
    }

    void create_manager(Manager& manager){
        manager.archetypes.create();
        manager.storage.create();
        manager.free_archetype_head = nullptr;
        manager.free_chunk_head = nullptr;
    }

    void destroy_manager(Manager& manager){
        manager.archetypes.destroy();

        for(auto& storage : manager.storage){
            for(auto& chunk : storage.chunks){
                bw_free(chunk);
            }
            storage.chunks.destroy();
        }
        manager.storage.destroy();

        Chunk* ptr = manager.free_chunk_head;
        while(ptr){
            Chunk* to_free = ptr;
            ptr = *(Chunk**)ptr;
            bw_free(to_free);
        }
    }

    u32 system_assess_archetype(const System& sys, const Archetype& arch, System_Param& param){
        if(arch.ntypes < sys.ntypes) return 0u;

        u32 iarch = 0u;
        u32 isys = 0u;

        while(iarch != arch.ntypes){
            if(sys.type_IDs[isys] == arch.type_IDs[iarch]){
                param.type_offsets[isys] = arch.type_offsets[iarch];
                ++isys;
                if(isys == sys.ntypes) return 1u;
            }
            ++iarch;
        }

        return 0u;
    }

    void execute_system(Manager& manager, const System& system){
        System_Param param;

        for(u32 iarch = 0u; iarch != manager.archetypes.size; ++iarch){
            Archetype& arch = manager.archetypes[iarch];

            if(system_assess_archetype(system, arch, param)){
                Archetype_Storage& storage = manager.storage[iarch];

                // NOTE(hugo): complete chunks
                for(u32 ichunk = 1u; ichunk < storage.chunks.size; ++ichunk){
                    param.chunk = storage.chunks[ichunk - 1u];
                    param.nentities = arch.entities_per_chunk;
                    system.update(system.data, param);
                }

                // NOTE(hugo): partial chunk at the end
                if(storage.chunks.size){
                    param.chunk = storage.chunks[storage.chunks.size - 1u];
                    param.nentities = arch.entities_per_chunk - storage.free;
                    system.update(system.data, param);
                }
            }
        }
    }
}


namespace archecs::unit_test{
    struct Component_u32{
        u32 value;
    };
    struct Component_vec2{
        vec2 value;
    };
};

void archecs_unit_test(){
    using namespace archecs;
    using namespace archecs::unit_test;

    archecs::Entity_Manager<Component_u32, Component_vec2> manager;
    manager.create();

    {
        archecs::Archetype empty_archetype = manager.create_archetype();
        archecs::Entity_Handle entity = manager.create_entity(empty_archetype);
        assert(manager.available_entity(entity));

        archecs::Entity_Handle* storage_entity = manager.get_data<Entity_Handle>(entity);
        assert(storage_entity);
        assert(storage_entity->virtual_index == entity.virtual_index);
        assert(storage_entity->generation    == entity.generation);

        manager.destroy_entity(entity);
        assert(!manager.available_entity(entity));
        assert(!manager.available_data<Entity_Handle>(entity));
    }

    {
        archecs::Archetype archetype = manager.create_archetype<Component_u32>();
        archecs::Entity_Handle entity = manager.create_entity(archetype);

        {
            Component_u32* cu32 = manager.get_data<Component_u32>(entity);
            assert(cu32);
            cu32->value = 100u;
        }

        manager.attach_data<Component_vec2>(entity);

        {
            Component_u32* cu32 = manager.get_data<Component_u32>(entity);
            assert(cu32);
            assert(cu32->value == 100u);

            Component_vec2* cvec2 = manager.get_data<Component_vec2>(entity);
            assert(cvec2);
            cvec2->value.x = -10.f;
            cvec2->value.y = -20.f;
        }

        manager.detach_data<Component_u32>(entity);

        {
            Component_u32* cu32 = manager.get_data<Component_u32>(entity);
            assert(cu32 == nullptr);

            Component_vec2* cvec2 = manager.get_data<Component_vec2>(entity);
            assert(cvec2);
            assert(cvec2->value.x == -10.f && cvec2->value.y == -20.f);
        }

        manager.detach_data<Component_vec2>(entity);

        manager.destroy_entity(entity);
    }

    manager.destroy();
}

