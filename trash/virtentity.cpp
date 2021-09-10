// REF(hugo):
// https://math.stackexchange.com/questions/13150/extracting-rotation-scale-values-from-2d-transformation-matrix

vec2 Scene_Transform::position() const{
    return {matrix.data[6u], matrix.data[7u]};
}

float Scene_Transform::orientation() const{
    return atan2f(matrix.data[3u], matrix.data[4u]);
}

vec2 Scene_Transform::forward() const{
    return normalized(vec2({matrix.data[3u], matrix.data[4u]}));
}

vec2 Scene_Transform::right() const{
    return normalized(vec2({matrix.data[0u], matrix.data[1u]}));
}

mat3 Local_Transform::matrix() const{
    float c = bw::cos(orientation);
    float s = bw::sin(orientation);
    mat3 mat_rot = mat3_rm(
        c,   - s, 0.f,
        s,     c, 0.f,
        0.f, 0.f, 1.f
    );
    mat3 mat_trans = mat3_rm(
        1.f, 0.f, position.x,
        0.f, 1.f, position.y,
        0.f, 0.f, 1.f
    );
    return mat_rot * mat_trans;
}

static Scene_Transform compute_transform(const Scene_Transform& parent, const Local_Transform& local){
    mat3 local_matrix = local.matrix();

    Scene_Transform output;
    output.matrix = parent.matrix * local.matrix();
    output.depth = parent.depth + local.depth;
    return output;
}

static Scene_Transform compute_transform(const Local_Transform& local){
    Scene_Transform output;
    output.matrix = local.matrix();
    output.depth = local.depth;
    return output;
}

void Entity::create(void* context){};
void Entity::destroy(void* context){};
void Entity::update(void* context){};
void Entity::render(void* context){};

DEFINE_ENTITY_TYPE(Entity);

void Entity::set_parent(Entity* new_parent){
    assert(new_parent);

    if(parent) remove_parent();

    // NOTE(hugo): insert this in new connections
    Entity* new_next = this;
    Entity* new_prev = this;

    if(new_parent){
        if((*new_parent).child){
            new_next = (*new_parent).child;
            new_prev = (*new_next).prev_sibling;

            (*new_prev).next_sibling = this;
            (*new_next).prev_sibling = this;
        }

        (*new_parent).child = this;
    }

    parent = new_parent;
    prev_sibling = new_prev;
    next_sibling = new_next;

    update_transform();
}

void Entity::remove_parent(){
    assert(parent);

    Entity* current_parent = parent;
    Entity* current_prev = prev_sibling;
    Entity* current_next = next_sibling;

    if(current_prev == this){
        assert(current_next == this && (*current_parent).child == this);
        (*current_parent).child = nullptr;
    }else{
        assert(current_next != this);
        (*current_parent).child = current_next;
        (*current_prev).next_sibling = current_next;
        (*current_next).prev_sibling = current_prev;
    }
}

void Entity::update_transform(){
    if(parent)  scene_transform = compute_transform((*parent).scene_transform, local_transform);
    else        scene_transform = compute_transform(local_transform);

    FOR_EACH_CHILD(this, iter){
        (*iter).update_transform();
    }
}

void Entity_Manager::create(){
    entity_map.create();
    entities.create();
}

void Entity_Manager::destroy(){
    entity_map.destroy();
    entities.destroy();
}

void Entity_Manager::update(){
    for(auto entity : entities){
        (*entity).update(context);
    }
}

void Entity_Manager::render(){
    for(auto entity : entities){
        (*entity).render(context);
    }
}

template<typename T>
T* Entity_Manager::create_entity(){
    indexmap_handle handle = entity_map.borrow_handle();
    *entity_map.search(handle) = entities.size;

    // NOTE(hugo): bw_new because we need the virtual table pointer
    T* typed_ptr = bw_new(T);

    entities.push((Entity*)typed_ptr);
    (*typed_ptr).handle = handle;

    (*typed_ptr).create(this);

    return typed_ptr;
}

void Entity_Manager::destroy_entity(Entity* entity){
    assert(entity);

    (*entity).remove_parent();
    Entity* child = (*entity).child;

    (*entity).destroy(context);

    u32* entity_index = entity_map.search((*entity).handle);
    assert(entity_index);

    *entity_map.search((*entities[entities.size - 1u]).handle) = *entity_index;
    entities.remove_swap(*entity_index);

    entity_map.return_handle((*entity).handle);

    // NOTE(hugo): bw_delete because we are using the constructor so we might as well allow destructors and std containers
    bw_delete(entity);


    // NOTE(hugo): destroy children
    if(child){
        Entity* cstart = child;
        Entity* citer = child;
        do{
            destroy_entity(cstart);
            (*citer).update_transform();
            citer = (*citer).next_sibling;
        }while(citer != cstart);
    }
}

void Entity_Manager::destroy_entity(indexmap_handle entity_handle){
    u32* index = entity_map.search(entity_handle);
    if(index){
        Entity* ptr = entities[*index];
        destroy_entity(ptr);
    }
}

Entity* Entity_Manager::get_entity(indexmap_handle entity_handle){
    Entity* ptr = nullptr;

    u32* index = entity_map.search(entity_handle);
    if(index) ptr = entities[*index];

    return ptr;
}

