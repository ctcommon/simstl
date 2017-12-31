#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "simconstruct.h"
#include "simalloc.h"
#include "simiterator.h"
#include "uninitialized.h"
#include <cstddef>

namespace SimSTL {

template <typename T>  //不同：不允许外部的配置器
class vector
{
public:
        typedef T                       value_type;
        typedef value_type*             pointer;
        typedef const value_type*       const_pointer;
        typedef value_type*             iterator;
        typedef const value_type*       const_iterator;
        typedef value_type&             reference;
        typedef const value_type&       const_reference;
        typedef size_t                  size_type;
        typedef ptrdiff_t               difference_type;
        typedef MiniSTL::reverse_iterator<iterator>             reverse_iterator;
        typedef MiniSTL::reverse_iterator<const_iterator>       const_reverse_iterator;
        typedef alloc allocator_type;

        allocator_type get_allocator() const { return allocator_type(); }


private:
        typedef simple_alloc<T, allocator_type> data_allocator;
        iterator        start;
        iterator        finish;
        iterator        end_of_storage;

private:
        pointer* allocate(size_t n)
        {
                return data_allocator::allocator(n);
        }

        void deallocate(pointer p, size_t n)
        {
                return data_allocator::deallocate(p, n);
        }

        void fill_initializer(size_type n, const T& value)
        {
                start = allocate_and_fill(n, value);
                finish = start + n;
                end_of_storage = finish;
        }

        iterator allocate_and_fill(size_type n, const T& value)
        {
                iterator result = data_allocator::allocate(n);
                uninitialized_fill_n(result, n, value);
                return result;
        }

public:
        vector() : start(0), finish(0), end_of_storage(0) {}
        explicit vector(size_type n) { fill_initializer(n, T()); }
        vector(size_t n, const T& value) { fill_initializer(n, value); }
        vector(int n, const T& value) { fill_initializer(n, value); }
        vector(long n, const T& value) { fill_initializer(n, value); }

        vector(const vector<T>& x)
        {
                start = data_allocator::allocate(x.size());
                finish = uninitialized_copy(x.begin(), x.end(), start);
                end_of_storage = finish;
        }

        vector(iterator first, iterator last)
        {
                __range_initialize(first, last, iterator_category(first));
        }

        ~vector()
        {
                destroy(start, finish);
                deallocate(start, end_of_storage - start);
        }


public:
        iterator begin() { return start; }
        iterator end() { return finish; }
        const_iterator begin() const { return start; }
        const_iterator end() const { return finish; }
        reverse_iterator rbegin() { return reverse_iterator(end()); }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        reverse_iterator rbegin() const { return reverse_iterator(end()); }
        reverse_iterator rend() const { return reverse_iterator(begin()); }
        size_t size() const { return size_type(end() - begin()); }
        bool empty() const { return begin() == end(); }
        size_t capacity() const { return size_type(end_of_storage - begin()); }
        reference operator[](size_type n) { return *(begin() + n); }
        const_reference operator[](size_type n) const { return *(begin() + n); }

public:
        reference front() const { return *begin(); }
        reference back() const { return *(end() - 1);}
        void insert_aux(iterator position, const T& x);
        void insert(iterator position, size_type n, const T& x);

        void push_back(const T& val)
        {
                if (finish != end_of_storage)
                {
                        construct(finish, val);
                        ++finish;
                }
                else
                        insert_aux(end(), val);
        }

        void pop_back()
        {
                --finish;
                destroy(finish);
        }

        iterator erase(iterator position)
        {
                if (position + 1 != end())
                        copy(position + 1, end(), position);
                --finish;
                destroy(finish);
                return position;
        }

        iterator erase(iterator first, iterator last)
        {
                iterator __i = copy(last, finish, first);
                destroy(__i, finish);
                finish = finish - (last - first);
                return first;
        }

        void resize(size_t new_size, const T& val)
        {
                if (new_size < size())
                        erase(begin() + new_size, end());
                else
                        insert(end(), new_size - size(), val);
        }

        void resize(size_t new_size) { resize(new_size, T()); }
        void clear() { erase(begin(), end()); }

public:
        template<typename InputIterator>
        void __range_initialize(InputIterator first, InputIterator last, input_iterator_tag)
        {
                for (; first != last; ++first)
                        push_back(*first);
        }

        template<typename ForwardIterator>
        void __range_initialize(ForwardIterator first, ForwardIterator last, forward_iterator_tag)
        {
                size_t n = distance(first, last);
                start = data_allocator::allocate(n);
                end_of_storage = start + n;
                finish = uninitialized_copy(first, last, start);
        }
};

template<typename T>
void
vector<T>::insert_aux(iterator position, const T& x)
{
        if (finish != end_of_storage)  //还有备用空间
        {
                construct(finish, *(finish - 1));
                ++finish;
                T x_copy = x;
                copy_backward(position, finish - 2, finish - 1);
                *position = x_copy;
        }
        else
        {
                const size_type old_size = size();
                const size_type len = old_size != 0 ? 2 * old_size : 1;  //2倍原空间大小
                iterator new_start = data_allocator::allocate(len);
                iterator new_finish = new_start;

                try {
                        new_finish = uninitialized_copy(start, position, new_start);
                        construct(new_finish, x);
                        ++new_finish;
                        new_finish = uninitialized_copy(position, finish, new_finish);
                }
                catch(...) {
                        destroy(new_start, new_finish);
                        data_allocator::deallocate(new_start, len);
                        throw;
                }

                destroy(begin(), end());
                deallocate(start, end_of_storage - start);
                start = new_start;
                finish = new_finish;
                end_of_storage = new_start + len;
        }
}

template <typename T>
void
vector<T>::insert(iterator position, size_type n, const T& x)
{
        if (n == 0)
                return ;
        if (size_type(end_of_storage - finish) >= n)  //1.备用空间够用
        {
                T x_copy = x;
                const size_type elems_after = finish - position;
                iterator old_finish = finish;
                if (elems_after > n)  //1.1插入点之后的现有元素个数大于新增元素个数
                {
                        uninitialized_copy(finish - n, finish, finish);
                        finish += n;
                        copy_backward(position, old_finish - n, old_finish);
                        fill(position, position + n, x_copy);
                }
                else  //1.2
                {
                        uninitialized_fill_n(finish, n - elems_after, x_copy);
                        finish += n - elems_after;
                        uninitialized_copy(position, old_finish, finish);
                        finish += elems_after;
                        fill(position, old_finish, x_copy);
                }
        }
        else  //2.备用空间个数小于新增元素个数
        {
                const size_type old_size = size();
                const size_type len = old_size + max(old_size, n);
                iterator new_start = data_allocator::allocate(len);
                iterator new_finish = new_start;
                try {
                        new_finish = uninitialized_copy(start, position, new_start);
                        new_finish = uninitialized_fill_n(new_finish, n, x);
                        new_finish = uninitialized_copy(position, finish, new_finish);
                }
                catch(...) {
                        destroy(new_start, new_finish);
                        data_allocator::deallocate(new_start, len);
                        throw;
                }
                destroy(start, finish);
                deallocate(start, end_of_storage - start);
                start = new_start;
                finish = new_finish;
                end_of_storage = new_start + len;
        }
}

}


#endif
