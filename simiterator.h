#ifndef _ITERATOR_H_
#define _ITERATOR_H_

#include "simiterator_base.h"

namespace SimSTL {

// 反向迭代器
template <typename Iterator>
class reverse_iterator
{
private:
        Iterator current;

public:
        typedef Iterator iterator_type;
        typedef reverse_iterator<Iterator> Self;

        typedef typename iterator_traits<Iterator>::iterator_category   iterator_category;
        typedef typename iterator_traits<Iterator>::value_type          value_type;
        typedef typename iterator_traits<Iterator>::difference_type     difference_type;
        typedef typename iterator_traits<Iterator>::pointer             pointer;
        typedef typename iterator_traits<Iterator>::reference           reference;

public:
        reverse_iterator() {}
        explicit reverse_iterator(iterator_type x):current(x) {}

public:
        reference operator*() const
        {
                Iterator tmp = current;
                return *--tmp;
        }

        pointer operator->() const
        {
                return &(operator*());
        }

        Self& operator++()
        {
                --current;
                return *this;
        }

        Self operator++(int)
        {
                Self tmp = *this;
                --current;
                return tmp;
        }

        Self operator+(difference_type n) const
        {
                return Self(current - n); //构造一个临时对象
        }

        Self operator+=(difference_type n)
        {
                current -= n;
                return *this;
        }

        Self& operator--()
        {
                ++current;
                return *this;
        }

        Self operator--(int)
        {
                Self tmp = *this;
                ++current;
                return tmp;
        }

        Self operator-(difference_type n) const
        {
                return Self(current + n); //构造一个临时对象
        }

        Self operator-=(difference_type n)
        {
                current += n;
                return *this;
        }

        reference operator[](difference_type n) const
        {
                return *(*this + n);
        }
};

template <typename Iterator>
inline bool operator==(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return x.base() == y.base();
}

template <typename Iterator>
inline bool operator!=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return !(x.base() == y.base());
}

template <typename Iterator>
inline bool operator<(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return x.base() < y.base();
}

template <typename Iterator>
inline bool operator<=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return !(y.base() < x.base());
}

template <typename Iterator>
inline bool operator>(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return y.base() < x.base();
}

template <typename Iterator>
inline bool operator>=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
        return !(x.base() < y.base());
}

}


#endif
