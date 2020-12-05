#ifndef H_DATA_STRUCTURE
#define H_DATA_STRUCTURE

template<typename T>
T* new_struct();

// ---- buffer

template<typename T>
struct buffer{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    // ---- data

    void* data = nullptr;
    u32 size = 0u;
};

// ---- darena

struct darena{
    void reserve(size_t arena_bytesize);
    void free();

    // NOTE(hugo): memory is not initialized
    void* push(size_t bytesize, size_t alignment);

    // NOTE(hugo): elements are initialized
    template<typename T>
    void* push();

    // NOTE(hugo): extensions are freed and base memory is extended to fit them
    void clear();

    // ---- data
    u8* memory = nullptr;
    size_t memory_bytesize = 0u;
    size_t position = 0u;

    // NOTE(hugo): extensions are allocated with the following layout : [header] | [padding] | [data]
    struct extension_header{
        extension_header* next = nullptr;
    };
    size_t extension_bytesize = 0u;
    extension_header* extension_head = nullptr;
};

// ---- dchunkarena

template<typename T, u16 chunk_capacity>
struct dchunkarena{
    T* get();
    void clear();
    void free();

    // ---- data

    struct chunk{
        T data[chunk_capacity];
        chunk* next;
    };

    chunk* head = nullptr;
    chunk* storage_head = nullptr;
    u16 current_chunk_space = 0u;
};

// ---- darray

template<typename T>
struct darray{

    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    void insert_empty(u32 index);
    void insert(u32 index, const T& value);
    void insert_multi(u32 index, u32 nelement);
    void remove(u32 index);
    void remove_multi(u32 index, u32 nelement);

    // NOTE(hugo): removes the /index/th element and replaces it with the one at the end of the array
    void remove_swap(u32 index);

    void push_empty();
    void push(const T& value);
    void push_multi(u32 nelement);
    void pop();
    void pop_multi(u32 nelement);

    void set_size(u32 new_size);
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
};

template<typename T>
void deep_copy(darray<T>& dest, darray<T>& src);

template<typename T>
bool deep_compare(const darray<T>& A, const darray<T>& src);

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

// ---- dpool

constexpr u32 dpool_no_element_available = UINT32_MAX;

#define DPOOL_KEEP_AVAILABLE_LIST_SORTED

template<typename T>
struct dpool{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    u32 insert_empty();
    u32 insert(const T& value);
    void remove(u32 identifier);

    void set_min_capacity(u32 new_capacity);

    // NOTE(hugo): very expensive ! should be used only to terminate / free a dpool
    // diterpool is a better solution if iteration is needed
    template<typename Action>
    void action_on_active(Action&& action);

    void clear();
    void free();

    size_t capacity_in_bytes();

    // ---- data

    struct element{
        union {
            T type;
            u32 next_element;
        };
    };

    element* memory = nullptr;
    u32 capacity = 0u;
    u32 available_element = dpool_no_element_available;
};

template<typename T>
void deep_copy(dpool<T>& dest, const dpool<T>& src);

// ---- diterpool

constexpr u32 diterpool_no_element_available = UINT32_MAX;

#define DITERPOOL_KEEP_AVAILABLE_LIST_SORTED TRUE

template<typename T>
struct diterpool{
    T& operator[](u32 index);
    const T& operator[](u32 index) const;

    // NOTE(hugo): use to iterate on active elements
    // u32 index, counter;
    // for(index = array.get_first(), counter = 0u; index < array.capacity && counter != array.size; index = array.get_next(index), ++counter)
    u32 get_first();
    u32 get_next(u32 current_identifier);

    u32 insert_empty();
    u32 insert(const T& value);
    void remove(u32 identifier);

    bool is_active(u32 identifier);

    void set_min_capacity(u32 new_capacity);

    void clear();
    void free();

    size_t capacity_in_bytes();

    // ---- internal / data
    // NOTE(hugo): the memory layout is [data, bitset]
    // so that the copy when reallocating is the smallest : the size of bitset
    // but this may be worse in terms of cache miss ?

    inline u8* get_bitset_ptr();
    inline void set_active(u32 identifier);
    inline void set_inactive(u32 identifier);

    struct element{
        union {
            T type = {};
            u32 next_element;
        };
    };

    element* memory = nullptr;
    u32 size = 0u;
    u32 capacity = 0u;
    u32 available_element = diterpool_no_element_available;
};

template<typename T>
void deep_copy(diterpool<T>& dest, const diterpool<T>& src);

// ---- dhashmap

constexpr u32 dhashmap_no_entry = UINT32_MAX;

template<typename K, typename T>
struct dhashmap{

    // NOTE(hugo): returned pointers are temporary references ie may change after any other operation
    T* get(const K& key, bool& was_entry_created);
    T* search(const K& key);
    void remove(const K& key);

    void set_min_capacity(u32 min_capacity);

    void clear();
    void free();

    size_t capacity_in_bytes();

    // ---- internal / data

    struct entry{
        K key;
        T value;
        u32 next = dhashmap_no_entry;
    };
    u32* table = nullptr;
    u32 table_capacity_minus_one = 0u;
    u32 nentries = 0u;
    diterpool<entry> storage;
};

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

#include "data_structure.inl"

#endif
