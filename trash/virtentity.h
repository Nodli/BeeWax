struct Scene_Transform{
    vec2 position() const;
    float orientation() const;
    vec2 forward() const;
    vec2 right() const;

    // ----

    mat3 matrix = identity_matrix<mat3>;
    float depth = 0.f;
};

struct Local_Transform{
    mat3 matrix() const;

    // ----

    vec2 position = {0.f, 0.f};
    float orientation = 0.f;
    float depth = 0.f;
};

// NOTE(hugo): Entity_Type must be compared with operator= and not strcmp
// because entities of the same type will refer to the same static string
struct Entity_Type{ const char* type_str; };

#define DECLARE_ENTITY_TYPE(Type)                           \
static  constexpr const char* type_str = STRINGIFY(Type);   \
static  Entity_Type stype();                                \
virtual Entity_Type vtype();

#define DEFINE_ENTITY_TYPE(Type)                \
Entity_Type Type::stype(){ return {type_str}; } \
Entity_Type Type::vtype(){ return {type_str}; }

struct Entity{
    virtual void create(void* context);
    virtual void destroy(void* context);
    virtual void update(void* context);
    virtual void render(void* context);

    DECLARE_ENTITY_TYPE(Entity);

    // ----

    void remove_parent();
    void set_parent(Entity* new_parent);

    void update_transform();

    void force_global_transform(vec2 position, float orientation);

    // ----

    indexmap_handle handle = indexmap_null_handle;

    Entity* parent = nullptr;
    // NOTE(hugo): siblings make a circular linked list
    Entity* prev_sibling = this;
    Entity* next_sibling = this;
    Entity* child = nullptr;

    Local_Transform local_transform;
    Scene_Transform scene_transform;
};

#define FOR_EACH_SIBLING(Entity_ptr, iter)  for(Entity* iter = (*Entity_ptr).next_sibling;  iter != Entity_ptr; iter = (*iter).next_sibling)
#define FOR_EACH_CHILD(Entity_ptr, iter)    for(Entity* iter = (*Entity_ptr).child;         iter;               iter = ((*iter).next_sibling == (*Entity_ptr).child) ? nullptr : (*iter).next_sibling)

struct Entity_Manager{
    void create();
    void destroy();

    void update();
    void render();

    // ----

    template<typename T>
    T* create_entity();

    void destroy_entity(Entity* entity);

    void destroy_entity(indexmap_handle entity_handle);

    Entity* get_entity(indexmap_handle entity_handle);

    // ----

    Entity root;

    indexmap<u32> entity_map;
    array<Entity*> entities;

    void* context;
};
