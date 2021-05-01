#ifndef H_SORT_SEARCH
#define H_SORT_SEARCH

// NOTE(hugo): generic comparison fuctions for qsort that avoid overflow
template<typename T>
s32 comparison_increasing_order(const T& A, const T& B);

template<typename T>
s32 comparison_decreasing_order(const T& A, const T& B);

// NOTE(hugo): compare(L, R) is
//  < 0 when L has lower  priority than R ie permutation NOT REQUIRED
//  = 0 when L has equal  priority with R ie permutation NOT REQUIRED
//  > 0 when L has higher priority than R ie permutation REQUIRED
//
// ex : sorting in increasing order means compare(L, R) = (L > R) - (L < R) = L - R (potential overflow !)
// ex : sorting in decreasing order means compare(L, R) = (R > L) - (R < L) = R - L (potential overflow !)

// NOTE(hugo): quicksort using the stdlib qsort
template<typename T, s32 (*compare)(const T& A, const T& B) = &comparison_increasing_order>
void qsort(T* data, u32 size);

// NOTE(hugo): insertion sort
// - fast for small array sizes bc. everything is in cache
// - stable
template<typename T, s32 (*compare)(const T& A, const T& B) = &comparison_increasing_order>
void isort(T* data, u32 size);

// NOTE(hugo): returns nullptr when /value/ is not found
template<typename T>
T* binsearch_lower(T* data, u32 size, const T& value);

// NOTE(hugo): returns the insertion point of /value/ in the array such as every value before that is < value
//             size is a valid return value
template<typename T>
u32 bininsert_lower(const T* data, u32 size, const T& value);

#include "algorithm.inl"

#endif
