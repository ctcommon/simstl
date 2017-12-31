#ifndef _MINISTL_LIST_H_
#define _MINISTL_LIST_H_

#include "simiterator.h"
#include "simalloc.h"
#include "simalgobase.h"

namespace SimSTL {


// list节点
template <typename T>
struct __list_node
{
        typedef void* void_pointer;
        void_pointer prev;
        void_pointer next;
        T data;
};

// list迭代器
template <typename T, typename Ref, typename Ptr>
struct __list_iterator
{
        typedef __list_iterator<T, T&, T*>              iterator;
        typedef __list_iterator<T, const T&, const T*>  const_iterator;
        typedef __list_iterator<T, Ref, Ptr>            self;

        typedef bidirectional_iterator_tag iterator_tag;
        typedef T               value_type;
        typedef ptrdiff_t       difference_type;
        typedef Ptr             pointer;
        typedef Ref             reference;
        typedef size_t          size_type;
        typedef __list_node<T>* link_type;

        link_type node;  // point to __list_node

        // constructor
        __list_iterator(link_type x) : node(x) {}
        __list_iterator() {}
        __list_iterator(const iterator& x) : node(x.node) {}

        bool operator==(const self& x) const { return node == x.node; }
        bool operator!=(const self& x) const { return node != x.node; }
        reference operator*() const { return (*node).data; }
        pointer operator->() const { return &(operator*());}
        self& operator++() { node = (link_type)(*node).next; return *this;}
        self operator++(int)
        {
                self tmp = *this;
                ++*this;
                return tmp;
        }
        self& operator--() { node = (link_type)(*node).prev; return *this;}
        self operator--(int)
        {
                self tmp = *this;
                --*this;
                return tmp;
        }
};

// list
template <typename T>
class list
{
public:
        // 基础类型
        typedef T                       value_type;
        typedef ptrdiff_t               difference_type;
        typedef value_type*             pointer;
        typedef const value_type*       const_pointer;
        typedef value_type&             reference;
        typedef const value_type&       const_reference;
        typedef size_t                  size_type;

        typedef __list_node<T>          list_node;
        typedef list_node*              link_type;

        // 空间配置器
        typedef simple_alloc<list_node, alloc> list_node_allocator;

private:
        //指向空白节点
        link_type               node;

public:
        // 迭代器
        typedef __list_iterator<T, T&, T*>              iterator;
        typedef __list_iterator<T, const T&, const T*>  const_iterator;
        typedef reverse_iterator<iterator>              reverse_iterator;
        typedef reverse_iterator<const_iterator>        const_reverse_iterator;

public:
        // 通过空白节点node完成
        iterator begin() const { return (link_type)(*node).next; }
        iterator end() const { return node;}
        bool empty() const { return node->next == node; }
        size_type size() const { return (size_type)distance(begin(), end); }
        reference front() const { return *begin(); }
        reference back() const { return *(--end());}

private:
        // 内部操作
        link_type get_node() { return list_node_allocator::allocate(1);}
        void put_node(link_type p) { list_node_allocator::deallocate(p, 1); }

        link_type create_node(const T& x)
        {
                link_type p = get_node();
                construct(&p->data, x);
                return p;
        }

        void destroy_node(link_type p)
        {
                destroy(p, 1);
                put_node(p);
        }

        void empty_initialize()
        {
                node = get_node();
                node->next = node;
                node->prev = node;
        }

        void transfer(iterator position, iterator first, iterator last);

public:
        iterator insert(iterator position, const T& x);
        iterator insert(iterator position);
        iterator insert(iterator position, size_type n, const T& x);
        iterator insert(iterator position, iterator first, iterator last);
        iterator erase(iterator position);
        iterator erase(iterator first, iterator last);
        void clear();
        void remove(const T& x);
        void unique();
        void splice(iterator position, list<T>& x);
        void splice(iterator position, list<T>& x, iterator i);
        void splice(iterator position, list<T>& x, iterator first, iterator last);
        void merge(list<T>& x);
        void reverse();
        void sort();
        void swap(list<T>& x);

        void push_back(const T& x) { insert(end(), x); }
        void push_front(const T& x) { insert(begin(), x); }
        void pop_front() { erase(begin()); }
        void pop_back() { iterator tmp = end(); erase(--tmp); }

public:
        list() { empty_initialize();}

        explicit list(size_type n)
        {
                empty_initialize();
                insert(begin(), n, T());
        }

        list(const list<T>& x)
        {
                empty_initialize();
                insert(begin(), x.begin(), x.end());
        }

        list(iterator first, iterator last)
        {
                empty_initialize();
                insert(begin(), first, last);
        }

        ~list() {}
        list<T>& operator=(const list<T>& x);
};

template <typename T>
list<T>&
list<T>::operator=(const list<T>& x)
{
        if (this == &x)
                return *this;
        iterator first1 = begin();
        iterator last1 = end();
        const_iterator first2 = x.begin();
        const_iterator last2 = x.end();
        while (first1 != last1 && first2 != last2)
                *first1++ = *first2++;
        if (first2 == last2)
                erase(first1, last1);
        else
                insert(last1, first2, last2);
        return *this;
}

template <typename T>
void
list<T>::transfer(iterator position, iterator first, iterator last)
{
        (*(link_type((*last.node).prev))).next = position.node;
        (*(link_type((*first.node).prev))).next = last.node;
        (*(link_type((*position.node).prev))).next = last.node;
        link_type tmp = link_type((*position.node).prev);
        (*last.node).prev = (*first.node).prev;
        (*first.node).prev = tmp;
}

template <typename T>
void
list<T>::insert(iterator position, const T& x)//posiiton之前插入
{
        link_type tmp = create_node(x);
        tmp->next = position.node;
        tmp->prev = position.node->prev;
        (link_type(position.node->prev))->next = tmp;
        position.node->prev = tmp;
        return tmp;
}

template <typename T>
void
list<T>::insert(iterator position)
{
        insert(position, T());
}

template <typename T>
void
list<T>::insert(iterator position, size_type n, const T& x)
{
        for (; n > 0; --n)
                insert(position, x);
}

template <typename T>
void
list<T>::insert(iterator position, iterator first, iterator last)
{
        for(; first != last; ++first)
                insert(position, *first);
}

template <typename T>
typename list<T>::iterator
list<T>::erase(iterator position)
{
        link_type next_node = (link_type)position.node->next;
        link_type prev_node = (link_type)position.node->prev;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        destroy_node(position);
        return (iterator)next_node;
}

template <typename T>
typename list<T>::iterator
list<T>::erase(iterator first, iterator last)
{
        while (first != last)
                erase(first++);
        return last;
}

template <typename T>
void
list<T>::clear()
{
        link_type cur = (link_type)node->next;
        while (cur != node)
        {
                link_type tmp = cur;
                cur = cur->next;
                destroy_node(tmp);
        }
        node->prev = node;
        node->next = node;
}

template <typename T>
void
list<T>::remove(const T& x)
{
        iterator first = begin();
        iterator last = end();
        while (first != last)
        {
                iterator next = first;
                ++next;
                if (*first == x)
                        erase(first);
                first = next;
        }
}

template <typename T>
void
list<T>::unique()
{
        iterator first = begin();
        iterator last = end();
        while (first != last)
        {
                iterator next = first;
                ++next;
                while (next != last && *next == *first)
                        erase(next++);
                first = next;
        }
}

template <typename T>
void
list<T>::splice(iterator position, list<T>& x)
{
        if (!x.empty())
                transfer(position, x.begin(), x.end());
}

template <typename T>
void
splice(iterator position, list<T>& x, iterator i)
{
        iterator next = i;
        ++next;
        if (position == i || position == j)
                return ;
        transfer(position, first, last);
}

template <typename T>
void
splice(iterator position, list<T>& x, iterator first, iterator last)
{
        if (first != last)
                transfer(position, first, last);
}

template <typename T>
void
list<T>::merge(list<T>& x) //前提两个list都已递增排序
{
        iterator first1 = begin();
        iterator last1 = end();
        iterator first2 = x.begin();
        iterator last2 = x.end();
        while (first1 != last1 && first2 != last2)
        {
                if (*first2 < *first1)
                {
                        iterator next = first2;
                        ++next;
                        transfer(first1, first2, next);
                        first2 = next;
                }
                else
                        ++first1;
        }
        if (first2 != last2)
                transfer(last1, first2, last2);
}

template <typename T>
void
list<T>::reverse()
{
        if (node->next == node || link_type(node->next)->next == node)
                return ;
        iteraotr first = begin();
        ++first;
        while (first != end())
        {
                iterator old = first;
                ++first;
                transfer(begin(), old, first);
        }
}

template <typename T>
void
list<T>::swap(list<T>& x)
{
        std::swap(node, x.node);
}

template <typename T>
inline void
swap(list<T>& x, list<T>& y)
{
        x.swap(y);
}

template <typename T>
void
list<T>::sort()
{
        if (node->next = node || linke_type(node->next)->next = node)
                return ;
        list<T> carry;
        list<T> count[64];
        int fill = 0;
        while (!empty())
        {
                carry.splice(carry.begin(), *this, begin());
                int i = 0;
                while (i < fill && !counter[i].empty())
                {
                        counter[i].merge(carry);
                        carry.swap(counter[i++]);
                }
                carry.swap(counter[i]);
                if (i == fill)
                        ++fill;
        }

        for (int i = 1; i < fill; ++i)
                counter[i].merge(counter[i - 1]);
        swap(counter[fill - 1]);
}

template <typename T>
inline bool
operator==(const list<T>& x, const list<T>& y)
{
        typedef typename list<T>::const_iterator const_iterator;
        const_iterator first1 = x.begin();
        const_iterator last1 = x.end();
        const_iterator first2 = y.begin();
        const_iterator last2 = y.end();
        while (first1 != last1 && first2 != last2 && *first1 == *first2)
        {
                ++first1;
                ++first2;
        }

        return first1 == last1 && first2 == last2;
}

template <typename T>
inline bool
operator!=(const list<T>& x, const list<T>& y)
{
        return !(x == y)
}

template <typename T>
inline bool
operator<(const list<T>& x, const list<T>& y)
{
        if (x.size() < y.size())
                return true;
        else if (x.size() > y.size())
                return false;
        else
        {
                typedef typename list<T>::const_iterator const_iterator;
                const_iterator first1 = x.begin();
                const_iterator first2 = y.begin();
                size_type n = x.size();
                for (; n > 0; --n)
                {
                        if (*first1 == *first2)
                        {
                                ++first1;
                                ++first2;
                                continue;
                        }
                        if (*first1 < *first2)
                                return true;
                        else
                                return false;
                }
                return false;
        }
}

template <typename T>
inline bool
operator>(const list<T>& x, const list<T>& y)
{
        return y < x;
}

template <typename T>
inline bool
operator<=(const list<T>& x, const list<T>& y)
{
        return !(y < x);
}

template <typename T>
inline bool
operator>=(const list<T>& x, const list<T>& y)
{
        return !(x < y);
}

} // namespace MiniSTL

#endif /*_MINISTL_LIST_H_*/
