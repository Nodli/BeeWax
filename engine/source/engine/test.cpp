#include <memory> // NOTE(hugo): t_align compares results with std::align

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
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (extension->extension_bytesize == sizeof(u32) && extension->next == nullptr);
        }

        u8* vu8 = (u8*)arena.push<u8>();
        success &= (vu8 != nullptr && (void*)vu8 == (void*)((u8*)vs32 + 4u));
        *vu8 = 4u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (extension->extension_bytesize == sizeof(u32) && extension->next == nullptr);
        }

        u64* vu64 = (u64*)arena.push<u64>();
        success &= (vu64 != nullptr);
        *vu64 = 5u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (extension->extension_bytesize == sizeof(u64) && extension->next != nullptr);
            extension = (darena::extension_header*)extension->next;
            success &= (extension->extension_bytesize == sizeof(u32) && extension->next == nullptr);
        }

        u8* v2u8 = (u8*)arena.push<u8>();
        success &= (v2u8 != nullptr && (void*)v2u8 == (void*)((u8*)vu8 + 1u));
        *v2u8 = 6u;
        success &= (arena.memory && arena.memory_bytesize == 10u && arena.position == sizeof(u32) + sizeof(s32) + sizeof(u8) + sizeof(u8));
        success &= (arena.extension_head != nullptr);
        {
            darena::extension_header* extension = (darena::extension_header*)arena.extension_head;
            success &= (extension->extension_bytesize == sizeof(u64) && extension->next != nullptr);
            extension = (darena::extension_header*)extension->next;
            success &= (extension->extension_bytesize == sizeof(u32) && extension->next == nullptr);
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

    void t_dqueue(){
        bool success = true;

        dqueue<u32> queue;

        queue.push_back(1u);
        success &= (queue.data && queue.size == 1u && queue.capacity == 2u && queue.head_index == 0u);
        success &= (queue[0] == 1u);

        queue.push_front(2u);
        success &= (queue.data && queue.size == 2u && queue.capacity == 2u && queue.head_index == 1u);
        success &= (queue[0] == 2u);
        success &= (queue[1] == 1u);

        queue.pop_back();
        success &= (queue.data && queue.size == 1u && queue.capacity == 2u && queue.head_index == 1u);
        success &= (queue[0] == 2u);

        queue.set_min_capacity(3u);
        success &= (queue.data && queue.size == 1u && queue.capacity == 3u && queue.head_index == 1u);
        success &= (queue[0] == 2u);

        queue.push_back(3u);
        queue.push_back(4u);
        success &= (queue.data && queue.size == 3u && queue.capacity == 3u && queue.head_index == 1u);
        success &= (queue[0] == 2u);
        success &= (queue[1] == 3u);
        success &= (queue[2] == 4u);

        queue.push_back(5u);
        success &= (queue.data && queue.size == 4u && queue.capacity == 6u && queue.head_index == 4u);
        success &= (queue[0] == 2u);
        success &= (queue[1] == 3u);
        success &= (queue[2] == 4u);
        success &= (queue[3] == 5u);

        queue.push_front(6u);
        queue.push_front(7u);
        success &= (queue.data && queue.size == 6u && queue.capacity == 6u && queue.head_index == 2u);
        success &= (queue[0] == 7u);
        success &= (queue[1] == 6u);
        success &= (queue[2] == 2u);
        success &= (queue[3] == 3u);
        success &= (queue[4] == 4u);
        success &= (queue[5] == 5u);

        queue.push_front(8u);
        success &= (queue.data && queue.size == 7u && queue.capacity == 12u && queue.head_index == 7u);
        success &= (queue[0] == 8u);
        success &= (queue[1] == 7u);
        success &= (queue[2] == 6u);
        success &= (queue[3] == 2u);
        success &= (queue[4] == 3u);
        success &= (queue[5] == 4u);
        success &= (queue[6] == 5u);

        queue.clear();
        success &= (queue.data && queue.size == 0u && queue.capacity == 12u && queue.head_index == 0u);

        queue.free();
        success &= (queue.data == nullptr && queue.size == 0u && queue.capacity == 0u && queue.head_index == 0u);

        if(!success){
            LOG_ERROR("utest::t_dqueue() FAILED");
        }else{
            LOG_INFO("utest::t_dqueue() SUCCESS");
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

    void t_dhashmap(){
        bool success = true;

        dhashmap<s32, s32> hmap;

        {
            s32* search = hmap.search(1);
            success &= (hmap.table == nullptr && hmap.table_capacity_minus_one == 0u);
            success &= (hmap.nentries == 0u && hmap.storage.memory == nullptr && hmap.storage.capacity == 0u && hmap.storage.get_first() == hmap.storage.capacity);
            success &= (search == nullptr);
        }

        {
            s32* get = hmap.get(1);
            success &= (get != nullptr);
            *get = -1;
            success &= (hmap.table != nullptr && hmap.table_capacity_minus_one == 1u);
            success &= (hmap.nentries == 1u && hmap.storage.memory != nullptr && hmap.storage.capacity == 2u && hmap.storage.get_first() == 0u);

            s32* search = hmap.search(1);
            success &= (search == get && *search == -1);

            u32 iter = hmap.storage.get_first();
            success &= (iter == 0u);
            iter = hmap.storage.get_next(iter);
            success &= (iter == hmap.storage.capacity);

            s32* get_second = hmap.get(1);
            success &= (get_second == get && *get_second == -1);
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
            *hmap.get(2) = -2;
            *hmap.get(3) = -3;
            *hmap.get(4) = -4;

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

        if(!success){
            LOG_ERROR("utest::t_dhashmap() FAILED");
        }else{
            LOG_INFO("utest::t_dhashmap() SUCCESS");
        }
    }

    void t_align(){
        bool success = true;

        u64 seed;
        seed_random_with_time(seed);
        u32 ntest = 16;
        u32 max_alignment_pow2 = 7; // 2^7 == 128

        for(u32 itest = 0u; itest != ntest; ++itest){
            u64 random = random_u64(seed);
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

#if 0
    void t_heap(){
        bool success = true;

        Min_Heap<s32, s32> heap;
        heap.push(2, 1);
        heap.push(1, 0);
        LOG_Binary_Heap(&heap);
        heap.push(1, 7);
        LOG_Binary_Heap(&heap);
        heap.push(2, 6);
        heap.push(4, 2);
        LOG_Binary_Heap(&heap);
        heap.push(8, 5);
        heap.push(6, 3);
        LOG_Binary_Heap(&heap);
        heap.push(3, 4);
        heap.push(3, 8);
        LOG_Binary_Heap(&heap);

        if(!success){
            LOG_ERROR("utest::t_heap() FAILED");
        }else{
            LOG_INFO("utest::t_heap() SUCCESS");
        }
    }
#endif

    void t_detect_vector_capacilities(){
        s32 vector_capabilities = detect_vector_capabilities();
        LOG_INFO("SSE       %s", vector_capabilities > 0 ? "YES" : "NO");
        LOG_INFO("SSE2      %s", vector_capabilities > 1 ? "YES" : "NO");
        LOG_INFO("SSE3      %s", vector_capabilities > 2 ? "YES" : "NO");
        LOG_INFO("SSSE3     %s", vector_capabilities > 3 ? "YES" : "NO");
        LOG_INFO("SSE4.1    %s", vector_capabilities > 4 ? "YES" : "NO");
        LOG_INFO("SSE4.2    %s", vector_capabilities > 5 ? "YES" : "NO");
    }
}

int main(){
    bw::utest::t_quat_rot();
    bw::utest::t_defer();
    bw::utest::t_darena();
    bw::utest::t_darray();
    bw::utest::t_dqueue();
    bw::utest::t_dpool();
    bw::utest::t_dhashmap();
    bw::utest::t_align();
    //bw::utest::t_find_noise_magic_normalizer();
    //bw::utest::t_heap();
    bw::utest::t_detect_vector_capacilities();
}
