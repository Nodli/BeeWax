#ifndef H_SORT_SEARCH
#define H_SORT_SEARCH

template<typename T>
int qsort_compare_increase(const void* A, const void* B);
template<typename T>
int qsort_compare_decrease(const void* A, const void* B);
template<template<int (*Compare)(const T& A, const T& B)>, typename T>
void qsort(T* data, u32 size);

// NOTE(hugo): return nullptr when value_to_find is not in data
template<typename T>
T* search_linear(T* data, u32 size, T* value);
template<typename T>
T* search_binary(T* data, u32 size, T* value);
// NOTE(hugo): return the leftmost /value/ if multiple values are in data
template<typename T>
T* search_binary_left(T* data, u32 size, T* value);
// NOTE(hugo): return the rightmost /value/ if multiple values are in data
template<typename T>
T* search_binary_right(T* data, u32 size, T* value);

#include "sort_search.inl"

#endif
