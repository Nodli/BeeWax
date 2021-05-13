constexpr size_t chunk_bytesize = KILOBYTES(16u) - 2u;
constexpr size_t archetype_max_types = 4u;

// TODO(hugo): compare implementation with https://imgeself.github.io/posts/2021-01-16-imge/
// https://ourmachinery.com/post/ecs-and-rendering/
// https://ourmachinery.com/post/making-the-move-rotate-scale-gizmos-work-with-any-component/

struct Type_Metadata{
    size_t bytesize;
    size_t alignment;
    void (*create)(void* ptr);
    void (*destroy)(void* ptr);
};

// TODO(hugo):
// * use a linked list of slabs ; it's fine right now because Chunks cannot be referenced through pointers
// * cache the number of entities per chunk & store the number of available entities instead of a cursor ?
// * cache the last used chunk index to search this one first ?
struct Data_Storage{
    struct Chunk{
        u8 data[chunk_bytesize];
        u16 cursor;
    };

    u32 nentities;
    std::vector<Chunk> chunks;
};

struct Archetype{
    u32 ntypes;
    size_t bytesize;
    u32 type_IDs[archetype_max_types];
    u32 type_offsets[archetype_max_types];
};

struct Entity{
    u32 archetype_index;
    u32 data_index;
};

struct Manager{
    Type_Metadata types[2u];
    std::vector<Archetype> archetypes;
    std::vector<Data_Storage> storage;
    std::vector<u32> free_index;
};

u32 add_archetype(Manager& manager){
    if(manager.free_index.size()){
        u32 index = manager.free_index[manager.free_index.size() - 1u];
        manager.free_index.pop_back();

        return index;
    }

    u32 index = manager.archetypes.size();
    manager.archetypes.resize(index + 1u);
    manager.storage.resize(index + 1u);

    return index;
}

void remove_archetype(Manager& manager, u32 index){
    assert(index < manager.archetypes.size());
    assert(manager.storage[index].nentities == 0u && manager.storage[index].chunks.size() == 0u);

    manager.free_index.push_back(index);
}

// TODO(hugo): update only for the types after index
void update_archetype_metadata(const Manager& manager, Archetype& archetype, u32 start_index){
    assert(archetype.ntypes);
    assert(start_index < archetype.ntypes);

    // NOTE(hugo): update offsets & bytesize for all types
    size_t type_offset = 0u;

    for(u32 itype = 0u; itype != archetype.ntypes; ++itype){
        const Type_Metadata& meta = manager.types[archetype.type_IDs[itype]];

        type_offset += align_offset_next(type_offset, meta.alignment);
        archetype.type_offsets[itype] = type_offset;
        type_offset += meta.bytesize;
    }

    // NOTE(hugo): post-entity padding to align the next entity
    type_offset += align_offset_next(type_offset, manager.types[archetype.type_IDs[0u]].alignment);

    archetype.bytesize = type_offset;
}

// NOTE(hugo): returns the index of type_ID in the archetype
s32 add_type(const Manager& manager, Archetype& archetype, u32 type_ID){
    assert(archetype.ntypes != archetype_max_types);

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

    update_archetype_metadata(manager, archetype, index);

    return (s32)index;
}

// NOTE(hugo): returns the index of type_ID in the archetype
s32 remove_type(const Manager& manager, Archetype& archetype, u32 type_ID){
    if(archetype.ntypes == 0u) return -1;

    // NOTE(hugo): find type index
    u32 index = 0u;
    while(index != archetype.ntypes && archetype.type_IDs[index] < type_ID) ++index;

    if(index == archetype.ntypes || archetype.type_IDs[index] != type_ID) return -1;

    // NOTE(hugo): remove type index
    for(u32 itype = index; itype != archetype.ntypes - 1u; ++itype){
        archetype.type_IDs[itype] = archetype.type_IDs[itype + 1u];
    }
    --archetype.ntypes;

    archetype.bytesize = 0u;

    if(archetype.ntypes){
        update_archetype_metadata(manager, archetype, index);
    }

    return (s32)index;
}

// NOTE(hugo): returns the data_index of the entity
u32 add_entity(const Archetype& arch, Data_Storage& storage){
    ++storage.nentities;

    if(!arch.bytesize) return UINT32_MAX;
    u32 entities_per_chunk = chunk_bytesize / arch.bytesize;

    if(storage.chunks.size()){
        u32 chunk_index = storage.chunks.size() - 1u;
        Data_Storage::Chunk& chunk = storage.chunks[chunk_index];

        if(chunk_bytesize - chunk.cursor >= arch.bytesize){
            u32 data_index = chunk_index * entities_per_chunk + chunk.cursor / arch.bytesize;
            chunk.cursor += arch.bytesize;

            return data_index;
        }
    }

    u32 data_index = storage.chunks.size() * entities_per_chunk;

    storage.chunks.resize(storage.chunks.size() + 1u);
    storage.chunks[storage.chunks.size() - 1u].cursor = arch.bytesize;

    return data_index;
}

// NOTE(hugo): returns the data_index of the entity that got moved to data_index ; or data_index if no entity was moved to data_index
u32 remove_entity(const Archetype& arch, Data_Storage& storage, u32 data_index){
    --storage.nentities;

    if(!arch.bytesize) return data_index;
    u32 entities_per_chunk = chunk_bytesize / arch.bytesize;

    u32 remove_chunk_index = data_index / entities_per_chunk;
    u32 remove_chunk_byte_offset = (data_index - remove_chunk_index * entities_per_chunk) * arch.bytesize;

    assert(remove_chunk_index < storage.chunks.size());
    Data_Storage::Chunk& remove_chunk = storage.chunks[remove_chunk_index];

#if 0
    // NOTE(hugo): move inside chunk
    if(remove_chunk_index == storage.chunks.size() - 1u){
        if(remove_chunk.cursor == arch.bytesize){
            assert(remove_chunk_byte_offset == 0u);
            storage.chunks.pop_back();
            return UINT32_MAX;

        }else if(remove_chunk.cursor == remove_chunk_byte_offset + arch.bytesize){
            remove_chunk.cursor -= arch.bytesize;
            return UINT32_MAX;

        }else{
            void* dst_ptr = remove_chunk.data + remove_chunk_byte_offset;
            void* src_ptr = remove_chunk.data + remove_chunk.cursor - arch.bytesize;
            memcpy(dst_ptr, src_ptr, arch.bytesize);

            remove_chunk.cursor -= arch.bytesize;
            u32 move_data_index = remove_chunk_index * entities_per_chunk + remove_chunk.cursor / arch.bytesize;

            return move_data_index;
        }

    // NOTE(hugo): move between chunks
    }else{
        u32 move_chunk_index = storage.chunks.size() - 1u;
        Chunk& move_chunk = storage.chunks[move_chunk_index];

        u32 move_chunk_byte_offset = move_chunk.cursor - arch.bytesize;

        void* dst_ptr = remove_chunk.data + remove_chunk_byte_offset;
        void* src_ptr = move_chunk.data + move_chunk_byte_offset;
        memcpy(dst_ptr, src_ptr, arch.bytesize);

        move_chunk.cursor -= arch.bytesize;
        u32 move_data_index = move_chunk_index + move_chunk_byte_offset / arch.bytesize;

        if(move_chunk.cursor == 0u) storage.chunks.pop_back();

        return move_data_index;
    }

#else
    u32 move_chunk_index = storage.chunks.size() - 1u;
    Data_Storage::Chunk& move_chunk = storage.chunks[move_chunk_index];
    u32 move_chunk_byte_offset = move_chunk.cursor - arch.bytesize;

    if(remove_chunk_index != move_chunk_index || remove_chunk_byte_offset != move_chunk_byte_offset){
        void* dst_ptr = remove_chunk.data + remove_chunk_byte_offset;
        void* src_ptr = move_chunk.data + move_chunk_byte_offset;
        memcpy(dst_ptr, src_ptr, arch.bytesize);
    }

    move_chunk.cursor -= arch.bytesize;
    u32 move_data_index = move_chunk_index + move_chunk_byte_offset / arch.bytesize;

    if(move_chunk.cursor == 0u) storage.chunks.pop_back();

    return move_data_index;

#endif
}

Entity allocate_entity(Manager& manager, const Archetype& arch){
    Entity entity;

    for(u32 iarch = 0u; iarch != manager.archetypes.size(); ++iarch){
        Archetype& other_arch = manager.archetypes[iarch];

        if(other_arch.bytesize == arch.bytesize
        && other_arch.ntypes == arch.ntypes
        && memcmp(other_arch.type_IDs, arch.type_IDs, arch.ntypes * sizeof(u32)) == 0u){
            entity.archetype_index = iarch;
            entity.data_index = add_entity(arch, manager.storage[iarch]);

            return entity;
        }
    }

    entity.archetype_index = add_archetype(manager);
    manager.archetypes[entity.archetype_index] = arch;

    entity.data_index = add_entity(arch, manager.storage[entity.archetype_index]);

    return entity;
}

// NOTE(hugo): returns the data_index of the entity that got moved to data_index ; or data_index if no entity was moved to data_index
u32 deallocate_entity(Manager& manager, Entity entity){
    assert(entity.archetype_index < manager.archetypes.size());

    Archetype& arch = manager.archetypes[entity.archetype_index];
    Data_Storage& storage = manager.storage[entity.archetype_index];
    u32 moved_entity = remove_entity(arch, storage, entity.data_index);

    if(storage.nentities == 0u){
        remove_archetype(manager, entity.archetype_index);
    }

    return moved_entity;
}

void* entity_memory(Manager& manager, Entity entity){
    assert(entity.archetype_index < manager.archetypes.size());

    Archetype& arch = manager.archetypes[entity.archetype_index];
    if(!arch.bytesize) return nullptr;

    u32 entities_per_chunk = chunk_bytesize / arch.bytesize;

    u32 chunk_index = entity.data_index / arch.bytesize;
    u32 chunk_byte_offset = (entity.data_index - chunk_index * entities_per_chunk) * arch.bytesize;

    Data_Storage& storage = manager.storage[entity.archetype_index];

    return (void*)(storage.chunks[chunk_index].data + chunk_byte_offset);
}

Entity add_component(Manager& manager, Entity entity, u32 type_ID){
    assert(entity.archetype_index < manager.archetypes.size());

    Archetype& arch = manager.archetypes[entity.archetype_index];

    Archetype new_arch = arch;
    s32 type_insert = add_type(manager, new_arch, type_ID);

    if(type_insert != -1){
        Entity new_entity = allocate_entity(manager, new_arch);

        void* ptr = entity_memory(manager, entity);
        if(!ptr) return new_entity;

        void* new_ptr = entity_memory(manager, new_entity);
        assert(new_ptr);

        u32 type_index = (u32)type_insert;

        if(type_index){
            u32 memcpy_bytesize = arch.type_offsets[type_index - 1u] + manager.types[arch.type_IDs[type_index - 1u]].bytesize;
            memcpy(new_ptr, ptr, memcpy_bytesize);
        }

        if(type_index != arch.ntypes){
            u32 ptr_offset = arch.type_offsets[type_index];
            u32 new_ptr_offset = new_arch.type_offsets[type_index + 1u];
            u32 memcpy_bytesize = arch.bytesize - ptr_offset;

            memcpy((u8*)new_ptr + new_ptr_offset, (u8*)ptr + ptr_offset, memcpy_bytesize);
        }

        deallocate_entity(manager, entity);

        return new_entity;
    }

    return entity;
}

Entity remove_component(Manager& manager, Entity entity, u32 type_ID){
    assert(entity.archetype_index < manager.archetypes.size());

    Archetype& arch = manager.archetypes[entity.archetype_index];

    Archetype new_arch = arch;
    s32 type_insert = remove_type(manager, new_arch, type_ID);

    if(type_insert != -1){
        Entity new_entity = allocate_entity(manager, new_arch);

        void* new_ptr = entity_memory(manager, new_entity);
        if(!new_ptr) return new_entity;

        void* ptr = entity_memory(manager, entity);
        assert(ptr);

        u32 type_index = (u32)type_insert;

        if(type_index){
            u32 memcpy_bytesize = arch.type_offsets[type_index - 1u] + manager.types[arch.type_IDs[type_index - 1u]].bytesize;
            memcpy(new_ptr, ptr, memcpy_bytesize);
        }

        if(type_index != arch.ntypes - 1u){
            u32 ptr_offset = arch.type_offsets[type_index + 1u];
            u32 new_ptr_offset = arch.type_offsets[type_index];
            u32 memcpy_bytesize = arch.bytesize - ptr_offset;

            memcpy((u8*)new_ptr + new_ptr_offset, (u8*)ptr + ptr_offset, memcpy_bytesize);
        }

        deallocate_entity(manager, entity);

        return new_entity;

    }

    return entity;
}

void* component_memory(Manager& manager, Entity entity, u32 type_ID){
    assert(entity.archetype_index < manager.archetypes.size());

    void* entity_mem = entity_memory(manager, entity);

    Archetype& archetype = manager.archetypes[entity.archetype_index];

    // NOTE(hugo): find type index
    u32 index = 0u;
    while(index != archetype.ntypes && archetype.type_IDs[index] < type_ID) ++index;

    if(index == archetype.ntypes || archetype.type_IDs[index] != type_ID) return nullptr;

    u32 type_offset = archetype.type_offsets[index];

    return (void*)((u8*)entity_mem + type_offset);
}

struct Component_Health{
    u32 health;
};
struct Component_Position{
    vec2 position;
};

void do_the_thing(){

    Manager manager;

    manager.types[0u].bytesize = sizeof(Component_Health);
    manager.types[0u].alignment = alignof(Component_Health);
    manager.types[1u].bytesize = sizeof(Component_Position);
    manager.types[1u].alignment = alignof(Component_Position);

    Archetype arch;
    arch.ntypes = 0u;
    arch.bytesize = 0u;

    Entity entity = allocate_entity(manager, arch);

    {
        entity = add_component(manager, entity, 0u);
        Component_Health* comp = (Component_Health*)component_memory(manager, entity, 0u);
        assert(comp);
        comp->health = 100.f;
    }

    {
        entity = add_component(manager, entity, 1u);
        Component_Position* compP = (Component_Position*)component_memory(manager, entity, 1u);
        assert(compP);
        compP->position = {-1.f, -2.f};

        Component_Health* compH = (Component_Health*)component_memory(manager, entity, 0u);
        assert(compH && compH->health == 100.f);
    }

    {
        entity = remove_component(manager, entity, 0u);
        Component_Health* compH = (Component_Health*)component_memory(manager, entity, 0u);
        assert(!compH);

        Component_Position* compP = (Component_Position*)component_memory(manager, entity, 1u);
        assert(compP && compP->position == vec2({-1.f, -2.f}));
    }

    {
        entity = remove_component(manager, entity, 1u);
        assert(!component_memory(manager, entity, 0u));
        assert(!component_memory(manager, entity, 1u));
    }

    deallocate_entity(manager, entity);
}


