#include <memory>       // NOTE(hugo): t_align compares results with std::align

// NOTE(hugo): data structures tests require min_..._capacity = 2u

using namespace bw;

namespace bw::utest{

    constexpr float ftolerance = 0.000001f;
    constexpr float dtolerance = 0.00000000000001f;

    bool equal_tolerance(float vA, float vB, float tolerance = ftolerance){
        return !(abs(vA - vB) > tolerance);
    }
    bool equal_tolerance_rad(float vA, float vB, float tolerance = ftolerance){
        float diffA = abs(vA - vB);
        float diffB = abs(2.f * PI - diffA);

        float diff = min(diffA, diffB);

        return !(diff > tolerance);
    }

    // ---- regression tests

    void t_Virtual_Arena(){
        bool success = true;

        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

        Virtual_Arena arena;
        arena.initialize(GIGABYTES(64u));

        {
            Virtual_Arena_Memory mem = arena.malloc(16u * sizeof(int), alignof(int));

            success &= mem.ptr && mem.previous_cursor == 0u && arena.commit_page_count == 1u && arena.cursor == 16u * sizeof(int);
            memset(mem.ptr, 0u, 16u * sizeof(int));

            arena.free(mem);
            success &= arena.commit_page_count == 1u && arena.cursor == 0u;
        }
        arena.reset();
        success &= arena.commit_page_count == 0u && arena.cursor == 0u;

        {
            Virtual_Arena_Memory memA = arena.malloc(KILOBYTES(7u), alignof(char));
            success &= memA.ptr && arena.commit_page_count == 2u;

            Virtual_Arena_Memory memB = arena.malloc(KILOBYTES(7u), alignof(char));
            success &= memB.ptr && arena.commit_page_count == 4u;

            arena.free(memB);
            success &= memB.ptr && arena.commit_page_count == 4u;

            arena.reset_to_cursor();
            success &= memB.ptr && arena.commit_page_count == 2u;

            // NOTE(hugo): intentionally not freeing memA to test the reset()
        }
        arena.reset();
        success &= arena.commit_page_count == 0u && arena.cursor == 0u;

        {
            Virtual_Arena_Memory malloc_tracker[128u];
            malloc_tracker[0u].previous_cursor = 0u;

            for(u32 ialloc = 1u; ialloc != 128u; ++ialloc){
                u32 nbytes = random_u32_range_uniform(KILOBYTES(16u)) * ialloc;

                size_t bytesize = nbytes * sizeof(char);
                Virtual_Arena_Memory mem = arena.malloc(bytesize, 4u);
                malloc_tracker[ialloc] = mem;

                uintptr_t arena_bytesize = (uintptr_t)mem.ptr - (uintptr_t)arena.vmemory + bytesize;
                size_t arena_page_count = arena_bytesize / BEEWAX_INTERNAL::vmemory_pagesize
                    + (arena_bytesize % BEEWAX_INTERNAL::vmemory_pagesize != 0u);

                success &= mem.ptr
                    && arena.commit_page_count == arena_page_count
                    && arena.cursor == arena_bytesize;
                memset(mem.ptr, 0u, bytesize);
            }
            for(u32 ialloc = 127u; ialloc != 0u; --ialloc){
                arena.free(malloc_tracker[ialloc]);
            }
        }
        arena.reset();
        success &= arena.commit_page_count == 0u && arena.cursor == 0u;

        arena.terminate();

        if(!success){
            LOG_ERROR("FAILED utest::t_Virtual_Arena() - seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
            LOG_ERROR("FAILED utest::t_Virtual_Arena()");
        }else{
            LOG_INFO("FINISHED utest::t_Virtual_Arena()");
        }
    }

    void t_array(){
        bool success = true;

        array<s32, contiguous_storage<s32>> array;

        array.set_min_capacity(2);
        success &= (array.size == 0 && array.storage.capacity == 2);

        array.push(1);
        array.push(2);
        success &= (array.size == 2 && array.storage.capacity == 2 && array[0] == 1 && array[1] == 2);

        array.set_min_capacity(4);
        success &= (array.size == 2 && array.storage.capacity == 4 && array[0] == 1 && array[1] == 2);

        array.insert(1, 3);
        success &= (array.size == 3 && array.storage.capacity == 4 && array[0] == 1 && array[1] == 3 && array[2] == 2);

        array.set_min_capacity(8);
        success &= (array.size == 3 && array.storage.capacity == 8);

        array.insert_multi(1, 2);
        array[1] = 4;
        array[2] = 5;
        success &= (array.size == 5 && array.storage.capacity == 8 && array[0] == 1 && array[1] == 4 && array[2] == 5 && array[3] == 3 && array[4] == 2);

        array.remove(3);
        success &= (array.size == 4 && array.storage.capacity == 8 && array[0] == 1 && array[1] == 4 && array[2] == 5 && array[3] == 2);

        array.remove_multi(2, 2);
        success &= (array.size == 2 && array.storage.capacity == 8 && array[0] == 1 && array[1] == 4);

        array.remove_swap(0);
        success &= (array.size == 1 && array.storage.capacity == 8 && array[0] == 4);

        array.push(7);
        array.push(8);
        array.push(9);
        array.push(10);
        success &= (array.size == 5 && array.storage.capacity == 8 && array[0] == 4 && array[1] == 7 && array[2] == 8 && array[3] == 9 && array[4] == 10);

        u32 counter = 0u;
        for(auto iter : array){
            success &= array[counter++] == iter;
        }
        counter = 0u;
        for(auto& iter : array){
            success &= array[counter++] == iter;
        }
        counter = 0u;
        for(const auto iter : array){
            success &= array[counter++] == iter;
        }
        counter = 0u;
        for(const auto& iter : array){
            success &= array[counter++] == iter;
        }

        array.pop();
        success &= (array.size == 4 && array.storage.capacity == 8 && array[0] == 4 && array[1] == 7 && array[2] == 8 && array[3] == 9);

        array.pop();
        array.pop();
        success &= (array.size == 2 && array.storage.capacity == 8 && array[0] == 4 && array[1] == 7);

        array.set_size(3);
        success &= (array.size == 3 && array.storage.capacity == 8 && array[0] == 4 && array[1] == 7);

        array.set_size(9);
        success &= (array.size == 9 && array.storage.capacity == 9 && array[0] == 4 && array[1] == 7);

        array.set_min_capacity(12);
        success &= (array.size == 9 && array.storage.capacity == 12 && array[0] == 4 && array[1] == 7);

        array.set_min_capacity(10);
        success &= (array.size == 9 && array.storage.capacity == 12 && array[0] == 4 && array[1] == 7);

        array.push(11);
        array.push(12);
        array.push(13);
        array.push(14);
        success &= (array.size == 13 && array.storage.capacity == 32 && array[0] == 4 && array[1] == 7);

        array.clear();
        success &= (array.size == 0 && array.storage.capacity == 32);

        array.free();

        array.set_min_capacity(5);
        success &= (array.size == 0 && array.storage.capacity == 5);

        array.push(11u);
        success &= (array.size == 1 && array.storage.capacity == 5 && array[0] == 11u);

        array.free();

        if(!success){
            LOG_ERROR("FAILED utest::t_array()");
        }else{
            LOG_INFO("FINISHED utest::t_array()");
        }
    }

    void t_pool(){
        bool success = true;

        pool<u32, bucketized_storage<Pool_Bucket<u32>, 32u>> pool;
        success &= (pool.size == 0u && pool.storage.capacity == 0 && pool.head_bucket == freelist_no_bucket);

        pool.set_min_capacity(1u);
        success &= (pool.size == 0u && pool.storage.capacity == 32u && pool.head_bucket == 0u);

        pool.insert(0u);
        success &= (pool.size == 1u && pool.storage.capacity == 32u && pool.head_bucket == 1u);
        success &= (pool[0u] == 0u);

        pool.insert(1u);
        success &= (pool.size == 2u && pool.storage.capacity == 32u && pool.head_bucket == 2u);
        {
            success &= (pool[0u] == 0u);
            success &= (pool[1u] == 1u);
        }

        pool.insert(2u);
        success &= (pool.size == 3u && pool.storage.capacity == 32u && pool.head_bucket == 3u);
        {
            success &= (pool[0u] == 0u);
            success &= (pool[1u] == 1u);
            success &= (pool[2u] == 2u);
        }

        pool.insert(3u);
        success &= (pool.size == 4u && pool.storage.capacity == 32u && pool.head_bucket == 4u);
        {
            success &= (pool[0u] == 0u);
            success &= (pool[1u] == 1u);
            success &= (pool[2u] == 2u);
            success &= (pool[3u] == 3u);
        }

        pool.remove(2u);
        success &= (pool.size == 3u && pool.storage.capacity == 32u && pool.head_bucket == 2u);
        {
            success &= (pool[0u] == 0u);
            success &= (pool[1u] == 1u);
            success &= (pool[3u] == 3u);
        }

        pool.remove(0u);
        success &= (pool.size == 2u && pool.storage.capacity == 32u && pool.head_bucket == 0u);
        {
            success &= (pool[1u] == 1u);
            success &= (pool[3u] == 3u);
        }

        pool.insert(4u);
        success &= (pool.size == 3u && pool.storage.capacity == 32u && pool.head_bucket == 2u);
        {
            success &= (pool[0u] == 4u);
            success &= (pool[1u] == 1u);
            success &= (pool[3u] == 3u);
        }

        pool.remove(3u);
        success &= (pool.size == 2u && pool.storage.capacity == 32u && pool.head_bucket == 3u);
        {
            success &= (pool[0u] == 4u);
            success &= (pool[1u] == 1u);
        }

        pool.clear();
        success &= (pool.size == 0u && pool.storage.capacity == 32u && pool.head_bucket == 0u);

        pool.insert(5u);
        success &= (pool.size == 1u && pool.storage.capacity == 32u && pool.head_bucket == 1u);
        success &= pool[0u] == 5u;

        pool.free();
        success &= (pool.size == 0u && pool.storage.capacity == 0u && pool.head_bucket == freelist_no_bucket);

        pool.set_min_capacity(33u);
        success &= (pool.size == 0u && pool.storage.capacity == 64u && pool.head_bucket == 0u);

        pool.free();

        if(!success){
            LOG_ERROR("FAILED utest::t_pool()");
        }else{
            LOG_INFO("FINISHED utest::t_pool()");
        }
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
            for(auto& iter : hmap){
                u32 index = lininsert_lower<u32>(keys, 1u, iter.key());
                success &= keys[index] == iter.key() && values[index] == iter.value();
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
            for(auto& iter : hmap){
                u32 index = lininsert_lower<u32>(keys, 3u, iter.key());
                success &= keys[index] == iter.key() && values[index] == iter.value();
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
            for(auto& iter : hmap){
                u32 index = lininsert_lower<u32>(keys, 2u, iter.key());
                success &= keys[index] == iter.key() && values[index] == iter.value();
                ++counter;
            }
        }

        hmap.free();

        if(!success){
            LOG_ERROR("FAILED utest::t_dhashmap()");
        }else{
            LOG_INFO("FINISHED utest::t_dhashmap()");
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
                            assert(hmap_counter);
                            success &= hmap_counter && *hmap_counter == status_counter[random_key];
                            ++(*hmap_counter);
                            ++status_counter[random_key];
                            assert(success);
                            break;
                        }
                    case 1:
                        {
                            s32* hmap_counter = hmap.search(random_key);
                            if(hmap_counter){
                                success &= status_counter[random_key] > 0 && *hmap_counter == status_counter[random_key];
                                ++(*hmap_counter);
                                ++status_counter[random_key];
                            }else{
                                success &= status_counter[random_key] == 0;
                            }
                            assert(success);
                            break;
                        }
                    case 2:
                        {
                            hmap.remove(random_key);
                            s32* searched = hmap.search(random_key);
                            success &= searched == nullptr;
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
            LOG_ERROR("FAILED utest::t_dhashmap_randomized() - seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
        }else{
            LOG_INFO("FINISHED utest::t_dhashmap_randomized()");
        }
    }

    void t_quat_rot(){
        vec3 v = {1., 1., 1.};
        vec3 pA = {1., 0., 0.};
        vec3 pB = {0., 1., 0.};

        vec3 norm = cross(pA, pB);
        quat q = quat_from_axis(PI / 2.f, norm);
        vec3 vquat = rotated(v, q);

        vec3 biv = wedge(pA, pB);
        rot3 r = make_rot(PI / 2.f, biv);
        vec3 vrot = rotated(v, r);

        if(!almost_equal(vquat.x, vrot.x, ftolerance, 1) && almost_equal(vquat.y, vrot.y, ftolerance, 1) && almost_equal(vquat.z, vrot.z, ftolerance, 1)){
            LOG_ERROR("FAILED utest::t_quat_rot()");
        }else{
            LOG_INFO("FINISHED utest::t_quat_rot()");
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
            LOG_ERROR("FAILED utest::t_defer()");
        }else{
            LOG_INFO("FINISHED utest::t_defer()");
        }
    }

    void t_align(){
        bool success = true;

        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

        constexpr u32 ntest = 16;
        constexpr u32 max_alignment_pow2 = 7; // 2^7 == 128

        for(u32 itest = 0u; itest != ntest; ++itest){
            u64 random = random_u64();
            void* adress = (void*)random;

            for(u32 ialign = 1u; ialign != max_alignment_pow2; ++ialign){
                size_t alignment = (size_t)1 << ialign;
                void* aligned_up = align_next(adress, alignment);
                void* aligned_down = align_prev(adress, alignment);

                void* adress_copy = adress;
                size_t size = 16;
                //std::align(alignment, (size_t)UINT64_MAX, adress_copy, size);

                success &= (((u64)aligned_up % (u64)alignment) == 0u) && ((u64)aligned_up >= random) && (((u64)aligned_up - random) < (u64)alignment);
                success &= (((u64)aligned_down % (u64)alignment) == 0u) && ((u64)aligned_down <= random) && ((random - (u64)aligned_down) < (u64)alignment);
            }
        }

        if(!success){
            LOG_ERROR("FAILED utest::t_align()- seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
        }else{
            LOG_INFO("FINISHED utest::t_align()");
        }
    }

    void t_isort(){
        bool success = true;

        constexpr u32 ntest = 10u;
        constexpr u32 array_size = 50u;

        s32 array_isort[array_size];

        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

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
            LOG_ERROR("FAILED utest::t_isort()- seed: %" PRId64 " %", PRId64, seed_copy.s0, seed_copy.s1);
        }else{
            LOG_INFO("FINISHED utest::t_isort()");
        }
    }

    void t_binsearch(){
        bool success = true;

        constexpr u32 ntest = 10u;
        constexpr u32 array_size = 50u;

        s32 array[array_size];
        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

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
            LOG_ERROR("FAILED utest::t_binsearch() - seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
        }else{
            LOG_INFO("FINISHED utest::t_binsearch()");
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
            LOG_ERROR("FAILED utest::t_constexpr_sqrt()");
        }else{
            LOG_INFO("FINISHED utest::t_constexpr_sqrt()");
        }
    }

    void t_Dense_Grid(){
        bool success = true;

        Dense_Grid<u32> grid;
        grid.set_dimensions(5u, 5u);

        success = success && grid.origin.x == 0 && grid.origin.y == 0 && grid.size_x == 5u && grid.size_y == 5u;

        auto display_grid = [&](){
            LOG_TRACE("origin: %d %d size: %d %d", grid.origin.x, grid.origin.y, grid.size_x, grid.size_y);
            for(s32 y = grid.size_y - 1; y >= 0; --y){
                for(s32 x = 0; x < grid.size_x; ++x){
                    printf("%d ", grid.at(grid.origin.x + x, grid.origin.y + y));
                }
                printf("\n");
            }
            printf("\n");
        };

        for(s32 y = 0; y < 5; ++y){
            for(s32 x = 0; x < 5; ++x){
                grid.at(x, y) = 10 * x + y;
            }
        }

        grid.extend_to_fit({-1, 7});
        success = success && grid.origin.x == -1 && grid.origin.y == 0 && grid.size_x == 6u && grid.size_y == 8u;

        for(s32 y = 0; y < 8; ++y){
            for(s32 x = -1; x < 5; ++x){
                if(x >= 0 && x <= 4 && y >= 0 && y <= 4){
                    success = success && grid.at(x, y) == 10 * x + y;
                }else{
                    success = success && grid.at(x, y) == 0;
                }
            }
        }

        grid.free();

        if(!success){
            LOG_ERROR("FAILED utest::t_Dense_Grid()");
        }else{
            LOG_INFO("FINISHED utest::t_Dense_Grid()");
        }
    }

    void t_coord_conversion(){
        bool success = true;

        // NOTE(hugo): spherical - cartesian
        {
            vec3 coord;
            vec2 other_coord;

            coord = spherical_to_cartesian(0., 0.);
            success = success && equal_tolerance(coord.x, 0.f) && equal_tolerance(coord.y, 1.f) && equal_tolerance(coord.z, 0.f);
            other_coord = cartesian_to_spherical(coord.x, coord.y, coord.z);
            success = success && equal_tolerance_rad(other_coord.x, 0.f) && equal_tolerance_rad(other_coord.y, 0.f);

            coord = spherical_to_cartesian(PI, 0.);
            success = success && equal_tolerance(coord.x, 0.f) && equal_tolerance(coord.y,-1.f) && equal_tolerance(coord.z, 0.f);
            other_coord = cartesian_to_spherical(coord.x, coord.y, coord.z);
            success = success && equal_tolerance_rad(other_coord.x, PI) && equal_tolerance_rad(other_coord.y, 0.f);

            coord = spherical_to_cartesian(PI / 2., 0.);
            success = success && equal_tolerance(coord.x, 0.f) && equal_tolerance(coord.y, 0.f) && equal_tolerance(coord.z, 1.f);
            other_coord = cartesian_to_spherical(coord.x, coord.y, coord.z);
            success = success && equal_tolerance_rad(other_coord.x, PI / 2.) && equal_tolerance_rad(other_coord.y, 0.f);

            coord = spherical_to_cartesian(PI / 2., PI / 2.);
            success = success && equal_tolerance(coord.x, 1.f) && equal_tolerance(coord.y, 0.f) && equal_tolerance(coord.z, 0.f);
            other_coord = cartesian_to_spherical(coord.x, coord.y, coord.z);
            success = success && equal_tolerance_rad(other_coord.x, PI / 2.) && equal_tolerance_rad(other_coord.y, PI / 2.);

            coord = spherical_to_cartesian(PI / 2., PI * 3. / 2.);
            success = success && equal_tolerance(coord.x, -1.f) && equal_tolerance(coord.y, 0.f) && equal_tolerance(coord.z, 0.f);
            other_coord = cartesian_to_spherical(coord.x, coord.y, coord.z);
            success = success && equal_tolerance(other_coord.x, PI / 2.) && equal_tolerance(other_coord.y, PI * 3. / 2.);
        }

        // NOTE(hugo): spherical - equirectangular
        {
            vec2 converted;
            vec2 reconverted;

            converted = spherical_to_equirectangular(0.f, 0.f);
            success = success && equal_tolerance(converted.x, 0.f) && equal_tolerance(converted.y, 1.f);
            reconverted = equirectangular_to_spherical(converted.x, converted.y);
            success = success && equal_tolerance_rad(reconverted.x, 0.f) && equal_tolerance_rad(reconverted.y, 0.f);

            converted = spherical_to_equirectangular(PI, 0.f);
            success = success && equal_tolerance(converted.x, 0.f) && equal_tolerance(converted.y, 0.f);
            reconverted = equirectangular_to_spherical(converted.x, converted.y);
            success = success && equal_tolerance_rad(reconverted.x, PI) && equal_tolerance_rad(reconverted.y, 0.f);

            converted = spherical_to_equirectangular(PI / 2.f, PI);
            success = success && equal_tolerance(converted.x, 1.f) && equal_tolerance(converted.y, 0.5f);
            reconverted = equirectangular_to_spherical(converted.x, converted.y);
            success = success && equal_tolerance_rad(reconverted.x, PI / 2.f) && equal_tolerance_rad(reconverted.y, PI);

            converted = spherical_to_equirectangular(PI / 2.f, 2.f * PI);
            success = success && equal_tolerance(converted.x, 2.f) && equal_tolerance(converted.y, 0.5f);
            reconverted = equirectangular_to_spherical(converted.x, converted.y);
            success = success && equal_tolerance_rad(reconverted.x, PI / 2.f) && equal_tolerance_rad(reconverted.y, 2.f * PI);
        }

        // NOTE(hugo): cartesian - cubemap
        {
            u32 fi;
            vec2 conv;
            vec3 reconv;

            conv = cartesian_to_cubemap(1.f, 0.f, 0.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 0u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, 1.f) && equal_tolerance(reconv.y, 0.f) && equal_tolerance(reconv.z, 0.f);

            conv = cartesian_to_cubemap(-1.f, 0.f, 0.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 1u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, -1.f) && equal_tolerance(reconv.y, 0.f) && equal_tolerance(reconv.z, 0.f);

            conv = cartesian_to_cubemap(0.f, 1.f, 0.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 2u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, 0.f) && equal_tolerance(reconv.y, 1.f) && equal_tolerance(reconv.z, 0.f);

            conv = cartesian_to_cubemap(0.f, -1.f, 0.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 3u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, 0.f) && equal_tolerance(reconv.y, -1.f) && equal_tolerance(reconv.z, 0.f);

            conv = cartesian_to_cubemap(0.f, 0.f, 1.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 4u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, 0.f) && equal_tolerance(reconv.y, 0.f) && equal_tolerance(reconv.z, 1.f);

            conv = cartesian_to_cubemap(0.f, 0.f, -1.f, fi);
            success = success && equal_tolerance(conv.x, 0.5f) && equal_tolerance(conv.y, 0.5f) && (fi == 5u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, 0.f) && equal_tolerance(reconv.y, 0.f) && equal_tolerance(reconv.z, -1.f);

            // ----

            vec3 temp;

            temp = normalized(vec3({1.f, 1.f, 1.f}));
            conv = cartesian_to_cubemap(temp.x, temp.y, temp.z, fi);
            success = success && equal_tolerance(conv.x, 1.f) && equal_tolerance(conv.y, 1.f) && (fi == 0u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, temp.x) && equal_tolerance(reconv.y, temp.y) && equal_tolerance(reconv.z, temp.z);

            temp = normalized(vec3({-1.f, -1.f, -1.f}));
            conv = cartesian_to_cubemap(temp.x, temp.y, temp.z, fi);
            success = success && equal_tolerance(conv.x, 1.f) && equal_tolerance(conv.y, 0.f) && (fi == 1u);
            reconv = cubemap_to_cartesian(conv.x, conv.y, fi);
            success = success && equal_tolerance(reconv.x, temp.x) && equal_tolerance(reconv.y, temp.y) && equal_tolerance(reconv.z, temp.z);
        }

        if(!success){
            LOG_ERROR("FAILED utest::t_geometry()");
        }else{
            LOG_INFO("FINISHED utest::t_geometry()");
        }
    }

    void t_triangulation_2D(){
        bool success = true;

        {
            vec2 vertices[] = {{-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f}, {-1.f, 1.f}};
            u32 indices[3u * (carray_size(vertices) - 2u)];
            triangulation_2D(carray_size(vertices), vertices, indices);

            u32 indices_expected[] = {
                3u, 0u, 1u,
                2u, 3u, 1u
            };
            success &= memcmp(indices_expected, indices, 2u * carray_size(vertices) - 2u) == 0u;
        }

        {
            vec2 vertices[] = {{0.f, 1.f}, {1.f, 0.f}, {0.f, 2.f}, {-1.f, 0.f}};
            u32 indices[3u * (carray_size(vertices) - 2u)];
            triangulation_2D(carray_size(vertices), vertices, indices);

            u32 indices_expected[] = {0u, 1u, 2u, 3u, 0u, 2u};
            success &= memcmp(indices_expected, indices, 2u * carray_size(vertices) - 2u) == 0u;
        }

        {
            vec2 vertices[] = {{-2.5f, 2.f}, {0.f, 0.f}, {2.75f, 2.5f}, {5.5f, 1.5f}, {8.f, 5.f}, {5.5f, 4.5f}, {3.5f, 6.f}, {1.5f, 2.f}, {-1.5f, 2.5f}, {-1.f, 5.f}};
            u32 indices[3u * (carray_size(vertices) - 2u)];
            triangulation_2D(carray_size(vertices), vertices, indices);

            u32 indices_expected_v0[] = {
                2, 3, 4,
                2, 4, 5,
                2, 5, 6,
                2, 6, 7,
                8, 9, 0,
                8, 0, 1,
                1, 2, 7,
                1, 7, 8,
            };
            u32 indices_expected_v1[] = {
                2, 3, 4,
                2, 4, 5,
                2, 5, 6,
                2, 6, 7,
                1, 2, 7,
                0, 1, 7,
                0, 7, 8,
                9, 0, 8
            };
            success &= memcmp(indices_expected_v1, indices, 2u * carray_size(vertices) - 2u) == 0u;
        }

        if(!success){
            LOG_ERROR("FAILED utest::t_triangulation_2D()");
        }else{
            LOG_INFO("FINISHED utest::t_triangulation_2D()");
        }
    }

    void t_hsv(){
        bool success = true;

        constexpr u32 ntest = 1000u;

        random_seed_with_time();
        random_seed_type seed_copy = random_seed_copy();

        for(u32 itest = 0u; itest != ntest; ++itest){
            vec4 rgba = {random_float(), random_float(), random_float(), 0.5f};
            vec4 hsva = rgba_to_hsva(rgba);
            vec4 re_rgba = hsva_to_rgba(hsva);
            success &= equal_tolerance(rgba.r, re_rgba.r) && equal_tolerance(rgba.g, re_rgba.g) && equal_tolerance(rgba.b, re_rgba.b) && (rgba.a == re_rgba.a);
            assert(success);
        }

        if(!success){
            LOG_ERROR("FAILED utest::t_hsv() - seed: %" PRId64 " %" PRId64, seed_copy.s0, seed_copy.s1);
            LOG_ERROR("FAILED utest::t_hsv()");
        }else{
            LOG_INFO("FINISHED utest::t_hsv()");
        }
    }

    void t_GJK(){
        bool success = true;

        cshape::Polygon polyA;
        polyA.nvertices = 4u;
        polyA.vertices = (vec2*)malloc(sizeof(vec2) * 4u);
        DEFER{ ::free(polyA.vertices); };
        polyA.vertices[0u] = {-1.f, -1.f};
        polyA.vertices[1u] = { 1.f, -1.f};
        polyA.vertices[2u] = { 1.f,  1.f};
        polyA.vertices[3u] = {-1.f,  1.f};

        cshape::Polygon polyB;
        polyB.nvertices = 4u;
        polyB.vertices = (vec2*)malloc(sizeof(vec2) * 4u);
        DEFER{ ::free(polyB.vertices); };
        polyB.vertices[0u] = { 0.f, -1.f};
        polyB.vertices[1u] = { 1.f,  0.f};
        polyB.vertices[2u] = { 0.f,  1.f};
        polyB.vertices[3u] = {-1.f,  0.f};

        cshape::Polygon polyC;
        polyC.nvertices = 3u;
        polyC.vertices = (vec2*)malloc(sizeof(vec2) * 3u);
        polyC.vertices[0u] = {2.f, -1.f};
        polyC.vertices[1u] = {2.f, 1.f};
        polyC.vertices[2u] = {1.5f, 0.f};


        success &= GJK(polyA, polyB);
        success &= !GJK(polyA, polyC);

        if(!success){
            LOG_ERROR("FAILED utest::t_GJK()");
        }else{
            LOG_INFO("FINISHED utest::t_GJK()");
        }
    }

    // ---- benchmark

    void t_detect_vector_capacilities(){
        s32 vector_capabilities = detect_vector_capabilities();
        LOG_INFO("SSE       %s", vector_capabilities > 0 ? "YES" : "NO");
        LOG_INFO("SSE2      %s", vector_capabilities > 1 ? "YES" : "NO");
        LOG_INFO("SSE3      %s", vector_capabilities > 2 ? "YES" : "NO");
        LOG_INFO("SSSE3     %s", vector_capabilities > 3 ? "YES" : "NO");
        LOG_INFO("SSE4.1    %s", vector_capabilities > 4 ? "YES" : "NO");
        LOG_INFO("SSE4.2    %s", vector_capabilities > 5 ? "YES" : "NO");
    }

    void t_find_noise_magic_normalizer(){
        random_seed_with_time();

        u32 nsamples = 100000000u;
        float scale = 10000.f;

        float max_value;
        float magic_normalizer;

        // NOTE(hugo): perlin_noise(const float x)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float value = perlin_noise(random_float() * scale);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() perlin_noise(x) max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): perlin_noise(const float x, const float y)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float x = random_float() * scale;
            float y = random_float() * scale;

            float value = perlin_noise(x, y);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() perlin_noise(x, y)  max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): simplex_noise(const float x)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float value = simplex_noise(random_float() * scale);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() simplex_noise(x) max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);

        // NOTE(hugo): simplex_noise(const float x, const float y)
        max_value = 0.f;
        for(u32 iter = 0u; iter != nsamples; ++iter){
            float x = random_float() * scale;
            float y = random_float() * scale;

            float value = simplex_noise(x, y);
            max_value = max(max_value, abs(value));
        }
        magic_normalizer = 1.f / max_value;
        LOG_INFO("utest::t_find_noise_magic_normalize() simplex_noise(x, y)  max_value: %f  MAGIC_NORMALIZER = %.10f", max_value, magic_normalizer);
    }

    void t_compare_triangulation_2D(){
        constexpr u32 nrun = 10000u;

        vec2 vertices[] = {{-2.5f, 2.f}, {0.f, 0.f}, {2.75f, 2.5f}, {5.5f, 1.5f}, {8.f, 5.f}, {5.5f, 4.5f}, {3.5f, 6.f}, {1.5f, 2.f}, {-1.5f, 2.5f}, {-1.f, 5.f}};
        u32 indices[3u * (carray_size(vertices) - 2u)];

        u64 timer_v0 = timer_ticks();

        for(u32 irun = 0u; irun != nrun; ++irun){
            triangulation_2D_v0(carray_size(vertices), vertices, indices);
        }

        u64 timer_v1 = timer_ticks();

        for(u32 irun = 0u; irun != nrun; ++irun){
            triangulation_2D_v1(carray_size(vertices), vertices, indices);
        }

        u64 timer_end = timer_ticks();

        LOG_INFO("triangulation_2D_v0: %" PRId64, timer_v1 - timer_v0);
        LOG_INFO("triangulation_2D_v1: %" PRId64, timer_end - timer_v1);
    }
}

int main(int argc, char* argv[]){

	SDL_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    setup_vmemory();
    setup_timer();
    setup_LOG();

    // ---- regression tests

    //bw::utest::t_Virtual_Arena();

    //bw::utest::t_array();
    //bw::utest::t_pool();
    //bw::utest::t_dhashmap();
    //bw::utest::t_dhashmap_randomized();

    //bw::utest::t_quat_rot();
    //bw::utest::t_defer();
    //bw::utest::t_align();
    //bw::utest::t_isort();
    //bw::utest::t_binsearch();
    //bw::utest::t_constexpr_sqrt();
    //bw::utest::t_Dense_Grid();

    //bw::utest::t_coord_conversion();
    //bw::utest::t_triangulation_2D();

    //bw::utest::t_hsv();

    bw::utest::t_GJK();

    // ---- benchmark

    //bw::utest::t_detect_vector_capacilities();
    //bw::utest::t_find_noise_magic_normalizer();
    //bw::utest::t_compare_triangulation_2D();

    // ----

    SDL_Quit();

    return 0;
}
