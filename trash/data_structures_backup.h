// ---- dring

template<typename T>
struct dring{

    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void push_front(const T& value);
    void pop_front();
    void push_back(const T& value);
    void pop_back();

    // NOTE(hugo): elements are removed from the tail when reducing capacity
    void set_capacity(u32 new_capacity);
    void set_min_capacity(u32 new_capacity);

    void clear();
    void free();

    size_t size_in_bytes();
    size_t capacity_in_bytes();

    // ---- data

    T* data = nullptr;
    u32 size = 0u;
    u32 capacity = 0u;
    u32 head_index = 0u;
};

// ---- dring

template<typename T>
static inline void dring_increase_capacity(dring<T>& dr, u32 new_capacity){
    assert(new_capacity > 0u);

    void* new_data = realloc((void*)dr.data, new_capacity * sizeof(T));
    if(new_data){
        dr.data = (T*)new_data;
    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory but subsequent write may be out of bound");
    }

    u32 head_to_endptr = dr.capacity - dr.head_index;
    if(dr.size > head_to_endptr){
        u32 new_head_index = new_capacity - head_to_endptr;
        memmove(dr.data + new_head_index, dr.data + dr.head_index, head_to_endptr * sizeof(T));
        dr.head_index = new_head_index;
    }

    dr.capacity = new_capacity;
}

template<typename T>
static inline void dring_decrease_capacity(dring<T>& dr, u32 new_capacity){
    assert(new_capacity > 0u);

    // NOTE(hugo): translate the head over the end of the tail before reducing memory
    // head_index > 0 means that capacity > 0
    if(dr.head_index > 0u){
        u32 head_to_endptr = min(dr.capacity - dr.head_index, new_capacity);
        u32 new_head_index = new_capacity - head_to_endptr;
        memmove(dr.data + new_head_index, dr.data + dr.head_index, head_to_endptr * sizeof(T));
        dr.head_index = new_head_index;
    }
    dr.capacity = new_capacity;
    dr.size = min(dr.size, dr.capacity);

    void* new_data = realloc((void*)dr.data, new_capacity * sizeof(T));
    if(new_data){
        dr.data = new_data;
    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory but subsequent write may be out of bound");
    }
}

static inline u32 dring_data_index_under(u32 queue_index_negative, u32 head_index, u32 capacity){
    u32 data_index = head_index + (queue_index_negative > head_index) * capacity;
    data_index = data_index - queue_index_negative;
    return data_index;
}

static inline u32 dring_data_index_over(u32 queue_index_positive, u32 head_index, u32 capacity){
    u32 data_index = head_index + queue_index_positive;
    data_index = data_index - (data_index >= capacity) * capacity;
    return data_index;
}

template<typename T>
T& dring<T>::operator[](u32 index){
    assert(index < size);

    u32 data_index = dring_data_index_over(index, head_index, capacity);
    return data[data_index];
}

template<typename T>
const T& dring<T>::operator[](u32 index) const{
    assert(index < size);

    u32 data_index = dring_data_index_over(index, head_index, capacity);
    return data[data_index];
}

template<typename T>
void dring<T>::push_front(const T& value){
    if(size == capacity){
        dring_increase_capacity(*this, max(1u, capacity) * 2u);
    }

    u32 insert_index = dring_data_index_under(1u, head_index, capacity);
    data[insert_index] = value;
    head_index = insert_index;
    ++size;
}

template<typename T>
void dring<T>::pop_front(){
    assert(size);
    --size;
    //data[head_index].~T();
    head_index = dring_data_index_over(1u, head_index, capacity);
}

template<typename T>
void dring<T>::push_back(const T& value){
    if(size == capacity){
        dring_increase_capacity(*this, max(1u, capacity) * 2u);
    }

    u32 insert_index = dring_data_index_over(size, head_index, capacity);
    data[insert_index] = value;
    ++size;
}

template<typename T>
void dring<T>::pop_back(){
    assert(size);
    //operator[](size - 1u).~T();
    --size;
}

template<typename T>
void dring<T>::set_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        dring_increase_capacity(*this, new_capacity);
    }else if(new_capacity < capacity){
        //for(u32 ielement = new_capacity; ielement != capacity; ++ielement){
        //    operator[](ielement).~T();
        //}

        dring_decrease_capacity(*this, new_capacity);
    }
}

template<typename T>
void dring<T>::set_min_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        dring_increase_capacity(*this, new_capacity);
    }
}

template<typename T>
void dring<T>::clear(){
    //for(u32 ielement = 0u; ielement != size; ++ielement){
    //    operator[](ielement).~T();
    //}

    size = 0u;
    head_index = 0u;
}

template<typename T>
void dring<T>::free(){
    //for(u32 ielement = 0u; ielement != size; ++ielement){
    //    operator[](ielement).~T();
    //}

    ::free((void*)data);

    *this = dring<T>();
}

template<typename T>
size_t dring<T>::size_in_bytes(){
    return size * sizeof(T);
}

template<typename T>
size_t dring<T>::capacity_in_bytes(){
    return capacity * sizeof(T);
}



    void t_dring(){
        bool success = true;

        dring<u32> ring;

        ring.push_back(1u);
        success &= (ring.data && ring.size == 1u && ring.capacity == 2u && ring.head_index == 0u);
        success &= (ring[0] == 1u);

        ring.push_front(2u);
        success &= (ring.data && ring.size == 2u && ring.capacity == 2u && ring.head_index == 1u);
        success &= (ring[0] == 2u);
        success &= (ring[1] == 1u);

        ring.pop_back();
        success &= (ring.data && ring.size == 1u && ring.capacity == 2u && ring.head_index == 1u);
        success &= (ring[0] == 2u);

        ring.set_min_capacity(3u);
        success &= (ring.data && ring.size == 1u && ring.capacity == 3u && ring.head_index == 1u);
        success &= (ring[0] == 2u);

        ring.push_back(3u);
        ring.push_back(4u);
        success &= (ring.data && ring.size == 3u && ring.capacity == 3u && ring.head_index == 1u);
        success &= (ring[0] == 2u);
        success &= (ring[1] == 3u);
        success &= (ring[2] == 4u);

        ring.push_back(5u);
        success &= (ring.data && ring.size == 4u && ring.capacity == 6u && ring.head_index == 4u);
        success &= (ring[0] == 2u);
        success &= (ring[1] == 3u);
        success &= (ring[2] == 4u);
        success &= (ring[3] == 5u);

        ring.push_front(6u);
        ring.push_front(7u);
        success &= (ring.data && ring.size == 6u && ring.capacity == 6u && ring.head_index == 2u);
        success &= (ring[0] == 7u);
        success &= (ring[1] == 6u);
        success &= (ring[2] == 2u);
        success &= (ring[3] == 3u);
        success &= (ring[4] == 4u);
        success &= (ring[5] == 5u);

        ring.push_front(8u);
        success &= (ring.data && ring.size == 7u && ring.capacity == 12u && ring.head_index == 7u);
        success &= (ring[0] == 8u);
        success &= (ring[1] == 7u);
        success &= (ring[2] == 6u);
        success &= (ring[3] == 2u);
        success &= (ring[4] == 3u);
        success &= (ring[5] == 4u);
        success &= (ring[6] == 5u);

        ring.clear();
        success &= (ring.data && ring.size == 0u && ring.capacity == 12u && ring.head_index == 0u);

        ring.free();
        success &= (ring.data == nullptr && ring.size == 0u && ring.capacity == 0u && ring.head_index == 0u);

        if(!success){
            LOG_ERROR("utest::t_dring() FAILED");
        }else{
            LOG_INFO("utest::t_dring() SUCCESS");
        }
    }

// ---- daryheap : mindheap, maxdheap

// NOTE(hugo): the highest priority element is stored at the top such as PRIORITY_PARENT >= PRIORITY_CHILD
//
// compare(PARENT, CHILD) is
//  < 0 when PARENT has lower  priority than CHILD ie permutation REQUIRED
//  = 0 when PARENT has equal  priority with CHILD ie permutation NOT REQUIRED
//  > 0 when PARENT has higher priority than CHILD ie permutation NOT REQUIRED
//
// ex : a max-heap such as PARENT has higher priority than CHILD means compare(PARENT, CHILD) = comparison_increasing_order
// ex : a min-heap such as PARENT has lower  priority than CHILD means compare(PARENT, CHILD) = comparison_decreasing_order
template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
struct daryheap{
    static_assert(D > 1u);

    u32 push(const T& element);
    void pop(u32 index);

    T& get_top();
    void pop_top();

    void clear();
    void free();

    // ----

    u32 size = 0u;
    u32 capacity = 0u;
    T* data = nullptr;
};

template<typename T, u32 D = 4u>
using maxdheap = daryheap<D, T, &BEEWAX_INTERNAL::comparison_increasing_order>;

template<typename T, u32 D = 4u>
using mindheap = daryheap<D, T, &BEEWAX_INTERNAL::comparison_decreasing_order>;

// ---- priodheap : minpriodheap, maxpriodheap

namespace BEEWAX_INTERNAL{
    template<typename PriorityType, typename DataType>
    struct priodheap_pair{
        PriorityType priority;
        DataType data;
    };

    template<typename PriorityType, typename DataType, s32 (*function)(const PriorityType& A, const PriorityType& B)>
    s32 priodheap_pair_comparison_wrapper(
            const priodheap_pair<PriorityType, DataType>& A,
            const priodheap_pair<PriorityType, DataType>& B){
        return function(A.priority, B.priority);
    }
}

template<u32 D, typename PriorityType, typename DataType, s32 (*compare)(const PriorityType& A, const PriorityType& B)>
using dprio = daryheap<D, BEEWAX_INTERNAL::priodheap_pair<PriorityType, DataType>, &BEEWAX_INTERNAL::priodheap_pair_comparison_wrapper<PriorityType, DataType, compare>>;

template<typename PriorityType, typename DataType, u32 D = 4u>
using mindprio = dprio<D, PriorityType, DataType, &BEEWAX_INTERNAL::comparison_increasing_order>;

template<typename PriorityType, typename DataType, u32 D = 4u>
using maxdprio = dprio<D, PriorityType, DataType, &BEEWAX_INTERNAL::comparison_increasing_order>;

// ---- daryheap

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
static inline void daryheap_reallocate_to_capacity(daryheap<D, T, compare>* heap){
    // NOTE(hugo): realloc may reallocate & memcpy even if the requested size is the same
    void* new_ptr = realloc(heap->data, sizeof(T) * heap->capacity);
    if(new_ptr){
        heap->data = (T*)new_ptr;
    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory but subsequent write may be out of bound");
    }
}

// NOTE(hugo): !WARNING! underflow when /index/ = 0u ie user code has to work with that
template<u32 D>
static inline u32 daryheap_get_parent(u32 index){
    return (index - 1u) / D;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
u32 daryheap<D, T, compare>::push(const T& element){
    if(size == capacity){
        capacity = 2u * max(1u, capacity);
        daryheap_reallocate_to_capacity(this);
    }

    u32 index_current = size;

    // NOTE(hugo): /element/ (as a CHILD) may have higher priority than some of its PARENTs
    // ie swap each invalid PARENT down until the heap condition is verified
    // NOTE(hugo): /index_current/ != 0u ensures that /index_parent/ did not underflow in daryheap_get_parent
    // NOTE(hugo): using the comma operator outputs smaller assembly
    u32 index_parent;
    while(index_current != 0u
    && (index_parent = daryheap_get_parent<D>(index_current), (compare(data[index_parent], element) < 0))){

        data[index_current] = data[index_parent];
        index_current = index_parent;
    }

    data[index_current] = element;
    ++size;

    return index_current;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
static inline u32 daryheap_get_highest_priority_child(daryheap<D, T, compare>* heap, u32 index_start){
    static_assert(D != 0u && D != 1u);

    // TODO(hugo): compare performances with a switch(number of children to check) at runtime to replace min()

    u32 max_index = heap->size - 1u;
    if constexpr (D == 2u){
        u32 index_next = min(index_start + 1u, max_index);
        return index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) < 0);

    }else if constexpr (D == 3u){
        u32 index_next = min(index_start + 1u, max_index);
        u32 temp_max = index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) < 0);

        index_next = min(index_start + 2u, max_index);
        return (compare(heap->data[temp_max], heap->data[index_next]) < 0) ? index_next : temp_max;

    }else if constexpr (D == 4u){
        // TODO(hugo): compare with a more pipelining-friendly version that may be faster
        // REF(hugo): https://probablydance.com/2020/08/31/on-modern-hardware-the-min-max-heap-beats-a-binary-heap/
        // current version : min(1, min(2, min(3, 4)))
        // reduced dependency chain version : min(min(1, 2), min(3, 4))

        u32 index_next = min(index_start + 1u, max_index);
        u32 temp_max = index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) < 0);

        index_next = min(index_start + 2u, max_index);
        temp_max = (compare(heap->data[temp_max], heap->data[index_next]) < 0) ? index_next : temp_max;

        index_next = min(index_start + 3u, max_index);
        return (compare(heap->data[temp_max], heap->data[index_next]) < 0) ? index_next : temp_max;

    }else{
        u32 index_end = min(index_start + D, heap->size);
        u32 temp_max = index_start;

        for(u32 index_current = index_start + 1u; index_current < index_end; ++index_current){
            temp_max = (compare(heap->data[temp_max], heap->data[index_current]) < 0) ? index_current : temp_max;
        }

        return temp_max;
    }
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::pop(u32 index){
    // NOTE(hugo): implies that size != 0
    assert(index < size);

    T element = data[index];

    u32 index_current = index;

    // NOTE(hugo): case 1 : /element/ (as a CHILD) has higher priority than some of its PARENTs
    // ie swap each invalid PARENT down until the heap condition is verified
    // NOTE(hugo): /index_current/ != 0u ensures that /index_parent/ did not underflow in daryheap_get_parent
    // NOTE(hugo): using the comma operator outputs smaller assembly
    u32 index_parent;
    while(index_current != 0u
    && (index_parent = daryheap_get_parent<D>(index_current), (compare(data[index_parent], element) < 0))){

        data[index_current] = data[index_parent];
        index_current = index_parent;
    }

    // NOTE(hugo): case 2 : /element/ (as a PARENT) has lower priority than its highest priority CHILD
    // ie swap each invalid CHILD up until the heap condition is verified
    // NOTE(hugo): using the comma operator outputs smaller assembly
    u32 index_start_children;
    u32 index_comp_child;
    while((index_start_children = index_current * D + 1u, index_start_children < size)
    && (index_comp_child = daryheap_get_highest_priority_child<D, T, compare>(this, index_start_children), compare(element, data[index_comp_child]) < 0)){

        data[index_current] = data[index_comp_child];
        index_current = index_comp_child;
    }

    data[index_current] = element;
    --size;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
T& daryheap<D, T, compare>::get_top(){
    return data[0u];
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::pop_top(){
    pop(0u);
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::clear(){
    //for(u32 ielement = 0u; ielement != size; ++ielement){
    //    data[ielement].~T();
    //}

    size = 0u;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::free(){
    //for(u32 ielement = 0u; ielement != size; ++ielement){
    //    data[ielement].~T();
    //}

    ::free((void*)data);

    *this = daryheap<D, T, compare>();
}

    template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
    bool is_daryheap_valid(const daryheap<D, T, compare>& heap){
        bool success = true;

        for(u32 ielement = 1u; ielement < heap.size; ++ielement){
            u32 parent_index = daryheap_get_parent<D>(ielement);
            success = success && !(compare(heap.data[parent_index], heap.data[ielement]) < 0);
        }

        return success;
    }

    void t_daryheap(){
        bool success = true;

        constexpr u32 ntest = 100u;
        constexpr u32 heap_size = 1000u;

        seed_random_with_time();
        u64 seed_copy = BEEWAX_INTERNAL::seed.seed64;

#define t_daryheap_validation(HEAP)                                     \
        for(u32 itest = 0u; itest != ntest; ++itest){                   \
            for(u32 inumber = 0u; inumber != heap_size; ++inumber)      \
                HEAP.push(random_s32());                                \
            success = success && is_daryheap_valid(HEAP);               \
            for(u32 inumber = 0u; inumber != heap_size / 2u; ++inumber) \
                HEAP.pop(random_u32_range_uniform(HEAP.size));          \
            success = success && is_daryheap_valid(HEAP);               \
            HEAP.clear();                                               \
        }

        {
            maxdheap<s32, 2u> heap;
            t_daryheap_validation(heap);
        }
        {
            maxdheap<s32, 3u> heap;
            t_daryheap_validation(heap);
        }
        {
            maxdheap<s32, 4u> heap;
            t_daryheap_validation(heap);
        }
        {
            maxdheap<s32, 5u> heap;
            t_daryheap_validation(heap);
        }
        {
            maxdheap<s32, 6u> heap;
            t_daryheap_validation(heap);
        }
        {
            mindheap<s32, 2u> heap;
            t_daryheap_validation(heap);
        }

#undef t_daryheap_validation

        if(!success){
            LOG_ERROR("utest::t_daryheap() FAILED");
            LOG_TRACE("utest::t_daryheap seed: %" PRId64, seed_copy);
        }else{
            LOG_INFO("utest::t_daryheap() SUCCESS");
        }
    }

    void t_diterpool(){
        bool success = true;

        diterpool<u32> pool;
        success &= (pool.capacity == 0 && pool.memory == nullptr && pool.available_element == diterpool_no_element_available);

        pool.set_min_capacity(1u);
        success &= (pool.capacity == 1u && pool.available_element == 0u);
        success &= (pool.get_first() == pool.capacity);

        pool.insert(0u);
        success &= (pool.capacity == 1u && pool.available_element == diterpool_no_element_available && pool.get_first() == 0u);

        pool.insert(1u);
        success &= (pool.capacity == 2u && pool.available_element == diterpool_no_element_available);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 0u && pool[iterator] == 0u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.insert(2u);
        success &= (pool.capacity == 4u && pool.available_element == 3u);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 0u && pool[iterator] == 0u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 2u && pool[iterator] == 2u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.insert(3u);
        success &= (pool.capacity == 4u && pool.available_element == diterpool_no_element_available);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 0u && pool[iterator] == 0u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 2u && pool[iterator] == 2u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 3u && pool[iterator] == 3u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.remove(2u);
        success &= (pool.capacity == 4u && pool.available_element == 2u);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 0u && pool[iterator] == 0u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 3u && pool[iterator] == 3u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.remove(0u);
        success &= (pool.capacity == 4u && pool.available_element == 0u);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 3u && pool[iterator] == 3u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.insert(4u);
        success &= (pool.capacity == 4u && pool.available_element == 2u);
        {
            u32 iterator = pool.get_first();
            success &= (iterator == 0u && pool[iterator] == 4u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 1u && pool[iterator] == 1u);

            iterator = pool.get_next(iterator);
            success &= (iterator == 3u && pool[iterator] == 3u);

            iterator = pool.get_next(iterator);
            success &= (iterator == pool.capacity);
        }

        pool.clear();
        success &= (pool.capacity == 4u && pool.available_element == 0u && pool.get_first() == pool.capacity);

        pool.insert(5u);

        pool.free();

        pool.set_min_capacity(5);
        success &= (pool.capacity == 5u && pool.available_element == 0u && pool.get_first() == pool.capacity);

        pool.free();

        if(!success){
            LOG_ERROR("utest::t_dpool() FAILED");
        }else{
            LOG_INFO("utest::t_dpool() SUCCESS");
        }
    }

// ---- bitmaparray

// NOTE(hugo): bitmaparray
// - iteration on active data
// - relaxed packing of active data
// - verification of the status (active / inactive) at a given index
// - managed as a pool

template<typename T>
size_t bitmaparray_bytesize(u32 capacity);

template<typename T>
struct bitmaparray {
    // NOTE(hugo): require that the adress at the end of the bitset is usable for every T
    static_assert(alignof(u64) >= alignof(T));
    static_assert(sizeof(T) >= sizeof(u32));

    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    // NOTE(hugo): use get_next when removing elements while iterating
    /*
        u32 index = UINT_MAX, counter = 0u;
        while(counter < array.size && (index = array.get_next(index), true)){

            *some code*

            if(remove){
                array.remove(index);
            }else{
                ++counter;
            }
        }
    */
    u32 get_next(u32 index) const;

    u32 insert_empty();
    u32 insert(const T& value);

    void remove(u32 index);

    void clear();

    // ---- internal

    bool is_active(u32 index) const;
    void make_active(u32 index);
    void make_inactive(u32 index);

    // ---- data

    union bucket{
        T type;
        u32 next;
    };

    u32 size = 0u;
    u32 capacity = 0u;
    u64* bitset_memory = nullptr;
    bucket* bucket_memory = nullptr;

    u32 head_bucket = bitmaparray_no_bucket;
};

template<typename T>
bitmaparray<T> bitmaparray_in_memory(void* memory, u32 capacity);

template<typename T>
bitmaparray<T> bitmaparray_grow_in_memory(const bitmaparray<T>& previous, void* memory, u32 new_capacity);

template<typename T>
struct dbitmaparray : bitmaparray<T>{
    u32 insert_empty();
    u32 insert(const T& value);

    void set_min_capacity(u32 new_capacity);

    void free();
};

template<typename T>
struct bitmaparray_iterator{
    bitmaparray_iterator& operator++();
    bool operator!=(const bitmaparray_iterator<T>& rhs) const;
    T& operator*();
    const T& operator*() const;

    // ---- data

    const bitmaparray<T>* container = nullptr;
    u32 counter = 0u;
    u32 index = 0u;
};

template<typename T>
bitmaparray_iterator<T> begin(const bitmaparray<T>& a);
template<typename T>
bitmaparray_iterator<T> end(const bitmaparray<T>& a);

// ---- dhashmap

template<typename K, typename T>
struct dhashmap{

    // NOTE(hugo): returned pointers are temporary references ie may change after any other operation
    T* get(const K& key, bool& was_entry_created);
    T* search(const K& key);
    void remove(const K& key);

    void set_min_capacity(u32 min_capacity);

    void clear();
    void free();

    // ---- internal / data

    struct bucket{
        K key;
        T value;
        u32 next;
    };

    u32* table = nullptr;
    u32 table_capacity_minus_one = 0u;
    bitmaparray<bucket> storage;
};

// ---- bitmaparray

static u32 bitmaparray_bitset_size(u32 capacity){
    u32 bitmask_63 = 0x3F;
    return capacity / 64u + (u32)((capacity & bitmask_63) != 0u);
}

template<typename T>
size_t bitmaparray_bytesize(u32 capacity){
    size_t bitset_bytesize = (size_t)bitmaparray_bitset_size(capacity) * sizeof(u64);
    size_t data_bytesize = capacity * sizeof(typename bitmaparray<T>::bucket);
    return bitset_bytesize + data_bytesize;
}

template<typename T>
static void dbitmaparray_grow_to_capacity(bitmaparray<T>& ha, u32 new_capacity){
    assert(new_capacity > ha.capacity);

    // NOTE(hugo): not using realloc here because we already need to memcpy the data
    void* new_data = malloc(bitmaparray_bytesize<T>(new_capacity));
    assert(new_data);

    u32 previous_bitset_size = bitmaparray_bitset_size(ha.capacity);
    u32 new_bitset_size = bitmaparray_bitset_size(new_capacity);

    u64* new_bitset_memory = (u64*)new_data;
    typename bitmaparray<T>::bucket* new_bucket_memory = (typename bitmaparray<T>::bucket*)(new_bitset_memory + new_bitset_size);

    memcpy(new_bitset_memory, ha.bitset_memory, previous_bitset_size * sizeof(u64));
    memset(new_bitset_memory + previous_bitset_size, 0u, (new_bitset_size - previous_bitset_size) * sizeof(u64));
    memcpy(new_bucket_memory, ha.bucket_memory, ha.capacity * sizeof(typename bitmaparray<T>::bucket));

    // NOTE(hugo): go to the end of the previous linked list
    // !using previous memory as it's better for pipelining!
    u32 end_index = ha.capacity;
    u32 next = ha.head_bucket;
    while(next != bitmaparray_no_bucket){
        end_index = next;
        next = ha.bucket_memory[end_index].next;
    }

    // NOTE(hugo): link the new buckets
    // !in the new memory!
    new_bucket_memory[end_index].next = ha.capacity;
    for(u32 ibucket = ha.capacity; ibucket < new_capacity - 1u; ++ibucket){
        new_bucket_memory[ibucket].next = ibucket + 1u;
    }
    new_bucket_memory[new_capacity - 1u].next = bitmaparray_no_bucket;

    ::free((void*)ha.bitset_memory);

    ha.head_bucket = min(ha.head_bucket, ha.capacity);
    ha.capacity = new_capacity;
    ha.bitset_memory = new_bitset_memory;
    ha.bucket_memory = new_bucket_memory;
}

template<typename T>
T& bitmaparray<T>::operator[](u32 index){
    assert(index < capacity && is_active(index));
    return bucket_memory[index].type;
}

template<typename T>
const T& bitmaparray<T>::operator[](u32 index) const{
    assert(index < capacity && is_active(index));
    return bucket_memory[index].type;
}

template<typename T>
u32 bitmaparray<T>::get_next(u32 index) const{
    index = index + 1u;
    while(index < capacity && !is_active(index)) ++index;
    return index;
}

template<typename T>
u32 bitmaparray<T>::insert_empty(){
    assert(size != capacity);

    u32 output_index = head_bucket;
    head_bucket = bucket_memory[output_index].next;

    assert(!is_active(output_index));
    make_active(output_index);
    new((void*)&bucket_memory[output_index].type) T{};

    ++size;
    return output_index;
}

template<typename T>
u32 dbitmaparray<T>::insert_empty(){
    if(bitmaparray<T>::size == bitmaparray<T>::capacity){
        u32 new_capacity = 2u * max(min_dbitmaparray_capacity / 2u, bitmaparray<T>::capacity);
        dbitmaparray_grow_to_capacity<T>(*this, new_capacity);
    }
    return bitmaparray<T>::insert_empty();
}

template<typename T>
u32 bitmaparray<T>::insert(const T& value){
    u32 index = insert_empty();
    bucket_memory[index].type = value;
    return index;
}

template<typename T>
u32 dbitmaparray<T>::insert(const T& value){
    u32 index = insert_empty();
    bitmaparray<T>::bucket_memory[index].type = value;
    return index;
}

template<typename T>
void bitmaparray<T>::remove(u32 index){
    assert(is_active(index));
    make_inactive(index);

    // NOTE(hugo): keep the linked list sorted by index
    if(head_bucket > index){
        bucket_memory[index].next = head_bucket;
        head_bucket = index;
    }else{
        u32 bucket_index = head_bucket;
        while(bucket_memory[bucket_index].next < index){
            bucket_index = bucket_memory[bucket_index].next;
        }
        bucket_memory[index].next = bucket_memory[bucket_index].next;
        bucket_memory[bucket_index].next = index;
    }

    --size;
}

template<typename T>
void dbitmaparray<T>::set_min_capacity(u32 new_capacity){
    if(new_capacity > bitmaparray<T>::capacity){
        dbitmaparray_grow_to_capacity<T>(*this, new_capacity);
    }
}

template<typename T>
void bitmaparray<T>::clear(){
    assert(capacity > 0u);

    size = 0u;
    head_bucket = 0u;

    u32 bitset_size = bitmaparray_bitset_size(capacity);
    memset(bitset_memory, 0u, bitset_size * sizeof(u64));

    // NOTE(hugo): link the buckets
    for(u32 bucket_index = 0u; bucket_index < capacity - 1u; ++bucket_index){
        bucket_memory[bucket_index].next = bucket_index + 1u;
    }
    bucket_memory[capacity - 1u].next = bitmaparray_no_bucket;
}

template<typename T>
void dbitmaparray<T>::free(){
    ::free((void*)bitmaparray<T>::bitset_memory);

    *this = dbitmaparray<T>();
}

template<typename T>
bool bitmaparray<T>::is_active(u32 identifier) const{
    u32 bitset_index = identifier / 64u;
    u32 bit_index = identifier - bitset_index * 64u;

    return extract_bit(bitset_memory[bitset_index], bit_index) != 0u;
}

template<typename T>
void bitmaparray<T>::make_active(u32 identifier){
    u32 bitset_index = identifier / 64u;
    u32 bit_index = identifier - bitset_index * 64u;

    set_bit(bitset_memory[bitset_index], bit_index);
}

template<typename T>
void bitmaparray<T>::make_inactive(u32 identifier){
    u32 bitset_index = identifier / 64u;
    u32 bit_index = identifier - bitset_index * 64u;

    unset_bit(bitset_memory[bitset_index], bit_index);
}

template<typename T>
bitmaparray<T> bitmaparray_in_memory(void* memory, u32 capacity){
    assert(capacity > 0u && is_aligned(memory, alignof(u64)));

    u32 bitset_size = bitmaparray_bitset_size(capacity);

    bitmaparray<T> output;
    output.size = 0u;
    output.capacity = capacity;
    output.bitset_memory = memory;
    output.bucket_memory = output.bitset_memory + bitset_size;
    output.head_bucket = 0u;

    memset(output.bitset_memory, 0u, bitset_size * sizeof(u64));

    // NOTE(hugo): link the buckets
    for(u32 bucket_index = 0u; bucket_index < output.capacity - 1u; ++bucket_index){
        output.bucket_memory[bucket_index].next_element = bucket_index + 1u;
    }
    output.bucket_memory[output.capacity - 1u].next_element = bitmaparray_no_bucket;

    return output;
}

template<typename T>
bitmaparray<T> bitmaparray_grow_in_memory(const bitmaparray<T>& previous, void* memory, u32 new_capacity){
    assert(new_capacity > previous.capacity && is_aligned(memory, alignof(u64)));

    u32 previous_bitset_size = bitmaparray_bitset_size(previous.capacity);
    u32 bitset_size = bitmaparray_bitset_size(new_capacity);

    bitmaparray<T> output;
    output.size = previous.size;
    output.capacity = new_capacity;
    output.bitset_memory = (u64*)memory;
    output.bucket_memory = (typename bitmaparray<T>::bucket*)(output.bitset_memory + bitset_size);
    output.head_bucket = min(previous.head_bucket, previous.capacity);

    memcpy(output.bitset_memory, previous.bitset_memory, previous_bitset_size * sizeof(u64));
    memset(output.bitset_memory + previous_bitset_size, 0u, (bitset_size - previous_bitset_size) * sizeof(u64));
    memcpy(output.bucket_memory, previous.bucket_memory, previous.capacity * sizeof(typename bitmaparray<T>::bucket));

    // NOTE(hugo): go to the end of the previous linked list
    u32 end_index = previous.capacity;
    u32 next = previous.head_bucket;
    while(next != bitmaparray_no_bucket){
        end_index = next;
        next = output.bucket_memory[end_index].next;
    }

    // NOTE(hugo): link the new buckets
    output.bucket_memory[end_index].next = previous.capacity;
    for(u32 ibucket = previous.capacity; ibucket < output.capacity - 1u; ++ibucket){
        output.bucket_memory[ibucket].next = ibucket + 1u;
    }
    output.bucket_memory[output.capacity - 1u].next = bitmaparray_no_bucket;

    return output;
}

template<typename T>
bitmaparray_iterator<T>& bitmaparray_iterator<T>::operator++(){
    ++counter;
    return *this;
}

template<typename T>
bool bitmaparray_iterator<T>::operator!=(const bitmaparray_iterator<T>& rhs) const{
    auto non_const_this = const_cast<bitmaparray_iterator<T>*>(this);
    return counter != rhs.counter && (non_const_this->index = container->get_next(index), true);
}

template<typename T>
T& bitmaparray_iterator<T>::operator*(){
    return (*const_cast<bitmaparray<T>*>(container))[index];
}

template<typename T>
const T& bitmaparray_iterator<T>::operator*() const{
    return (*container)[index];
}

template<typename T>
bitmaparray_iterator<T> begin(const bitmaparray<T>& a){
    bitmaparray_iterator<T> iter;
    iter.container = &a;
    iter.counter = 0u;
    iter.index = UINT_MAX;
    return iter;
}

template<typename T>
bitmaparray_iterator<T> end(const bitmaparray<T>& a){
    bitmaparray_iterator<T> iter;
    iter.counter = a.size;
    return iter;
}

// ---- dhashmap

template<typename K>
static u32 dhashmap_hash_key(const K& key){
    return FNV1a_32ptr((uchar*)&key, sizeof(K));
}
static u32 dhashmap_hash_key(const u32& key){
    return wang_hash(key);
}
static u32 dhashmap_hash_key(const s32& key){
    return wang_hash(*(u32*)&key);
}

template<typename K, typename T>
void dhashmap_grow_to_storage_capacity(dhashmap<K, T>& dhm, u32 new_capacity){
    assert(new_capacity > dhm.storage.capacity && is_pow2(new_capacity));

    u32 table_capacity = 2u * new_capacity;
    u32 table_capacity_minus_one = table_capacity - 1u;

    // NOTE(hugo): determine the bytesize for the table & bitmaparray
    size_t table_bytesize = table_capacity * sizeof(u32);
    size_t table_to_bitmaparray = align_offset_next(table_bytesize, alignof(u64));
    size_t storage_bytesize = bitmaparray_bytesize<typename dhashmap<K, T>::bucket>(new_capacity);
    size_t required_bytesize = table_bytesize + table_to_bitmaparray + storage_bytesize;

    void* to_free = dhm.table;

    // NOTE(hugo): not using realloc here because we already need to memcpy the data
    void* memory = malloc(required_bytesize);
    assert(memory);

    // NOTE(hugo): setup the table with 0xFF = UINT32_MAX = dhashmap_no_bucket
    dhm.table = (u32*)memory;
    dhm.table_capacity_minus_one = table_capacity_minus_one;
    memset(dhm.table, 0xFF, sizeof(u32) * table_capacity);

    // NOTE(hugo): setup the storage
    void* bitmaparray_memory = (u8*)memory + table_bytesize + table_to_bitmaparray;
    dhm.storage = bitmaparray_grow_in_memory(dhm.storage, bitmaparray_memory, new_capacity);

    // NOTE(hugo): rehash current storage
    u32 index = UINT_MAX, counter = 0u;
    while(counter < dhm.storage.size && (index = dhm.storage.get_next(index), true)){
        typename dhashmap<K, T>::bucket& b = dhm.storage[index];

        u32 hash = dhashmap_hash_key(b.key);
        u32 table_index = hash & dhm.table_capacity_minus_one;

        b.next = dhm.table[table_index];
        dhm.table[table_index] = index;

        ++counter;
    }

    ::free(to_free);
}

template<typename K, typename T>
void dhashmap<K, T>::set_min_capacity(u32 min_capacity){
    u32 new_storage_capacity = next_pow2(min_capacity);
    if(new_storage_capacity > storage.capacity){
        dhashmap_grow_to_storage_capacity(*this, new_storage_capacity);
    }
}

// NOTE(hugo): search for an already existing entry in the table
#define DHASHMAP_GET_SEARCH_COMMON                      \
    u32* next_ptr = table + table_index;                \
    u32 next = *next_ptr;                               \
    while(next != dhashmap_no_bucket){                  \
        if(storage[next].key == key){                   \
            return &storage[next].value;                \
        }                                               \
        next_ptr = &storage[next].next;                 \
        next = *next_ptr;                               \
    }

template<typename K, typename T>
T* dhashmap<K, T>::get(const K& key, bool& was_entry_created){
    u32 hash = dhashmap_hash_key(key);
    u32 table_index = hash & table_capacity_minus_one;
    was_entry_created = false;

    if(table != nullptr){

        DHASHMAP_GET_SEARCH_COMMON

    }

    // NOTE(hugo): reallocate when needed
    if(table == nullptr || storage.size == storage.capacity){
        u32 new_storage_capacity = 2u * max(min_dhashmap_storage_capacity / 2u, storage.capacity);
        dhashmap_grow_to_storage_capacity(*this, new_storage_capacity);

        // NOTE(hugo): rehashing the element to insert
        table_index = hash & table_capacity_minus_one;
    }

    // NOTE(hugo): create a new entry
    u32 new_index = storage.insert_empty();
    storage[new_index].key = key;
    storage[new_index].next = table[table_index];
    table[table_index] = new_index;
    was_entry_created = true;

    return &storage[new_index].value;
}

template<typename K, typename T>
T* dhashmap<K, T>::search(const K& key){
    if(table != nullptr){
        u32 hash = dhashmap_hash_key(key);
        u32 table_index = hash & table_capacity_minus_one;

        DHASHMAP_GET_SEARCH_COMMON
    }

    return nullptr;
}
#undef DHASHMAP_GET_SEARCH_COMMON

template<typename K, typename T>
void dhashmap<K, T>::remove(const K& key){
    if(table != nullptr){
        u32 hash = dhashmap_hash_key(key);
        u32 table_index = hash & table_capacity_minus_one;

        // NOTE(hugo): search for an existing entry
        u32* next_ptr = table + table_index;
        u32 next = *next_ptr;
        while(next != dhashmap_no_bucket){
            if(storage[next].key == key){
                *next_ptr = storage[next].next;
                storage.remove(next);
                break;
            }
            next_ptr = &storage[next].next;
            next = *next_ptr;
        }
    }
}

template<typename K, typename T>
void dhashmap<K, T>::clear(){
    // NOTE(hugo): 0xFF = UINT32_MAX = dhashmap_no_bucket
    memset(table, 0xFF, sizeof(u32) * (table_capacity_minus_one + 1u));
    storage.clear();
}

template<typename K, typename T>
void dhashmap<K, T>::free(){
    ::free(table);
    *this = dhashmap<K, T>();
}

// ---- hashmap


// ---- hashmap
// - user-provided identifiers
// - get, search & remove are o(1)
// - iterable

template<typename Key_Type>
u32 hashmap_hash_key(const Key_Type& key);
u32 hashmap_hash_key(const u32& key);
u32 hashmap_hash_key(const s32& key);

// NOTE(hugo): /Storage_Type/ must be stable in memory for the pointers to work
template<typename Key_Type, typename Data_Type,
    template<typename T> typename Table_Storage_Type = contiguous_storage,
    template<typename T> typename Data_Storage_Type = bucketized_storage_32>
struct hashmap{
    Data_Type* get(const Key_Type& key, bool& was_entry_created);
    Data_Type* search(const Key_Type& key);
    void remove(const Key_Type& key);

    void set_min_capacity(u32 min_capacity);

    void clear();
    void free();

    // ---- iterator

    auto begin();
    auto end();

    // ---- data

    struct bucket{
        Key_Type key;
        Data_Type value;
        bucket* next;
    };

    // NOTE(hugo): packed data to speed-up rehashing
    u32 table_capacity_minus_one = 0u;
    Table_Storage_Type<bucket*> table;
    array<bucket, Data_Storage_Type> data;
};


// ---- hashmap

template<typename K>
u32 hashmap_hash_key(const K& key){
    return FNV1a_32ptr((uchar*)&key, sizeof(K));
}
u32 hashmap_hash_key(const u32& key){
    return xorshift_hash(key);
}
u32 hashmap_hash_key(const s32& key){
    return xorshift_hash(*(u32*)&key);
}

template<typename Hashmap_Type>
void rehash_data_to_table(Hashmap_Type* hm){
    hm->table.zero(0u, hm->table.capacity);

    // NOTE(hugo): rehash data into the new table
    for(u32 ibucket = 0u; ibucket != hm->data.size; ++ibucket){
        Hashmap_Type::bucket* rehash_bucket = &hm->data[ibucket];

        u32 hash = hashmap_hash_key(rehash_bucket->key);
        u32 table_index = hash & hm->table_capacity_minus_one;

        Hashmap_Type::bucket** insertion_point = &hm->table[table_index];
        Hashmap_Type::bucket* table_bucket = *insertion_point;
        while(table_bucket != nullptr){
            insertion_point = &table_bucket->next;
            table_bucket = table_bucket->next;
        }

        *insertion_point = rehash_bucket;
    }
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
Data_Type* hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::get(const Key_Type& key, bool& was_entry_created){
    u32 hash = hashmap_hash_key(key);

    u32 table_index;
    bucket** insertion_point;
    bucket* search_bucket;

    // NOTE(hugo): search when necessary
    if(table.capacity != 0u){
        table_index = hash & table_capacity_minus_one;
        insertion_point = &table[table_index];
        search_bucket = *insertion_point;

        while(search_bucket != nullptr){
            if(search_bucket->key == key){
                was_entry_created = false;
                return &search_bucket->value;
            }

            insertion_point = &search_bucket->next;
            search_bucket = search_bucket->next;
        }
    }

    // NOTE(hugo): insert
    if(data.size == data.storage.capacity){
        // NOTE(hugo): extend data
        data.storage.increase_capacity();

        // NOTE(hugo): extend table
        u32 required_table_capacity = round_up_pow2(data.storage.capacity + data.storage.capacity / 3u);
        if(required_table_capacity > table_capacity_minus_one + 1u){
            if(required_table_capacity > table.capacity){
                table.increase_capacity_min(required_table_capacity);
            }

            table.zero(0u, required_table_capacity);
            table_capacity_minus_one = required_table_capacity - 1u;

            rehash_data_to_table(this);
        }

        // NOTE(hugo): search the new insertion point
        table_index = hash & table_capacity_minus_one;
        insertion_point = &table[table_index];
        search_bucket = *insertion_point;

        while(search_bucket != nullptr){
            insertion_point = &search_bucket->next;
            search_bucket = search_bucket->next;
        }
    }

    u32 new_bucket_index = data.push_empty();

    bucket* new_bucket = &data[new_bucket_index];
    *insertion_point = new_bucket;
    new_bucket->key = key;
    new_bucket->next = nullptr;

    was_entry_created = true;
    return &new_bucket->value;
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
Data_Type* hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::search(const Key_Type& key){
    if(data.size != 0u){
        u32 hash = hashmap_hash_key(key);
        u32 table_index = hash & table_capacity_minus_one;

        bucket* search_bucket = table[table_index];
        while(search_bucket != nullptr){
            if(search_bucket->key == key){
                return &search_bucket->value;
            }

            search_bucket = search_bucket->next;
        }
    }

    return nullptr;
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
void hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::remove(const Key_Type& key){
    if(data.size != 0u){
        // NOTE(hugo): search the bucket to remove
        u32 remove_hash = hashmap_hash_key(key);
        u32 remove_table_index = remove_hash & table_capacity_minus_one;

        bucket** to_remove_bucket = &table[remove_table_index];
        bucket* remove_bucket = *to_remove_bucket;
        while(remove_bucket != nullptr){

            if(remove_bucket->key == key){
                // NOTE(hugo): skip the remove bucket
                *to_remove_bucket = remove_bucket->next;

                bucket* swap_bucket = &data[data.size - 1u];

                if(swap_bucket != remove_bucket){
                    // NOTE(hugo): search the bucket to swap
                    u32 swap_hash = hashmap_hash_key(swap_bucket->key);
                    u32 swap_table_index = swap_hash & table_capacity_minus_one;

                    bucket** to_swap_bucket = &table[swap_table_index];
                    bucket* temp = *to_swap_bucket;
                    while(temp != swap_bucket){
                        to_swap_bucket = &temp->next;
                        temp = temp->next;
                    }

                    // NOTE(hugo): swap and pop
                    *to_swap_bucket = remove_bucket;
                    memcpy(remove_bucket, swap_bucket, sizeof(bucket));
                }

                data.pop();

                return;
            }

            to_remove_bucket = &remove_bucket->next;
            remove_bucket = remove_bucket->next;
        }
    }
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
void hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::set_min_capacity(u32 min_capacity){
    min_capacity = max(32u, min_capacity);

    if(min_capacity > data.storage.capacity){
        // NOTE(hugo): extend data
        data.set_min_capacity(max(32u, min_capacity));

        // NOTE(hugo): extend table
        u32 required_table_capacity = round_up_pow2(data.storage.capacity + data.storage.capacity / 3u);
        if(required_table_capacity > table_capacity_minus_one + 1u){
            if(required_table_capacity > table.capacity){
                table.increase_capacity_min(required_table_capacity);
            }

            table.zero(0u, table.capacity);
            table_capacity_minus_one = required_table_capacity - 1u;

            rehash_data_to_table(this);
        }
    }
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
void hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::clear(){
    table.zero(0u, table.capacity);
    data.clear();
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
void hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::free(){
    table.free();
    data.free();
    *this = hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>();
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
auto hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::begin(){
    return data.begin();
}

template<typename Key_Type, typename Data_Type, template<typename T> typename Table_Storage_Type, template<typename T> typename Data_Storage_Type>
auto hashmap<Key_Type, Data_Type, Table_Storage_Type, Data_Storage_Type>::end(){
    return data.end();
}

    void t_dhashmap(){
        bool success = true;

        hashmap<s32, s32> hmap;
        bool was_created;

        {
            s32* search = hmap.search(1);
            success &= (search == nullptr);
        }

        {
            s32* get = hmap.get(1, was_created);
            success &= (get != nullptr && was_created);
            *get = -1;

            s32* search = hmap.search(1);
            success &= (search == get && *search == -1);

            u32 counter = 0u;
            u32 keys[] = {1u};
            u32 values[] = {-1u};
            for(auto& iter : hmap.data){
                success &= iter.key == keys[counter] && iter.value == values[counter];
                ++counter;
            }

            s32* get_second = hmap.get(1, was_created);
            success &= (get_second == get && *get_second == -1 && !was_created);
        }

        {
            hmap.remove(1);

            s32* search = hmap.search(1);
            success &= (search == nullptr);
        }

        {
            *hmap.get(2, was_created) = -2;
            *hmap.get(3, was_created) = -3;
            *hmap.get(4, was_created) = -4;

            success &= (*hmap.search(2) == -2);
            success &= (*hmap.search(3) == -3);
            success &= (*hmap.search(4) == -4);

            u32 counter = 0u;
            u32 keys[] = {2u, 3u, 4u};
            u32 values[] = {-2u, -3u, -4u};
            for(auto& iter : hmap.data){
                success &= iter.key == keys[counter] && iter.value == values[counter];
                ++counter;
            }
        }

        {
            hmap.remove(3);

            s32* search = hmap.search(2);
            success &= (search && *search == -2);

            search = hmap.search(3);
            success &= (search == nullptr);

            search = hmap.search(4);
            success &= (search && *search == -4);

            u32 counter = 0u;
            u32 keys[] = {2u, 4u};
            u32 values[] = {-2u, -4u};
            for(auto& iter : hmap.data){
                success &= iter.key == keys[counter] && iter.value == values[counter];
                ++counter;
            }
        }

        hmap.free();

        if(!success){
            LOG_ERROR("utest::t_dhashmap() FAILED");
        }else{
            LOG_INFO("utest::t_dhashmap() SUCCESS");
        }
    }

    void t_dhashmap_randomized(){
        bool success = true;

        constexpr u32 ntest = 100u;
        constexpr u32 nvalues = 100u;
        constexpr u32 noperations = 10000u;

        s32 status_counter[nvalues];
        memset(status_counter, 0, nvalues * sizeof(s32));

        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

        hashmap<u32, s32> hmap;
        //hmap.set_min_capacity(256u);

        for(u32 itest = 0u; itest != ntest; ++itest){
            for(u32 ioperation = 0u; ioperation != noperations; ++ioperation){
                u32 random_key = random_u32_range_uniform(nvalues);
                u32 random_operation = random_u32() % 3u;

                switch(random_operation){
                    default:
                    case 0:
                        {
                            bool was_created;
                            s32* hmap_counter = hmap.get(random_key, was_created);
                            //LOG_TRACE("get[%d] %d %d was_created: %d", random_key, *hmap_counter, status_counter[random_key], was_created);
                            success = success && *hmap_counter == status_counter[random_key];
                            ++(*hmap_counter);
                            ++status_counter[random_key];
                            break;
                        }
                    case 1:
                        {
                            s32* hmap_counter = hmap.search(random_key);
                            if(hmap_counter){
                                //LOG_TRACE("search[%d] %d %d", random_key, *hmap_counter, status_counter[random_key]);
                                success = success && status_counter[random_key] > 0 && *hmap_counter == status_counter[random_key];
                                ++(*hmap_counter);
                                ++status_counter[random_key];
                            }else{
                                //LOG_TRACE("search[%d] X %d", random_key, status_counter[random_key]);
                                success = success && status_counter[random_key] == 0;
                            }
                            break;
                        }
                    case 2:
                        {
                            //hmap.remove(random_key);
                            //s32* searched = hmap.search(random_key);
                            //LOG_TRACE("remove[%d] %d", random_key, status_counter[random_key]);
                            //success = success && searched == nullptr;
                            //status_counter[random_key] = 0;
                            break;
                        }
                }
            }

            memset(status_counter, 0, nvalues * sizeof(s32));
            hmap.clear();
        }

        hmap.free();

        if(!success){
            LOG_ERROR("utest::t_dhashmap_randomized() FAILED");
            LOG_TRACE("utest::t_dhashmap_randomized() seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
        }else{
            LOG_INFO("utest::t_dhashmap_randomized() SUCCESS");
        }
    }

// ---- ptr_pool

// NOTE(hugo): /Storage_Type/ must be stable in memory
template<typename T, template<typename T> typename Storage_Type = bucketized_storage_32>
struct ptr_pool{
    T* insert_empty();
    void remove(T* ptr);

    void set_min_capacity(u32 new_capacity);

    void clear();
    void free();

    // ---- data

    union bucket{
        T type;
        bucket* next;
    };

    Storage_Type<bucket> storage;
    u32 size = 0u;
    bucket* head_bucket = nullptr;
};

// ---- ptr_pool

template<typename T, template<typename T> typename Storage_Type>
T* ptr_pool<T, Storage_Type>::insert_empty(){
    if(head_bucket == nullptr){
        u32 previous_capacity = storage.capacity;
        storage.increase_capacity();

        head_bucket = &storage[previous_capacity];
        for(u32 index = previous_capacity; index < storage.capacity - 1u; ++index){
            storage[index].next = &storage[index + 1u];
        }
        storage[storage.capacity - 1u].next = nullptr;
    }

    bucket* insert_bucket = head_bucket;
    head_bucket = head_bucket->next;
    new((void*)&insert_bucket->type) T{};
    ++size;
    return &insert_bucket->type;
}

template<typename T, template<typename T> typename Storage_Type>
void ptr_pool<T, Storage_Type>::remove(T* ptr){
    bucket* remove_bucket = (bucket*)ptr;
    remove_bucket->next = head_bucket;
    head_bucket = remove_bucket;
    --size;
}

template<typename T, template<typename T> typename Storage_Type>
void ptr_pool<T, Storage_Type>::set_min_capacity(u32 new_capacity){
    if(new_capacity > storage.capacity){
        u32 previous_capacity = storage.capacity;
        storage.increase_capacity_min(new_capacity);

        for(u32 index = previous_capacity; index < storage.capacity - 1u; ++index){
            storage[index].next = &storage[index + 1u];
        }
        storage[storage.capacity - 1u].next = head_bucket;
        head_bucket = &storage[previous_capacity];
    }
}

template<typename T, template<typename T> typename Storage_Type>
void ptr_pool<T, Storage_Type>::clear(){
    if(storage.capacity > 0u){
        size = 0u;
        head_bucket = &storage[0u];

        for(u32 index = 0u; index < storage.capacity - 1u; ++index){
            storage[index].next = &storage[index + 1u];
        }
        storage[storage.capacity - 1u].next = nullptr;
    }
}

template<typename T, template<typename T> typename Storage_Type>
void ptr_pool<T, Storage_Type>::free(){
    storage.free();
    *this = ptr_pool<T, Storage_Type>();
}

    void t_ptr_pool(){
        bool success = true;

        ptr_pool<u32, bucketized_storage_32> pool;
        success &= (pool.size == 0u && pool.storage.capacity == 0 && pool.head_bucket == nullptr);

        pool.set_min_capacity(1u);
        success &= (pool.size == 0u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[0]);

        u32* first = pool.insert_empty();
        *first = 0u;
        success &= (pool.size == 1u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[1]);
        success &= (pool.storage[0u].type == 0u);

        u32* second = pool.insert_empty();
        *second = 1u;
        success &= (pool.size == 2u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[2u]);
        {
            success &= (pool.storage[0u].type == 0u);
            success &= (pool.storage[1u].type == 1u);
        }

        u32* third = pool.insert_empty();
        *third = 2u;
        success &= (pool.size == 3u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[3u]);
        {
            success &= (pool.storage[0u].type == 0u);
            success &= (pool.storage[1u].type == 1u);
            success &= (pool.storage[2u].type == 2u);
        }

        u32* fourth = pool.insert_empty();
        *fourth = 3u;
        success &= (pool.size == 4u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[4u]);
        {
            success &= (pool.storage[0u].type == 0u);
            success &= (pool.storage[1u].type == 1u);
            success &= (pool.storage[2u].type == 2u);
            success &= (pool.storage[3u].type == 3u);
        }

        pool.remove(third);
        success &= (pool.size == 3u && pool.storage.capacity == 32u && (void*)pool.head_bucket == (void*)third);
        {
            success &= (pool.storage[0u].type == 0u);
            success &= (pool.storage[1u].type == 1u);
            success &= (pool.storage[3u].type == 3u);
        }

        pool.remove(first);
        success &= (pool.size == 2u && pool.storage.capacity == 32u && (void*)pool.head_bucket == (void*)first);
        {
            success &= (pool.storage[1u].type == 1u);
            success &= (pool.storage[3u].type == 3u);
        }

        u32* fifth = pool.insert_empty();
        *fifth = 4u;
        success &= (fifth == first);
        success &= (pool.size == 3u && pool.storage.capacity == 32u && (void*)pool.head_bucket == (void*)third);
        {
            success &= (pool.storage[0u].type == 4u);
            success &= (pool.storage[1u].type == 1u);
            success &= (pool.storage[3u].type == 3u);
        }

        pool.remove(fourth);
        success &= (pool.size == 2u && pool.storage.capacity == 32u && (void*)pool.head_bucket == (void*)fourth);
        {
            success &= (pool.storage[0u].type == 4u);
            success &= (pool.storage[1u].type == 1u);
        }

        pool.clear();
        success &= (pool.size == 0u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[0u]);

        u32* sixth = pool.insert_empty();
        *sixth = 5u;
        success &= (pool.size == 1u && pool.storage.capacity == 32u && pool.head_bucket == &pool.storage[1u]);

        pool.free();
        success &= (pool.size == 0u && pool.storage.capacity == 0u && pool.head_bucket == nullptr);

        pool.set_min_capacity(33u);
        success &= (pool.size == 0u && pool.storage.capacity == 64u && pool.head_bucket == &pool.storage[0u]);

        pool.free();

        if(!success){
            LOG_ERROR("utest::t_ptr_pool() FAILED");
        }else{
            LOG_INFO("utest::t_ptr_pool() SUCCESS");
        }

    }

    bw::utest::t_ptr_pool();
