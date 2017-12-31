#ifndef _ALGOBASE_H_
#define _ALGOBASE_H_

#include "simiterator_base.h"

namespace SimSTL {

template <typename T>
inline const T&
max(const T& a, const T& b)
{
        return a < b ? b : a;
}

template <typename ForwardIterator, typename T>
void
fill(ForwardIterator first, ForwardIterator last, const T& value)
{
        for (; first != last; ++first)
                *first = value;
}

template <typename OutputIterator, typename Size, typename T>
OutputIterator
fill_n(OutputIterator first, Size n, const T& x)
{
        for (; n > 0; --n, ++first)
                *first = x;
        return first;
}

template <typename InputIterator, typename OutputIterator, typename Distance>
inline OutputIterator
__copy_d(InputIterator first, InputIterator last, OutputIterator result, Distance*)
{
        for (Distance n = last - first; n > 0; --n, ++first, ++result)
                *result = *first;
        return result;
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator
__copy(InputIterator first, InputIterator last,
        OutputIterator result, input_iterator_tag)
{
        for (; first != last; ++first, ++result)
                *result = *first;
        return result;
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator
__copy(InputIterator first, InputIterator last,
        OutputIterator result, random_access_iterator_tag)
{
        return __copy_d(first, last, result, distance_type(first));
}

template <typename InputIterator, typename OutputIterator>
struct __copy_dispatch
{
        OutputIterator operator()(InputIterator first, InputIterator last,
                                  OutputIterator result)
        {
                return __copy(first, last, result, iterator_category(first));
        }
};


template <typename InputIterator, typename OutputIterator>
OutputIterator
copy(InputIterator first, InputIterator last, OutputIterator result)
{
        return __copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2,
          typename Distance>
inline BidirectionalIterator2
__copy_backward(BidirectionalIterator1 first,
                BidirectionalIterator1 last,
                BidirectionalIterator2 result,
                bidirectional_iterator_tag,
                Distance*)
{
        while (first != last)
                *--result = *--last;
        return result;
}

template <typename RandomAccessIterator, typename BidirectionalIterator,
          typename Distance>
inline BidirectionalIterator
__copy_backward(RandomAccessIterator first,
                RandomAccessIterator last,
                BidirectionalIterator result,
                random_access_iterator_tag,
                Distance*)
{
        for (Distance n = last - first; n > 0; n--)
                *--result = *--last;
        return result;
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2>
inline BidirectionalIterator2
copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
              BidirectionalIterator2 result)
{
        return __copy_backward(first, last, result,
                               iterator_category(first), distance_type(first));
}





}


#endif
