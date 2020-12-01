#ifndef H_TWEENING
#define H_TWEENING

#define FOR_EACH_TWEEN_TYPE(FUNCTION)   \
FUNCTION(float)                         \
FUNCTION(vec2)                          \

template<typename T>
struct Tween_ID{
    explicit operator T() const {return *this;};
    bool operator==(const Tween_ID<T>& rhs) const {return index == rhs.index && generation == rhs.generation;};
    bool operator!=(const Tween_ID<T>& rhs) const {return !(*this == rhs);};

    u32 index;
    u32 generation;
};

template<typename T>
constexpr Tween_ID<T> unknown_tween = {UINT_MAX, UINT_MAX};

struct Tween_Manager{
    template<typename T>
    struct Tween{
        T* ptr = nullptr;
        u32 tick_count = 0u;
        u32 tick_duration = 0u;
        T base;
        T range;
        u32 generation;
    };

    void terminate();

    void next_tick();

#define DECLARE_TWEEN_METHODS(Type)                                                     \
    Tween_ID<Type> start_tween(Type* ptr, Type start, Type stop, u32 tick_duration);    \
    bool is_valid(Tween_ID<Type> tween);                                                \
    void stop_tween(Tween_ID<Type> tween);

    FOR_EACH_TWEEN_TYPE(DECLARE_TWEEN_METHODS)

#undef DECLARE_TWEEN_METHODS

    // --- data

    u32 generation = 0u;

#define DECLARE_TWEEN_STORAGE(Type) diterpool<Tween_Manager::Tween<Type>> CONCATENATE(storage_, Type);
    FOR_EACH_TWEEN_TYPE(DECLARE_TWEEN_STORAGE)
#undef DECLARE_TWEEN_STORAGE
};

void Tween_Manager::terminate(){
#define TWEEN_TERMINATE(Type) CONCATENATE(storage_, Type).free();
    FOR_EACH_TWEEN_TYPE(TWEEN_TERMINATE)
#undef TWEEN_TERMINATE
}

template<typename T>
static void storage_next_tick(diterpool<Tween_Manager::Tween<T>>& storage){
    u32 index, counter;
    for(index = storage.get_first(), counter = 0u;
        index < storage.capacity;
        index = storage.get_next(index), ++counter){

        Tween_Manager::Tween<T>& tween = storage[index];
        *tween.ptr = tween.base + tween.range * (float)tween.tick_count++ / (float)tween.tick_duration;

        if(tween.tick_count == tween.tick_duration){
            storage.remove(index);
        }
    }
}

void Tween_Manager::next_tick(){
#define TWEEN_STORAGE_NEXT_TICK(Type) storage_next_tick(CONCATENATE(storage_, Type));
    FOR_EACH_TWEEN_TYPE(TWEEN_STORAGE_NEXT_TICK)
#undef TWEEN_STORAGE_NEXT_TICK
}

template<typename T>
static Tween_ID<T> storage_start_tween(u32& manager_generation, diterpool<Tween_Manager::Tween<T>>& storage, T* ptr, T start, T stop, u32 tick_duration){
    Tween_Manager::Tween<T> tween;
    tween.ptr = ptr;
    tween.tick_count = 0u;
    tween.tick_duration = tick_duration;
    tween.base = start;
    tween.range = stop - start;
    tween.generation = manager_generation++;

    u32 tween_index = storage.insert(tween);
    Tween_ID<T> output = {tween_index, tween.generation};
    return output;
}

template<typename T>
static bool storage_is_valid(diterpool<Tween_Manager::Tween<T>>& storage, Tween_ID<T> tween){
    return tween.index < storage.capacity
        && storage.is_active(tween.index)
        && storage[tween.index].generation == tween.generation;
}

template<typename T>
static void storage_stop_tween(diterpool<Tween_Manager::Tween<T>>& storage, Tween_ID<T> tween){
    if(storage_is_valid(storage, tween)){
        storage.remove(tween.index);
    }
}

#define DEFINE_TWEEN_METHODS(Type)                                                                          \
Tween_ID<Type> Tween_Manager::start_tween(Type* ptr, Type start, Type stop, u32 tick_duration){             \
    return storage_start_tween(generation, CONCATENATE(storage_, Type), ptr, start, stop, tick_duration);   \
}                                                                                                           \
bool Tween_Manager::is_valid(Tween_ID<Type> tween){                                                         \
    return storage_is_valid(CONCATENATE(storage_, Type), tween);                                            \
}                                                                                                           \
void Tween_Manager::stop_tween(Tween_ID<Type> tween){                                                       \
    storage_stop_tween(CONCATENATE(storage_, Type), tween);                                                 \
}

    FOR_EACH_TWEEN_TYPE(DEFINE_TWEEN_METHODS)

#undef DEFINE_TWEEN_METHODS

#endif
