#ifndef _ITERATOR_BASE_TYPES_H_
#define _ITERATOR_BASE_TYPES_H_

#include <cstddef>  //for ptrdiff_t

namespace SimSTL {


//type-traits
//算法编译时自动判断最合适迭代器类型
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

//5种迭代器
template <typename T, typename Distance>
struct input_iterator
{
        typedef input_iterator_tag      iterator_category;
        typedef T                       value_type;
        typedef Distance                difference_type;
        typedef T*                      pointer;
        typedef T&                      reference;
};

struct output_iterator
{
        typedef output_iterator_tag     iterator_category;
        typedef void                    value_type;
        typedef void                    difference_type;
        typedef void                    pointer;
        typedef void                    reference;

};

template <typename T, typename Distance>
struct forward_iterator
{
        typedef forward_iterator_tag    iterator_category;
        typedef T                       value_type;
        typedef Distance                difference_type;
        typedef T*                      pointer;
        typedef T&                      reference;
};

template <typename T, typename Distance>
struct bidirectional_iterator
{
        typedef bidirectional_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef Distance                        difference_type;
        typedef T*                              pointer;
        typedef T&                              reference;
};

template <typename T, typename Distance>
struct random_access_iterator
{
        typedef random_access_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef Distance                        difference_type;
        typedef T*                              pointer;
        typedef T&                              reference;
};

//迭代器都应继承自此结构
template <typename Category, typename T, typename Distance = ptrdiff_t,
          typename Pointer = T*, typename Reference = T&>
struct iterator
{
        typedef Category        iterator_category;
        typedef T               value_type;
        typedef Distance        difference_type;
        typedef Pointer         pointer;
        typedef Reference       reference;
};


//iterator traits
template <typename Iterator>
struct iterator_traits
{
        typedef typename Iterator::iterator_category    iterator_category;
        typedef typename Iterator::value_type           value_type;
        typedef typename Iterator::difference_type      difference_type;
        typedef typename Iterator::pointer              pointer;
        typedef typename Iterator::reference            reference;
};

//iterator traits原生指针的偏特化版本
template <typename T>
struct iterator_traits<T*>
{
        typedef random_access_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef ptrdiff_t                       difference_type;
        typedef T*                              pointer;
        typedef T&                              reference;
};

//iterator traits原生常量指针的偏特化版本
template <typename T>
struct iterator_traits<const T*>
{
        typedef random_access_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef ptrdiff_t                       difference_type;
        typedef const T*                        pointer;
        typedef const T&                        reference;
};

//判断迭代器属于5种迭代器中哪一种 iterator_category
template <typename Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{
        typedef typename iterator_traits<Iterator>::iterator_category category;
        return category();
}

//返回迭代器指向的对象类型 value_type*
template <typename Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
        return static_cast<typename iterator_traits<Iterator>::value_type*>(NULL);
}

//返回两个迭代器之间距离的类型 difference_type*
template <typename Iterator>
inline typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&)
{
        return static_cast<typename iterator_traits<Iterator>::difference_type*>(NULL);
}

template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag)
{
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last)
        {
                ++first;
                ++n;
        }
        return n;
}

template <typename RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
        return last - first;
}

// 返回两个迭代器之间距离
template <typename Iterator>
inline typename iterator_traits<Iterator>::difference_type
distance(Iterator first, Iterator last)
{
        return __distance(first, last, iterator_category(first));
}

template <typename InputIterator, typename Distance>
inline void
__advance(InputIterator& i, Distance n, input_iterator_tag)
{
        while (n--)
                ++i;
}

template <typename BidirectionalIterator, typename Distance>
inline void
__advance(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
{
        if (n >= 0)
        {
                while (n--)
                        ++i;
        }
        else
        {
                while (n++)
                        --i;
        }
}

template <typename RandomAccessIterator, typename Distance>
inline void
__advance(RandomAccessIterator& i, Distance n, random_access_iterator_tag)
{
        i += n;
}

//移动迭代器n步
template <typename Iterator, typename Distance>
inline void
advance(Iterator& i, Distance n)
{
        __advance(i, n, iterator_category(i));
}

}


#endif
