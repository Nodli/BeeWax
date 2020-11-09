namespace BEEWAX_INTERNAL{
    template<typename T, s32 (*function)(const T& A, const T&B)>
    s32 qsort_comparison_wrapper(const void* A, const void* B){
        return function(*(T*)A, *(T*)B);
    }

    template<typename T>
    s32 comparison_increasing_order(const T& A, const T& B){
        return (A > B) - (A < B);
    }

    template<typename T>
    s32 comparison_decreasing_order(const T& A, const T& B){
        return - comparison_increasing_order<T>(A, B);
    }
}

template<typename T, s32 (*compare)(const T& A, const T& B)>
void qsort(T* data, u32 size){
    ::qsort((void*)data, size, sizeof(T), &BEEWAX_INTERNAL::qsort_comparison_wrapper<T, compare>);
}

template<typename T, s32 (*compare)(const T& A, const T& B)>
void isort(T* data, u32 size){
    for(u32 icurrent = 1u; icurrent < size; ++icurrent){
        u32 current_position = icurrent;
        while(current_position != 0u && compare(data[current_position - 1u], data[current_position]) > 0){
            swap(data[current_position - 1u], data[current_position]);
            --current_position;
        }
    }
}

template<typename T>
T* linsearch_lower(T* data, u32 size, const T& value){
    for(u32 idata = 0; idata != size; ++idata){
        if(data[idata] == value){
            return data + idata;
        }
    }
    return nullptr;
}

namespace BEEWAX_INTERNAL{
    // NOTE(hugo): returns the insertion poinclosest element that is < /value/
    // ie may return (data + size) that is outside of data when there is no element >= /value/ in /data/
    template<typename T>
    static inline T* bininsert_lower_internal(T* data, u32 size, const T& value){
        T* range_start = data;
        u32 range_size = size;

        while(range_size != 0u){
            u32 range_half_offset = range_size / 2u;
            T* range_half = range_start + range_half_offset;

            if(*range_half < value){
                range_start = range_half;
                ++range_start;

                // NOTE(hugo):
                // (A) range_size + 1u > range_size
                // (B) range_size > range_size / 2u
                //     unless range_size == 0u or range_size == 1u
                // (A + B - 1u) range_size > range_size - 1u > range_size / 2u - 1u
                //     unless range_size == 0u or range_size == 1u
                // - range_size != 0u
                // - range_size == 1u means range_size - range_size / 2u - 1u = 0u
                // ie range_size can be an unsigned integer
                range_size = range_size - range_half_offset - 1u;
            }else{

                range_size = range_half_offset;
            }
        }

        return range_start;
    }
}

template<typename T>
T* binsearch_lower(T* data, u32 size, const T& value){
    T* insertion_point = BEEWAX_INTERNAL::bininsert_lower_internal(data, size, value);
    if(insertion_point < data + size && *insertion_point == value){
        return insertion_point;
    }
    return nullptr;
}

template<typename T>
u32 lininsert_lower(const T* data, u32 size, const T& value){
    u32 insert_index = 0u;
    while(insert_index < size){
        if(value <= data[insert_index]){
            break;
        }

        ++insert_index;
    }
    return insert_index;
}

// NOTE(hugo): pointer substraction is defined if both pointers point to elements of the same array or one past the end
template<typename T>
u32 bininsert_lower(const T* data, u32 size, const T& value){
    T* insertion_point = BEEWAX_INTERNAL::bininsert_lower_internal(data, size, value);
    return insertion_point - data;
}
