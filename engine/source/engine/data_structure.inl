// ---- buffer

template<typename T>
T& buffer<T>::operator[](u32 index){
    assert(data && index < size);

    return *((T*)data + index);
}

template<typename T>
const T& buffer<T>::operator[](u32 index) const{
    assert(data && index < size);

    return *((T*)data + index);
}

// ---- darena

void darena::reserve(size_t arena_bytesize){
    assert(!memory);

    memory = (u8*)malloc(arena_bytesize);
    memory_bytesize = arena_bytesize;
    if(!memory){
        LOG_ERROR("malloc FAILED - subsequent write will be out of bound");
    }
}

void darena::free(){
    ::free((void*)memory);
    while(extension_head){
        extension_header* next = extension_head->next;
        ::free(extension_head);
        extension_head = next;
    }
    *this = darena();
}

void* darena::push(size_t bytesize, size_t alignment){
    size_t offset_to_data = align_up_offset((uintptr_t)memory + position, alignment);

    // NOTE(hugo): using available memory
    if(!(position + offset_to_data + bytesize > memory_bytesize)){
        void* output_adress = (void*)(memory + position + offset_to_data);
        position += offset_to_data + bytesize;

        return output_adress;

    // NOTE(hugo): using an extension
    }else{
        size_t post_header_padding = align_up_offset(sizeof(extension_header), alignment);
        size_t offset_to_data = sizeof(extension_header) + post_header_padding;
        size_t allocation_bytesize = offset_to_data + bytesize;

        void* extension_memory = malloc(allocation_bytesize);
        if(!memory){
            LOG_ERROR("malloc FAILED - subsequent write will be out of bound");
        }
        extension_bytesize += bytesize;

        extension_header* header = (extension_header*)extension_memory;
        header->next = extension_head;
        extension_head = (extension_header*)extension_memory;

        return (void*)((u8*)extension_memory + offset_to_data);
    }
}

template<typename T>
void* darena::push(){
    return push(sizeof(T), alignof(T));
}

void darena::clear(){
    // NOTE(hugo): free extensions
    while(extension_head){
        extension_header* next = extension_head->next;
        ::free(extension_head);
        extension_head = next;
    }

    // NOTE(hugo): extend the arena to fit the extensions
    size_t expected_bytesize = position + extension_bytesize;
    if(expected_bytesize > memory_bytesize){
        void* new_memory = realloc((void*)memory, expected_bytesize);
        if(new_memory){
            memory = (u8*)new_memory;
            memory_bytesize = expected_bytesize;
        }else{
            LOG_ERROR("malloc FAILED - aborting memory extension");
        }
    }

    position = 0u;
}

// ---- dchunkarena

template<typename T, u16 chunk_capacity>
T* dchunkarena<T, chunk_capacity>::get(){
    if(current_chunk_space == 0u){
        chunk* new_chunk;
        if(storage_head){
            new_chunk = storage_head;
            storage_head = storage_head->next;
        }else{
            new_chunk = (chunk*)malloc(sizeof(chunk));
            if(!new_chunk){
                LOG_ERROR("malloc FAILED - subsequent write may be out of bounds");
            }
        }

        for(u32 iT = 0u; iT != chunk_capacity; ++iT){
            new((void*)&(new_chunk->data[iT])) T;
        }

        new_chunk->next = head;
        head = new_chunk;
        current_chunk_space = chunk_capacity;
    }

    u16 output_offset = chunk_capacity - current_chunk_space;
    --current_chunk_space;
    return &head->data[output_offset];
}

template<typename T, u16 chunk_capacity>
void dchunkarena<T, chunk_capacity>::clear(){
    while(head){
        for(u32 iT = 0u; iT != chunk_capacity; ++iT){
            head->data[iT].~T();
        }

        chunk* next_head = head->next;
        head->next = storage_head;
        storage_head = head;
        head = next_head;
    }
}

template<typename T, u16 chunk_capacity>
void dchunkarena<T, chunk_capacity>::free(){
    while(head){
        chunk* next_head = head->next;

        for(u32 iT = 0u; iT != chunk_capacity; ++iT){
            head->data[iT].~T();
        }

        ::free(head);
        head = next_head;
    }

    // NOTE(hugo): elements were already destructed when clearing
    while(storage_head){
        chunk* next_storage_head = storage_head->next;
        ::free(storage_head);
        storage_head = next_storage_head;
    }

    *this = dchunkarena();
}

// ---- darray

template<typename T>
static inline void darray_reallocate_to_capacity(darray<T>& da){
    // NOTE(hugo): realloc may reallocate & memcpy even if the requested size is the same
    void* new_data = realloc((void*)da.data, (size_t)da.capacity * sizeof(T));
    if(new_data){
        da.data = (T*)new_data;
    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory but subsequent write may be out of bound");
    }
}

template<typename T>
T& darray<T>::operator[](u32 index){
    assert(index < size);
    return data[index];
}

template<typename T>
const T& darray<T>::operator[](u32 index) const{
    assert(index < size);
    return data[index];
}

template<typename T>
void darray<T>::insert(u32 index, const T& value){
    assert(index <= size);

    if(size == capacity){
        capacity = 2u * max(1u, capacity);
        darray_reallocate_to_capacity(*this);
    }

    // NOTE(hugo): prevents undefined behavior due to invalid pointers when inserting at the end
    if(index < size){
        memmove(&data[index + 1], &data[index], sizeof(T) * (size - index));
    }
    data[index] = value;
    ++size;
}

template<typename T>
void darray<T>::insert_multi(u32 index, u32 nelement){
    assert(index <= size);

    if(size + nelement > capacity){
        capacity = max(2u * capacity, size + nelement);
        darray_reallocate_to_capacity(*this);
    }

    for(u32 ielement = 0u; ielement != nelement; ++ielement){
        new((void*)&data[size + ielement]) T;
    }

    // NOTE(hugo): prevents undefined behavior due to invalid pointers when inserting at the end
    //  destination_index < expected_new_size
    //   index + nelement < size + nelement
    //              index < size
    if(index < size){
        memmove(&data[index + nelement], &data[index], sizeof(T) * (size - index));
    }

    size += nelement;
}

template<typename T>
void darray<T>::remove(u32 index){
    assert(index < size);
    data[index].~T();

    // NOTE(hugo): prevents undefined behavior due to invalid pointers when removing at the end
    if(index < size - 1u){
        memmove(&data[index], &data[index + 1], sizeof(T) * (size - index - 1u));
    }

    --size;
}

template<typename T>
void darray<T>::remove_multi(u32 index, u32 nelement){
    assert(index + nelement - 1u < size);

    for(u32 ielement = 0u; ielement != nelement; ++ielement){
        data[index + ielement].~T();
    }

    // NOTE(hugo): prevents undefined behavior due to invalid pointers when removing at the end
    //     source_index < current_size
    // index + nelement < current_size
    if(index + nelement < size){
        memmove(&data[index], &data[index + nelement], sizeof(T) * (size - (index + nelement)));
    }

    size -= nelement;
}

template<typename T>
void darray<T>::remove_swap(u32 index){
    // NOTE(hugo): implies that size > 0
    assert(index < size);
    data[index].~T();
    data[index] = data[size - 1u];
    --size;
}

template<typename T>
void darray<T>::push(const T& value){
    if(size == capacity){
        capacity = 2u * max(1u, capacity);
        darray_reallocate_to_capacity(*this);
    }

    data[size] = value;
    ++size;
}

template<typename T>
void darray<T>::push_multi(u32 nelement){
    if(size + nelement > capacity){
        capacity = 2u * max(1u, capacity);
        darray_reallocate_to_capacity(*this);
    }

    for(u32 ielement = 0u; ielement != nelement; ++ielement){
        new((void*)&data[size + ielement]) T;
    }

    size += nelement;
}

template<typename T>
void darray<T>::pop(){
    assert(size);
    data[size].~T();
    --size;
}

template<typename T>
void darray<T>::pop_multi(u32 nelement){
    assert(nelement <= size);
    for(u32 ielement = size - nelement; ielement != size; ++ielement){
        data[ielement].~T();
    }
    size -= nelement;
}

template<typename T>
void darray<T>::set_size(u32 new_size){
    if(new_size > capacity){
        capacity = new_size;
        darray_reallocate_to_capacity(*this);
    }

    for(u32 ielement = new_size; ielement < size; ++ielement){
        data[ielement].~T();
    }

    size = new_size;
}

template<typename T>
void darray<T>::set_capacity(u32 new_capacity){
    assert(new_capacity >= size);

    if(new_capacity != capacity){
        capacity = new_capacity;
        darray_reallocate_to_capacity(*this);
    }
}

template<typename T>
void darray<T>::set_min_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        capacity = new_capacity;
        darray_reallocate_to_capacity(*this);
    }
}

template<typename T>
void darray<T>::clear(){
    for(u32 ielement = 0u; ielement != size; ++ielement){
        data[ielement].~T();
    }

    size = 0u;
}

template<typename T>
void darray<T>::free(){
    for(u32 ielement = 0u; ielement != size; ++ielement){
        data[ielement].~T();
    }

    ::free((void*)data);

    *this = darray<T>();
}

template<typename T>
size_t darray<T>::size_in_bytes(){
    return size * sizeof(T);
}

template<typename T>
size_t darray<T>::capacity_in_bytes(){
    return capacity * sizeof(T);
}

template<typename T>
void deep_copy(darray<T>& dest, darray<T>& src){
    assert(dest.data != src.data);

    dest.set_capacity(src.capacity);
    memmove(dest.data, src.data, src.size * sizeof(T));
    dest.size = src.size;
}

template<typename T>
bool deep_compare(const darray<T>& A, const darray<T>& B){
    return (A.size == B.size)
        && (A.capacity == B.capacity)
        && memcmp(&A, &B, sizeof(T) * A.size);
}

// ---- dqueue

template<typename T>
static inline void dqueue_increase_capacity(dqueue<T>& dr, u32 new_capacity){
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
static inline void dqueue_decrease_capacity(dqueue<T>& dr, u32 new_capacity){
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

static inline u32 dqueue_data_index_under(u32 queue_index_negative, u32 head_index, u32 capacity){
    u32 data_index = head_index + (queue_index_negative > head_index) * capacity;
    data_index = data_index - queue_index_negative;
    return data_index;
}

static inline u32 dqueue_data_index_over(u32 queue_index_positive, u32 head_index, u32 capacity){
    u32 data_index = head_index + queue_index_positive;
    data_index = data_index - (data_index >= capacity) * capacity;
    return data_index;
}

template<typename T>
T& dqueue<T>::operator[](u32 index){
    assert(index < size);

    u32 data_index = dqueue_data_index_over(index, head_index, capacity);
    return data[data_index];
}

template<typename T>
const T& dqueue<T>::operator[](u32 index) const{
    assert(index < size);

    u32 data_index = dqueue_data_index_over(index, head_index, capacity);
    return data[data_index];
}

template<typename T>
void dqueue<T>::push_front(const T& value){
    if(size == capacity){
        dqueue_increase_capacity(*this, max(1u, capacity) * 2u);
    }

    u32 insert_index = dqueue_data_index_under(1u, head_index, capacity);
    data[insert_index] = value;
    head_index = insert_index;
    ++size;
}

template<typename T>
void dqueue<T>::pop_front(){
    assert(size);
    --size;
    data[head_index].~T();
    head_index = dqueue_data_index_over(1u, head_index, capacity);
}

template<typename T>
void dqueue<T>::push_back(const T& value){
    if(size == capacity){
        dqueue_increase_capacity(*this, max(1u, capacity) * 2u);
    }

    u32 insert_index = dqueue_data_index_over(size, head_index, capacity);
    data[insert_index] = value;
    ++size;
}

template<typename T>
void dqueue<T>::pop_back(){
    assert(size);
    operator[](size - 1u).~T();
    --size;
}

template<typename T>
void dqueue<T>::set_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        dqueue_increase_capacity(*this, new_capacity);
    }else if(new_capacity < capacity){
        for(u32 ielement = new_capacity; ielement != capacity; ++ielement){
            operator[](ielement).~T();
        }

        dqueue_decrease_capacity(*this, new_capacity);
    }
}

template<typename T>
void dqueue<T>::set_min_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        dqueue_increase_capacity(*this, new_capacity);
    }
}

template<typename T>
void dqueue<T>::clear(){
    for(u32 ielement = 0u; ielement != size; ++ielement){
        operator[](ielement).~T();
    }

    size = 0u;
    head_index = 0u;
}

template<typename T>
void dqueue<T>::free(){
    for(u32 ielement = 0u; ielement != size; ++ielement){
        operator[](ielement).~T();
    }

    ::free((void*)data);

    *this = dqueue<T>();
}

template<typename T>
size_t dqueue<T>::size_in_bytes(){
    return size * sizeof(T);
}

template<typename T>
size_t dqueue<T>::capacity_in_bytes(){
    return capacity * sizeof(T);
}

// ---- dpool

static inline size_t dpool_get_bitset_bytesize(u32 capacity){
    size_t bitset_size = (size_t)(capacity / 8u);
    bitset_size = bitset_size + (size_t)((8u * (u32)bitset_size) != capacity);
    return bitset_size;
}

template<typename T>
static inline size_t dpool_get_data_bytesize(u32 capacity){
    return sizeof(typename dpool<T>::element) * (size_t)capacity;
}

template<typename T>
static inline size_t dpool_get_memory_bytesize(u32 capacity){
    return dpool_get_data_bytesize<T>(capacity) + dpool_get_bitset_bytesize(capacity);
}

// NOTE(hugo): ptr must have a bytesize of at least dpool_get_memory_bytesize<T>(capacity)
template<typename T>
static inline void dpool_use_memory(dpool<T>& dpa, u32 capacity, void* ptr){
    assert(capacity && ptr);
    assert(is_aligned(ptr, alignof(typename dpool<T>::element)));

    dpa.memory = (typename dpool<T>::element*)ptr;
    dpa.capacity = capacity;
    dpa.available_element = 0u;

    // NOTE(hugo): clearing the bitset
    memset(ptr + dpool_get_data_bytesize<T>(capacity), 0u, dpool_get_bitset_bytesize(capacity));

    // NOTE(hugo): making new elements available in the linked list
    u32 current_identifier = 0u;
    for(u32 new_identifier = 1; new_identifier < capacity - 1; ++new_identifier){
        dpa.memory[current_identifier].next_element = new_identifier;
        current_identifier = new_identifier;
    }
    dpa.memory[current_identifier].next_element = dpool_no_element_available;
}

template<typename T>
static inline void dpool_increase_capacity(dpool<T>& dpa, u32 new_capacity){
    assert(new_capacity > dpa.capacity); // NOTE(hugo): implies that new_capacity > 0u

    void* new_memory = realloc((void*)dpa.memory, dpool_get_memory_bytesize<T>(new_capacity));

    if(new_memory){
        dpa.memory = (typename dpool<T>::element*)new_memory;

        // NOTE(hugo): move the bitset & clear the unused bitset memory
        u8* bitset_ptr = dpa.get_bitset_ptr();
        u8* new_bitset_ptr = (u8*)dpa.memory + new_capacity * sizeof(typename dpool<T>::element);

        memset(new_bitset_ptr, 0u, dpool_get_bitset_bytesize(new_capacity));
        memmove(new_bitset_ptr, bitset_ptr, dpool_get_bitset_bytesize(dpa.capacity));

        // NOTE(hugo): go to the end of the linked list of available elements
        u32 owner_next = dpa.capacity;
        u32 next = dpa.available_element;
        while(next != dpool_no_element_available){
            owner_next = next;
            next = dpa.memory[owner_next].next_element;
        }

        // NOTE(hugo): make new elements available at the end of the linked list
        dpa.memory[owner_next].next_element = dpa.capacity;
        for(u32 new_available = dpa.capacity; new_available < new_capacity - 1u; ++new_available){
            dpa.memory[new_available].next_element = new_available + 1u;
        }
        dpa.memory[new_capacity - 1u].next_element = dpool_no_element_available;

        dpa.available_element = min(dpa.available_element, dpa.capacity);

        dpa.capacity = new_capacity;

    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory and capacity");
    }
}

template<typename T>
T& dpool<T>::operator[](u32 index){
    assert(index < capacity && is_active(index));
    return memory[index].type;
}

template<typename T>
const T& dpool<T>::operator[](u32 index) const{
    assert(index < capacity && is_active(index));
    return memory[index].type;
}

template<typename T>
u32 dpool<T>::get_first(){
    for(u32 identifier = 0u; identifier != capacity; ++identifier){
        if(is_active(identifier)){
            return identifier;
        }
    }

    return capacity;
}

template<typename T>
u32 dpool<T>::get_next(u32 current_identifier){
    assert(is_active(current_identifier));
    for(u32 identifier = current_identifier + 1u; identifier < capacity; ++identifier){
        if(is_active(identifier)){
            return identifier;
        }
    }

    return capacity;
}

// TODO(hugo): remove iteration by using a linked list of free elements
template<typename T>
u32 dpool<T>::insert_empty(){
    if(available_element == dpool_no_element_available){
        dpool_increase_capacity(*this, 2u * max(1u, capacity));
    }

    u32 insertion_index = available_element;
    available_element = memory[insertion_index].next_element;
    set_active(insertion_index);
    new((void*)&memory[insertion_index].type) T;

    return insertion_index;
}

template<typename T>
u32 dpool<T>::insert(const T& value){
    u32 insertion_index = insert_empty();
    memory[insertion_index].type = value;
    return insertion_index;
}

template<typename T>
void dpool<T>::remove(u32 identifier){
    assert(is_active(identifier));
    set_inactive(identifier);
    memory[identifier].type.~T();

#ifdef DPOOL_KEEP_AVAILABLE_LIST_SORTED
    // NOTE(hugo): everything works because dpool_no_element_available > identifier
    if(available_element > identifier){
        memory[identifier].next_element = available_element;
        available_element = identifier;
    }else{
        u32 current_element = available_element;
        while(memory[current_element].next_element < identifier){
            current_element = memory[current_element].next_element;
        }

        memory[identifier].next_element = memory[current_element].next_element;
        memory[current_element].next_element = identifier;
    }

#else
    // NOTE(hugo): the order of elements in the linked list depends on the insert / remove order
    memory[identifier].next_element = available_element;
    available_element = identifier;
#endif
}

template<typename T>
void dpool<T>::set_min_capacity(u32 new_capacity){
    if(new_capacity > capacity){
        dpool_increase_capacity(*this, new_capacity);
    }
}

template<typename T>
void dpool<T>::clear(){
    // NOTE(hugo): memset expects a non-null pointer
    if(capacity){
        memset(get_bitset_ptr(), 0, dpool_get_bitset_bytesize(capacity));

        available_element = 0u;
        for(u32 ielement = 0; ielement < (capacity - 1u); ++ielement){
            memory[ielement].type.~T();
            memory[ielement].next_element = ielement + 1;
        }
        memory[capacity - 1u].type.~T();
        memory[capacity - 1u].next_element = dpool_no_element_available;
    }
}

template<typename T>
void dpool<T>::free(){
    for(u32 ielement = 0; ielement != capacity; ++ielement){
        memory[ielement].type.~T();
    }

    ::free(memory);

    *this = dpool<T>();
}

template<typename T>
size_t dpool<T>::capacity_in_bytes(){
    return dpool_get_memory_bytesize<T>(capacity);
}

template<typename T>
u8* dpool<T>::get_bitset_ptr(){
    return (u8*)memory + capacity * sizeof(typename dpool<T>::element);
}

template<typename T>
void dpool<T>::set_active(u32 identifier){
    assert(identifier < capacity); // NOTE(hugo): implies that capacity > 0u

    u32 byte_offset = identifier / 8u;
    u8* identifier_ptr = get_bitset_ptr()  + byte_offset;
    u32 bit_offset = identifier - byte_offset * 8u;

    assert(!is_active(identifier));

    (*identifier_ptr) |= (u8)(1u << bit_offset);
}

template<typename T>
void dpool<T>::set_inactive(u32 identifier){
    assert(identifier < capacity); // NOTE(hugo): implies that capacity > 0u

    u32 byte_offset = identifier / 8u;
    u8* identifier_ptr = get_bitset_ptr()  + byte_offset;
    u32 bit_offset = identifier - byte_offset * 8u;

    assert(is_active(identifier));

    (*identifier_ptr) &= (u8)(~(1u << bit_offset));
}

template<typename T>
bool dpool<T>::is_active(u32 identifier){
    assert(identifier < capacity);

    u32 byte_offset = identifier / 8u;
    u8* identifier_ptr = get_bitset_ptr()  + byte_offset;
    u32 bit_offset = identifier - byte_offset * 8u;

    return (((u32)(*identifier_ptr) >> bit_offset) & 1u) != 0u;
}

template<typename T>
void deep_copy(dpool<T>& dest, dpool<T>& src){
    if(dest.capacity != src.capacity){
        // NOTE(hugo): set_min_capacity is not appropriate because the bitset location needs to be the same
        void* new_memory = realloc((void*)dest.memory, dpool_get_memory_bytesize<T>(src.capacity));
        if(new_memory){
            dest.memory = (typename dpool<T>::element*)new_memory;
            memmove(dest.memory, src.memory, dpool_get_memory_bytesize<T>(src.capacity));
            dest.capacity = src.capacity;
            dest.available_element = src.available_element;
        }else{
            LOG_ERROR("realloc FAILED - keeping previous memory and capacity");
        }
    }
}

// ---- dhashmap

template<typename K>
static u32 dhashmap_hash_key(const K& key){
    return FNV1a_32ptr((uchar*)&key, sizeof(K));
}
template<u32>
static u32 dhashmap_hash_key(const u32& key){
    return wang_hash(key);
}
template<s32>
static u32 dhashmap_hash_key(const s32& key){
    return wang_hash(*(u32*)&key);
}


template<typename K, typename T>
static void dhashmap_reallocate_to_capacity(dhashmap<K, T>& dhm){
    u32 table_capacity = dhm.table_capacity_minus_one + 1u;

    // NOTE(hugo): extend the table
    ::free(dhm.table);
    dhm.table = (u32*)malloc(sizeof(u32) * table_capacity);
    if(!dhm.table){
        LOG_ERROR("malloc FAILED - subsequent write will be out of memory");
    }

    // NOTE(hugo): 0xFF is used to set table to dhashmap_no_entry = UINT32_MAX
    memset(dhm.table, 0xFF, sizeof(u32) * table_capacity);

    // NOTE(hugo): rehash the current storage
    for(u32 ientry = dhm.storage.get_first(); ientry != dhm.storage.capacity; ientry = dhm.storage.get_next(ientry)){
        typename dhashmap<K, T>::entry* storage_entry = &dhm.storage[ientry];
        u32 hash = dhashmap_hash_key(storage_entry->key);
        u32 table_index = hash & dhm.table_capacity_minus_one;

        storage_entry->next = dhm.table[table_index];
        dhm.table[table_index] = ientry;
    }
}

template<typename K, typename T>
void dhashmap<K, T>::set_min_capacity(u32 min_capacity){
    u32 min_table_size = next_pow2(min_capacity);
    if((table_capacity_minus_one + 1u) < min_table_size){
        table_capacity_minus_one = min_table_size - 1u;
        dhashmap_reallocate_table(*this);
    }

    storage.set_min_capacity(min_capacity);
}

// NOTE(hugo): search for an already existing entry in the table
#define DHASHMAP_GET_SEARCH_COMMON                      \
    u32* next_ptr = table + table_index;                \
    u32 next = *next_ptr;                               \
    while(next != dhashmap_no_entry){                   \
        if(storage[next].key == key){                   \
            return &storage[next].value;                \
        }                                               \
        next_ptr = &storage[next].next;                 \
        next = *next_ptr;                               \
    }

template<typename K, typename T>
T* dhashmap<K, T>::get(const K& key){
    u32 hash = dhashmap_hash_key(key);
    u32 table_index = hash & table_capacity_minus_one;

    if(table != nullptr){
        DHASHMAP_GET_SEARCH_COMMON
    }

    // NOTE(hugo): reallocate when needed
    if(table == nullptr || (float)nentries / (float)(table_capacity_minus_one + 1u) > 0.5f){
        table_capacity_minus_one = (table_capacity_minus_one << 1u) | 1u;
        dhashmap_reallocate_to_capacity(*this);
    }

    // NOTE(hugo): create a new entry
    ++nentries;
    u32 new_index = storage.insert_empty();
    storage[new_index].key = key;
    storage[new_index].next = table[table_index];
    table[table_index] = new_index;
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
#undef DHASHMAP_SEARCH_GET_COMMON

template<typename K, typename T>
void dhashmap<K, T>::remove(const K& key){
    if(table != nullptr){
        u32 hash = dhashmap_hash_key(key);
        u32 table_index = hash & table_capacity_minus_one;

        // NOTE(hugo): search for an existing entry
        u32* next_ptr = table + table_index;
        u32 next = *next_ptr;
        while(next != dhashmap_no_entry){
            if(storage[next].key == key){
                --nentries;
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
    // NOTE(hugo): 0xFF is used to set table to dhashmap_no_entry = UINT32_MAX
    memset(table, 0xFF, sizeof(u32) * (table_capacity_minus_one + 1u));
    nentries = 0u;
    storage.clear();
}

template<typename K, typename T>
void dhashmap<K, T>::free(){
    ::free(table);
    storage.free();
    *this = dhashmap<K, T>();
}

template<typename K, typename T>
size_t dhashmap<K, T>::capacity_in_bytes(){
    size_t table_bytesize = sizeof(u32) * (table_capacity_minus_one + 1u);
    return table_bytesize + storage.capacity_in_bytes();
}

// ---- daryheap

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
static inline void resize_daryheap(daryheap<D, T, compare>* heap, u32 new_capacity){
    // NOTE(hugo): realloc may reallocate & memcpy even if the requested size is the same
    void* new_ptr = realloc(heap->data, sizeof(T) * new_capacity);
    if(new_ptr){
        heap->data = (T*)new_ptr;
    }else{
        LOG_ERROR("realloc FAILED - keeping previous memory but subsequent write may be out of bound");
    }
}

template<u32 D>
static inline u32 daryheap_get_parent(u32 index){
    return (index - 1u) / D;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
u32 daryheap<D, T, compare>::push(T element){
    if(size == capacity){
        u32 new_capacity = max(1u, 2u * capacity);
        resize_daryheap(this, new_capacity);
    }

    // NOTE(hugo): swap everything down until at the right spot for the new element
    u32 index_current = size;
    ++size;
    u32 index_parent = daryheap_get_parent<D>(index_current);

    while(index_current && compare(data[index_parent], element) < 0){
        data[index_current] = data[index_parent];
        index_current = index_parent;
        index_parent = daryheap_get_parent<D>(index_current);
    }

    data[index_current] = element;

    return index_current;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
static inline u32 daryheap_get_highest_priority_child(daryheap<D, T, compare>* heap, u32 index_start){
    if constexpr (D == 2){
        u32 index_next = min(index_start + 1u, heap->size);
        return index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) <= 0);

    }else if constexpr (D == 3){
        u32 index_next = min(index_start + 1u, heap->size);
        u32 temp_max = index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) <= 0);

        index_next = min(index_start + 2u, heap->size);
        return (compare(heap->data[temp_max], heap->data[index_next]) <= 0) ? index_next : temp_max;

    }else if constexpr (D == 4){
        u32 index_next = min(index_start + 1u, heap->size);
        u32 temp_max = index_start + (u32)(compare(heap->data[index_start], heap->data[index_next]) <= 0);

        index_next = min(index_start + 2u, heap->size);
        temp_max = (compare(heap->data[temp_max], heap->data[index_next]) <= 0) ? index_next : temp_max;

        index_next = min(index_start + 3u, heap->size);
        return (compare(heap->data[temp_max], heap->data[index_next]) <= 0) ? index_next : temp_max;

    }else{
        u32 index_end = min(index_start + D, heap->size);
        u32 index_max = index_start;

        for(u32 index_current = index_start + 1u; index_current < index_end; ++index_current){
            if(compare(heap->data[index_max], heap->data[index_current]) <= 0){
                index_max = index_current;
            }
        }

        return index_max;
    }
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::pop(u32 index){
    assert(size);

    --size;
    T& element = data[size];

    u32 index_current = index;

    // NOTE(hugo): element (CHILD) has higher priority than its parents (PARENT)
    // ie swap them down until at the right spot
    u32 index_parent = daryheap_get_parent<D>(index_current);
    while(index_current && (compare(data[index_parent], element) < 0)){
        data[index_current] = data[index_parent];
        index_current = index_parent;
        index_parent = daryheap_get_parent<D>(index_current);
    }

    // NOTE(hugo): element (PARENT) has lower priority than its children (CHILD)
    // ie swap the highest priority child up until at the right spot
    u32 index_start_children = index_current * D + 1u;
    while(index_start_children < size){
        u32 index_comp_child = daryheap_get_highest_priority_child<D, T, compare>(this, index_start_children);

        if(!(compare(element, data[index_comp_child]) < 0)){
            break;
        }

        data[index_current] = data[index_comp_child];
        index_current = index_comp_child;
        index_start_children = index_current * D + 1u;
    }

    data[index_current] = element;
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
T& daryheap<D, T, compare>::get_top(){
    return data[0u];
}

template<u32 D, typename T, s32 (*compare)(const T& A, const T& B)>
void daryheap<D, T, compare>::pop_top(){
    pop(0u);
}
