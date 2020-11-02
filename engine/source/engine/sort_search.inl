template<typename T>
int qsort_compare_increase(const void* A, const void* B){
    T* typedA = (T*)A;
    T* typedB = (T*)B;
    return (int)(*typedA - *typedB);
}

template<typename T>
int qsort_compare_decrease(const void* A, const void* B){
    T* typedA = (T*)A;
    T* typedB = (T*)B;
    return (int)(*typedB - *typedA);
}

template<template<int (*Compare)(const T& A, const T& B)>, typename T>
void qsort(T* data, u32 size){
    qsort((void*)data, size, sizeof(T), &Compare);
}

template<typename T>
T* search_linear(T* data, u32 size, T* value){
    for(u32 idata = 0; idata != size; ++idata){
        if(*(data + idata) == *value){
            return data + idata;
        }
    }
    return nullptr;
}

template<typename T>
T* search_binary(T* data, u32 size, T* value){
    u32 imin = 0;
    u32 imax = size - 1;
    while(imin <= imax){
        u32 imiddle = (imin + imax) / 2;
        if(*(data + imiddle) > *value){
            imax = imiddle - 1;
        }else if(*(data + imiddle) < *value){
            imin = imiddle + 1;
        }else{
            return data + imiddle;
        }
    }
    return nullptr;
}

template<typename T>
T* search_binary_left(T* data, u32 size, T* value){
    u32 imin = 0;
    u32 imax = size - 1;
    while(imin <= imax){
        u32 imiddle = (imin + imax) / 2;
        if(*(data + imiddle) >= *value){
            imax = imiddle - 1;
        }else{
            imin = imiddle + 1;
        }
    }
    return imin;
}

template<typename T>
T* search_binary_right(T* data, u32 size, T* value){
    u32 imin = 0;
    u32 imax = size - 1;
    while(imin <= imax){
        u32 imiddle = (imin + imax) / 2;
        if(*(data + imiddle) > *value){
            imax = imiddle - 1;
        }else{
            imin = imiddle + 1;
        }
    }
    return imin;
}
