#ifndef _CONSTRUCT_H_H
#define _CONSTRUCT_H_H

#include <new> //for 定位new
#include "simtype_traits.h"
#include "simiterator_base.h"  //for value_type()

namespace SimSTL {


template <typename T1, typename T2>
inline void construct(T1 *p, const T2& value)
{
        //定位new
        new (p) T1(value);
}

//destroy()第一版本，接受一个指针
template <typename T>
inline void destroy(T* point)
{
        point->~T();
}

//有trivial destructor
template <typename Iterator>
inline void __destroy_aux(Iterator first, Iterator last, __true_type) {}

//有non-trivial destructor
template <typename Iterator>
inline void __destroy_aux(Iterator first, Iterator last, __false_type)
{
        for (; first < last; first++)
                destroy(&*first);  //调用第一版本destroy()
}

//判断元素的值类型是否有trivial destructor
template <typename Iterator, typename T>
inline void __destroy(Iterator first, Iterator last, T* )
{
        typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
        __destroy_aux(first, last, trivial_destructor());
}

//destroy()第二版本，接受两个迭代器
template <typename Iterator>
inline void destroy(Iterator first, Iterator last)
{
        __destroy(first, last, value_type(first));
}


}

#endif
