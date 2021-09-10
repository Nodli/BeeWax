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

