#include <memory>       // NOTE(hugo): t_align compares results with std::align

namespace bw::utest{

    constexpr float ftolerance = 0.000001f;
    constexpr float dtolerance = 0.00000000000001f;

    void t_quat_rot(){
        vec3 v = {1., 1., 1.};
        vec3 pA = {1., 0., 0.};
        vec3 pB = {0., 1., 0.};

        vec3 norm = cross(pA, pB);
        quat q = quat_from_axis(PI<float> / 2.f, norm);
        vec3 vquat = rotate(v, q);

        vec3 biv = wedge(pA, pB);
        rot3 r = make_rot(PI<float> / 2.f, biv);
        vec3 vrot = rotate(v, r);

        if(!almost_equal(vquat.x, vrot.x, ftolerance, 1) && almost_equal(vquat.y, vrot.y, ftolerance, 1) && almost_equal(vquat.z, vrot.z, ftolerance, 1)){
            LOG_ERROR("utest::t_quat_rot() FAILED");
        }else{
            LOG_INFO("utest::t_quat_rot() SUCCESS");
        }
    }

    void t_defer(){
        bool success = true;
        int value = 1;
        int* value_ptr = &value;
        {
            success = success && (value == 1);
            DEFER {
                *value_ptr = 2;
            };
            success = success && (value == 1);
        }
        success = success && (value == 2);

        if(!success){
            LOG_ERROR("utest::t_defer() FAILED");
        }else{
            LOG_INFO("utest::t_defer() SUCCESS");
        }
    }

    void t_darena(){
        bool success = true;

        size_t bytesize = 0u;

        darena arena;
        arena.reserve(10u);
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == 0u);
        success &= (arena.extension_head == nullptr);

        u32* vu32 = (u32*)arena.push(sizeof(u32), alignof(u32));
        success &= (vu32 != nullptr && (void*)vu32 == arena.memory);
        *vu32 = 1u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32));
        success &= (arena.extension_head == nullptr);

        s32* vs32 = (s32*)arena.push(sizeof(s32), alignof(s32));
        success &= (vs32 != nullptr && (void*)vs32 == (void*)((u8*)vu32 + 4u));
        *vs32 = 2;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32));
        success &= (arena.extension_head == nullptr);

        u32* v2u32 = (u32*) arena.push<u32>();
        success &= (v2u32 != nullptr && (((void*)v2u32 < (void*)arena.memory) || ((void*)v2u32 > (void*)((u8*)arena.memory + arena.memory_bytesize))));
        *v2u32 = 3u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32));
        success &= (arena.extension_head != nullptr);
        bytesize += sizeof(u32);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (arena.extension_bytesize == bytesize && extension->next == nullptr);
        }

        u8* vu8 = (u8*)arena.push<u8>();
        success &= (vu8 != nullptr && (void*)vu8 == (void*)((u8*)vs32 + 4u));
        *vu8 = 4u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (arena.extension_bytesize == bytesize && extension->next == nullptr);
        }

        u64* vu64 = (u64*)arena.push<u64>();
        success &= (vu64 != nullptr);
        *vu64 = 5u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        bytesize += sizeof(u64);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (arena.extension_bytesize == bytesize && extension->next != nullptr);
            extension = (darena::extension_header*)extension->next;
            success &= (extension->next == nullptr);
        }

        u8* v2u8 = (u8*)arena.push<u8>();
        success &= (v2u8 != nullptr && (void*)v2u8 == (void*)((u8*)vu8 + 1u));
        *v2u8 = 6u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (arena.extension_bytesize == bytesize && extension->next != nullptr);
            extension = (darena::extension_header*)extension->next;
            success &= (extension->next == nullptr);
        }

        arena.clear();
        success &= (arena.memory && arena.memory_bytesize == sizeof(u32) + sizeof(s32) + sizeof(u8) + sizeof(u32) + sizeof(u8) + sizeof(u64) && arena.position == 0u);
        success &= (arena.extension_head == nullptr);

        u8* v3u8 = (u8*)arena.push<u8>();
        success &= (v3u8 != nullptr && (void*)v3u8 == arena.memory);
        success &= (arena.extension_head == nullptr);

        u64* v2u64 = (u64*)arena.push<u64>();
        success &= (v2u64 != nullptr && (void*)v2u64 == (void*)((u8*)arena.memory + 8u));
        success &= (arena.extension_head == nullptr);

        arena.free();
        success &= (arena.memory == nullptr && arena.memory_bytesize == 0u && arena.position == 0u);
        success &= (arena.extension_head == nullptr);

        arena.reserve(2u);
        arena.push<u8>();
        arena.push<u32>();

        arena.free();

        if(!success){
            LOG_ERROR("utest::t_darena() FAILED");
        }else{
            LOG_INFO("utest::t_darena() SUCCESS");
        }
    }

    void t_darray(){
        bool success = true;

        darray<s32> array;

        array.set_capacity(2);
        success &= (array.size == 0 && array.capacity == 2);

        array.push(1);
        array.push(2);
        success &= (array.size == 2 && array.capacity == 2 && array[0] == 1 && array[1] == 2);

        array.insert(1, 3);
        success &= (array.size == 3 && array.capacity == 4 && array[0] == 1 && array[1] == 3 && array[2] == 2);

        array.insert_multi(1, 2);
        array[1] = 4;
        array[2] = 5;
        success &= (array.size == 5 && array.capacity == 8 && array[0] == 1 && array[1] == 4 && array[2] == 5 && array[3] == 3 && array[4] == 2);

        array.remove(3);
        success &= (array.size == 4 && array.capacity == 8 && array[0] == 1 && array[1] == 4 && array[2] == 5 && array[3] == 2);

        array.remove_multi(2, 2);
        success &= (array.size == 2 && array.capacity == 8 && array[0] == 1 && array[1] == 4);

        array.remove_swap(0);
        success &= (array.size == 1 && array.capacity == 8 && array[0] == 4);

        array.push_multi(4);
        array[1] = 7;
        array[2] = 8;
        array[3] = 9;
        array[4] = 10;
        success &= (array.size == 5 && array.capacity == 8 && array[0] == 4 && array[1] == 7 && array[2] == 8 && array[3] == 9 && array[4] == 10);

        array.pop();
        success &= (array.size == 4 && array.capacity == 8 && array[0] == 4 && array[1] == 7 && array[2] == 8 && array[3] == 9);

        array.pop_multi(2);
        success &= (array.size == 2 && array.capacity == 8 && array[0] == 4 && array[1] == 7);

        success &= (array.size_in_bytes() == 2u * sizeof(int));
        success &= (array.capacity_in_bytes() == 8u * sizeof(int));

        array.set_size(3);
        success &= (array.size == 3 && array.capacity == 8 && array[0] == 4 && array[1] == 7);
        array.set_size(9);
        success &= (array.size == 9 && array.capacity == 9 && array[0] == 4 && array[1] == 7);

        array.set_capacity(12);
        success &= (array.size == 9 && array.capacity == 12 && array[0] == 4 && array[1] == 7);

        array.set_min_capacity(10);
        success &= (array.size == 9 && array.capacity == 12 && array[0] == 4 && array[1] == 7);

        array.clear();
        success &= (array.size == 0 && array.capacity == 12);

        array.free();

        array.set_min_capacity(5);
        success &= (array.size == 0 && array.capacity == 5);

        array.push(11u);
        success &= (array.size == 1 && array.capacity == 5 && array[0] == 11u);

        array.free();

        if(!success){
            LOG_ERROR("utest::t_darray() FAILED");
        }else{
            LOG_INFO("utest::t_darray() SUCCESS");
        }
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

    void t_dpool(){
        bool success = true;

        dpool<u32> pool;
        success &= (pool.capacity == 0 && pool.memory == nullptr && pool.available_element == dpool_no_element_available);

        pool.set_min_capacity(1u);
        success &= (pool.capacity == 1u && pool.available_element == 0u);
        success &= (pool.get_first() == pool.capacity);

        pool.insert(0u);
        success &= (pool.capacity == 1u && pool.available_element == dpool_no_element_available && pool.get_first() == 0u);

        pool.insert(1u);
        success &= (pool.capacity == 2u && pool.available_element == dpool_no_element_available);
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
        success &= (pool.capacity == 4u && pool.available_element == dpool_no_element_available);
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

    void t_dhashmap_randomized(){
        bool success = true;

        constexpr u32 ntest = 100u;
        constexpr u32 nvalues = 100u;
        constexpr u32 noperations = 10000u;

        s32 status_counter[nvalues];
        memset(status_counter, 0, nvalues * sizeof(s32));

        seed_random_with_time();
        u64 seed_copy = BEEWAX_INTERNAL::seed.seed64;

        dhashmap<u32, s32> hmap;

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
                            hmap.remove(random_key);
                            s32* searched = hmap.search(random_key);
                            //LOG_TRACE("remove[%d] %d", random_key, status_counter[random_key]);
                            success = success && searched == nullptr;
                            status_counter[random_key] = 0;
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
            LOG_TRACE("utest::t_dhashmap_randomized() seed: %" PRId64, seed_copy);
        }else{
            LOG_INFO("utest::t_dhashmap_randomized() SUCCESS");
        }
    }

    void t_dhashmap(){
        bool success = true;

        dhashmap<s32, s32> hmap;
        bool was_created;

        {
            s32* search = hmap.search(1);
            success &= (hmap.table == nullptr && hmap.table_capacity_minus_one == 0u);
            success &= (hmap.nentries == 0u && hmap.storage.memory == nullptr && hmap.storage.capacity == 0u && hmap.storage.get_first() == hmap.storage.capacity);
            success &= (search == nullptr);
        }

        {
            s32* get = hmap.get(1, was_created);
            success &= (get != nullptr && was_created);
            *get = -1;
            success &= (hmap.table != nullptr && hmap.table_capacity_minus_one == 1u);
            success &= (hmap.nentries == 1u && hmap.storage.memory != nullptr && hmap.storage.capacity == 2u && hmap.storage.get_first() == 0u);

            s32* search = hmap.search(1);
            success &= (search == get && *search == -1);

            u32 iter = hmap.storage.get_first();
            success &= (iter == 0u);
            iter = hmap.storage.get_next(iter);
            success &= (iter == hmap.storage.capacity);

            s32* get_second = hmap.get(1, was_created);
            success &= (get_second == get && *get_second == -1 && !was_created);
            success &= (hmap.table != nullptr && hmap.table_capacity_minus_one == 1u);
            success &= (hmap.nentries == 1u && hmap.storage.memory != nullptr && hmap.storage.capacity == 2u && hmap.storage.get_first() == 0u);
        }

        {
            hmap.remove(1);
            success &= (hmap.table != nullptr && hmap.table_capacity_minus_one == 1u);
            success &= (hmap.nentries == 0u && hmap.storage.memory != nullptr && hmap.storage.capacity == 2u && hmap.storage.get_first() == hmap.storage.capacity);

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

            success &= (hmap.nentries == 3u && hmap.storage.capacity == 4u);

            u32 iter = hmap.storage.get_first();
            success &= (iter == 0u && hmap.storage[iter].key == 2 && hmap.storage[iter].value == -2);
            iter = hmap.storage.get_next(iter);
            success &= (iter == 1u && hmap.storage[iter].key == 3 && hmap.storage[iter].value == -3);
            iter = hmap.storage.get_next(iter);
            success &= (iter == 2u && hmap.storage[iter].key == 4 && hmap.storage[iter].value == -4);
            iter = hmap.storage.get_next(iter);
            success &= (iter == hmap.storage.capacity);
        }

        {
            hmap.remove(3);

            s32* search = hmap.search(2);
            success &= (search && *search == -2);

            search = hmap.search(3);
            success &= (search == nullptr);

            search = hmap.search(4);
            success &= (search && *search == -4);

            u32 iter = hmap.storage.get_first();
            success &= (iter == 0u && hmap.storage[iter].key == 2 && hmap.storage[iter].value == -2);
            iter = hmap.storage.get_next(iter);
            success &= (iter == 2u && hmap.storage[iter].key == 4 && hmap.storage[iter].value == -4);
            iter = hmap.storage.get_next(iter);
            success &= (iter == hmap.storage.capacity);
        }

        hmap.free();

        if(!success){
            LOG_ERROR("utest::t_dhashmap() FAILED");
        }else{
            LOG_INFO("utest::t_dhashmap() SUCCESS");
        }
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

    void t_align(){
        bool success = true;

        seed_random_with_time();
        u64 seed_copy = BEEWAX_INTERNAL::seed.seed64;

        constexpr u32 ntest = 16;
        constexpr u32 max_alignment_pow2 = 7; // 2^7 == 128

        for(u32 itest = 0u; itest != ntest; ++itest){
            u64 random = random_u64();
            void* adress = (void*)random;

            for(u32 ialign = 1u; ialign != max_alignment_pow2; ++ialign){
                size_t alignment = (size_t)1 << ialign;
                void* aligned_up = void_align_up(adress, alignment);
                void* aligned_down = void_align_down(adress, alignment);

                void* adress_copy = adress;
                size_t size = 16;
                //std::align(alignment, (size_t)UINT64_MAX, adress_copy, size);

                success &= (((u64)aligned_up % (u64)alignment) == 0u) && ((u64)aligned_up >= random) && (((u64)aligned_up - random) < (u64)alignment);
                success &= (((u64)aligned_down % (u64)alignment) == 0u) && ((u64)aligned_down <= random) && ((random - (u64)aligned_down) < (u64)alignment);
            }
        }

        if(!success){
            LOG_ERROR("utest::t_align() FAILED");
            LOG_TRACE("utest::t_align() seed: %" PRId64, seed_copy);
        }else{
            LOG_INFO("utest::t_align() SUCCESS");
        }
    }

    void t_find_noise_magic_normalizer(){
        seed_random_with_time();

        u32 nsamples = 100000000u;
        float scale = 10000.f;

        float max_value;
        float magic_normalizer;

        // NOTE(hugo): perlin_noise(const float x)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float value = perlin_noise(random_float_normalized() * scale);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() perlin_noise(x) max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): perlin_noise(const float x, const float y)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float x = random_float_normalized() * scale;
            float y = random_float_normalized() * scale;

            float value = perlin_noise(x, y);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() perlin_noise(x, y)  max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): simplex_noise(const float x)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float value = simplex_noise(random_float_normalized() * scale);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() simplex_noise(x) max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): simplex_noise(const float x, const float y)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float x = random_float_normalized() * scale;
            float y = random_float_normalized() * scale;

            float value = simplex_noise(x, y);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() simplex_noise(x, y)  max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);
    }

    void t_detect_vector_capacilities(){
        s32 vector_capabilities = detect_vector_capabilities();
        LOG_INFO("SSE       %s", vector_capabilities > 0 ? "YES" : "NO");
        LOG_INFO("SSE2      %s", vector_capabilities > 1 ? "YES" : "NO");
        LOG_INFO("SSE3      %s", vector_capabilities > 2 ? "YES" : "NO");
        LOG_INFO("SSSE3     %s", vector_capabilities > 3 ? "YES" : "NO");
        LOG_INFO("SSE4.1    %s", vector_capabilities > 4 ? "YES" : "NO");
        LOG_INFO("SSE4.2    %s", vector_capabilities > 5 ? "YES" : "NO");
    }

    void t_isort(){
        bool success = true;

        constexpr u32 ntest = 10u;
        constexpr u32 array_size = 50u;

        s32 array_isort[array_size];
        seed_random_with_time();
        u64 seed_copy = BEEWAX_INTERNAL::seed.seed64;

        for(u32 itest = 0u; itest != ntest; ++itest){
            // NOTE(hugo): generate a random array
            for(u32 inumber = 0u; inumber != array_size; ++inumber){
                array_isort[inumber] = random_s32();
            }

            // NOTE(hugo): sort and validate
            isort(&array_isort[0], array_size);

            for(u32 inumber = 0u; inumber != array_size - 1u; ++inumber){
                success = success && array_isort[inumber] <= array_isort[inumber + 1u];
            }
        }

        if(!success){
            LOG_ERROR("utest::t_isort() FAILED");
            LOG_TRACE("utest::t_isort seed: %" PRId64, seed_copy);
        }else{
            LOG_INFO("utest::t_isort() SUCCESS");
        }
    }

    void t_binsearch(){
        bool success = true;

        constexpr u32 ntest = 10u;
        constexpr u32 array_size = 50u;

        s32 array[array_size];
        seed_random_with_time();
        u64 seed_copy = BEEWAX_INTERNAL::seed.seed64;

        for(u32 itest = 0u; itest != ntest; ++itest){
            for(u32 inumber = 0u; inumber != array_size; ++inumber){
                array[inumber] = random_s32();
            }

            qsort(array, array_size);

            for(u32 isearch = 0u; isearch != 10u * array_size; ++isearch){
                u32 item_index = random_u32_range_uniform(array_size);
                s32& item_value = array[item_index];

                s32* lin_ptr = linsearch_lower(array, array_size, item_value);
                s32* bin_ptr = binsearch_lower(array, array_size, item_value);

                success = success && lin_ptr && bin_ptr && (lin_ptr == bin_ptr) && *lin_ptr == item_value;
            }

            for(u32 iinsert = 0u; iinsert != 10u * array_size; ++iinsert){
                s32 item_value = random_s32();

                u32 lin_ins = lininsert_lower(array, array_size, item_value);
                u32 bin_ins = lininsert_lower(array, array_size, item_value);

                success = success && (lin_ins == bin_ins);
                success = success && ((lin_ins == array_size && item_value > array[array_size - 1u]) || (array[lin_ins] >= item_value));
            }
        }

        if(!success){
            LOG_ERROR("utest::t_binsearch() FAILED");
            LOG_TRACE("utest::t_binsearch seed: %" PRId64, seed_copy);
        }else{
            LOG_INFO("utest::t_binsearch() SUCCESS");
        }
    }

    void t_constexpr_sqrt(){
        bool success = true;

#define t_constexpr_sqrt_display(VALUE) LOG_TRACE("%.10f %.10f", (float)constexpr_sqrt(VALUE), sqrt(VALUE));
#define t_constexpr_sqrt_unit(VALUE) success = success && almost_equal((float)constexpr_sqrt(VALUE), sqrt(VALUE), 0.0000001f, 1);

        t_constexpr_sqrt_unit(3.f);
        t_constexpr_sqrt_unit(11.f);
        t_constexpr_sqrt_unit(56.f);
        t_constexpr_sqrt_unit(78.f);
        t_constexpr_sqrt_unit(110.f);
        t_constexpr_sqrt_unit(1510.f);
        t_constexpr_sqrt_unit(10256.f);
        t_constexpr_sqrt_unit(198256.f);
        t_constexpr_sqrt_unit(1980256.f);
        t_constexpr_sqrt_unit(12030256.f);
        t_constexpr_sqrt_unit(112030256.f);
        t_constexpr_sqrt_unit(1009128256.f);

#undef t_constexpr_sqrt_display
#undef t_constexpr_sqrt_unit

        if(!success){
            LOG_ERROR("utest::t_constexpr_sqrt() FAILED");
        }else{
            LOG_INFO("utest::t_constexpr_sqrt() SUCCESS");
        }
    }

    void t_Dense_Grid(){
        bool success = true;

        Dense_Grid<u32> grid;
        grid.set_dimensions(5u, 5u);

        success = success && grid.origin.x == 0 && grid.origin.y == 0 && grid.size_x == 5u && grid.size_y == 5u;

        for(s32 y = 4; y >= 0; --y){
            for(s32 x = 0; x <= 4; ++x){
                grid.at(x, y) = 10 * x + y;
            }
        }

        grid.extend_to_fit({-1, 7});

        success = success && grid.origin.x == -1 && grid.origin.y == 0 && grid.size_x == 6u && grid.size_y == 8u;

        for(s32 y = 7; y >= 0; --y){
            for(s32 x = -1; x <= 4; ++x){
                if(x >= 0 && x <= 4 && y >= 0 && y <= 4){
                    success = success && grid.at(x, y) == 10 * x + y;
                }else{
                    success = success && grid.at(x, y) == 0;
                }
            }
        }

        if(!success){
            LOG_ERROR("utest::t_Dense_Grid() FAILED");
        }else{
            LOG_INFO("utest::t_Dense_Grid() SUCCESS");
        }
    }
}

int main(int argc, char* argv[]){
    //bw::utest::t_find_noise_magic_normalizer();
    //bw::utest::t_detect_vector_capacilities();

    // ---- regression tests

    bw::utest::t_quat_rot();
    bw::utest::t_defer();
    bw::utest::t_darena();
    bw::utest::t_darray();
    bw::utest::t_dring();
    bw::utest::t_dpool();
    bw::utest::t_dhashmap();
    bw::utest::t_dhashmap_randomized();
    bw::utest::t_daryheap();
    bw::utest::t_align();
    bw::utest::t_isort();
    bw::utest::t_binsearch();
    bw::utest::t_constexpr_sqrt();
    bw::utest::t_Dense_Grid();

    return 0;
}
