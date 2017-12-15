#ifndef _ALLOC_H_
#define _ALLOC_H_

#if     0
#       include<new>
#       define __THROW_BAD_ALLOC throw bad_alloc
#elif   !defined(__THROW_BAD_ALLOC)
#       include <iostream>
#       define __THROW_BAD_ALLOC std::cerr << "out of memory" << std::endl; exit(1)
#endif

namespace SimiSTL {


//第一级配置器
template <int inst>
class __malloc_alloc
{
private:
        static void *oom_malloc(size_t);
        static void *oom_realloc(void *, size_t);
        static void (*malloc_alloc_oom_handler)();

public:
        static void *allocate(size_t n)
        {
                void *result = malloc(n);
                if (NULL == result)
                        result = oom_malloc(n);
                return result;
        }

        static void deallocate(void *p, size_t) { free(p); }

        static void *reallocate(void *p, size_t, size_t new_size)
        {
                void *result = realloc(p, new_size);
                if (NULL == result)
                        result = oom_realloc(p, new_size);
                return result;
        }

        static void (*set_malloc_handler(void (*f)()))()
        {
                malloc_alloc_oom_handler = f;
        }

};

template <int inst>
void (* __malloc_alloc<inst>::malloc_alloc_oom_handler)() = NULL;

template <int inst>
void* __malloc_alloc<inst>::oom_malloc(size_t n)
{
        void (*my_malloc_handler)();
        void *result;

        for (;;)
        {
                my_malloc_handler = malloc_alloc_oom_handler;
                if (NULL == my_malloc_handler)
                        __THROW_BAD_ALLOC;
                (*my_malloc_handler)();
                result = malloc(n);
                if (result)
                        return result;
        }
}

template <int inst>
void* __malloc_alloc<inst>::oom_realloc(void *p, size_t n)
{
        void (*my_malloc_handler)();
        void *result;

        for (;;)
        {
                my_malloc_handler = malloc_alloc_oom_handler;
                if (NULL == my_malloc_handler)
                        __THROW_BAD_ALLOC;
                (*my_malloc_handler)();
                result = realloc(p, n);
                if (result)
                        return result;
        }

}

typedef __malloc_alloc<0> malloc_alloc;


//第二级配置器
template <bool threads>
class __default_alloc
{
public:
        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *reallocate(void *p, size_t old_size, size_t new_size);

private:
        //返回一个大小为n的块,重新填充链表
        static void *refill(size_t n);

        //配置nobjs个块，每块大小为size的空间
        static char *chunk_alloc(size_t size, int& nobjs);

private:
        //区块边界
        enum {__ALIGN = 8};

        //区块上限
        enum {__MAX_BYTES = 128};

        //链表个数
        enum {__NFREELISTS = __MAX_BYTES / __ALIGN};

        //填充链表时的块数
        enum {__NOBJS = 20};

private:
        //自由链表
        union obj
        {
                union obj *free_list_link;
                char data[1];
        };

        //自由链表数组
        static obj *volatile free_list[__NFREELISTS];

        //字节上调为8的倍数
        static size_t ROUND_UP(size_t bytes)
        {
                return ((bytes + __ALIGN - 1) & ~(__ALIGN - 1));
        }

        //选择自由链表
        static size_t FREELIST_INDEX(size_t bytes)
        {
                return (bytes + __ALIGN - 1) / __ALIGN - 1;
        }

private:
        static char *start_free;
        static char *end_free;
        static size_t heap_size;
};

//内存池起始位置
template <bool threads>
char *__default_alloc<threads>::start_free = NULL;

//内存池结束位置
template <bool threads>
char *__default_alloc<threads>::end_free = NULL;

template <bool threads>
size_t __default_alloc<threads>::heap_size = 0;

template <bool threads>
typename __default_alloc<threads>::obj *volatile
__default_alloc<threads>::free_list[__NFREELISTS] =
        {
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
        };

template <bool threads>
void *__default_alloc<threads>::allocate(size_t n)
{
        obj *volatile *my_free_list;
        obj *result;
        if (n > (size_t)__MAX_BYTES) //调用第一级配置器
                return malloc_alloc::allocate(n);

        my_free_list = free_list + FREELIST_INDEX(n);
        result = *my_free_list;
        if (result == NULL)
        {
                //第n号链表无内存块，则准备重新填充该链表
                void *r = refill(ROUND_UP(n));
                return r;
        }
        *my_free_list = result->free_list_link;
        return result;
}

template <bool threads>
void __default_alloc<threads>::deallocate(void *p, size_t n)
{
        //大于__MAX_BYTES，则释放该内存
        if (n > (size_t)__MAX_BYTES)
                malloc_alloc::deallocate(p, n);

        obj *q = (obj *)p;
        obj *volatile *my_free_list;

        my_free_list = free_list + FREELIST_INDEX(n);
        //小于__MAX_BYTES，则回收区块,并未释放
        q->free_list_link = *my_free_list;
        *my_free_list = q;
}

template <bool threads>
void *__default_alloc<threads>::reallocate
    (void *p, size_t old_size, size_t new_size)
{
        obj *volatile *my_free_list;
        obj *result;
        if (new_size > (size_t)__MAX_BYTES) //调用第一级配置器
                return malloc_alloc::reallocate(p, old_size, new_size);

        my_free_list = free_list + FREELIST_INDEX(new_size);
        result = *my_free_list;
        if (result == NULL)
        {
                //第n号链表无内存块，则准备重新填充该链表
                void *r = refill(ROUND_UP(new_size));
                return r;
        }
        *my_free_list = result->free_list_link;
        return result;

}

template <bool threads>
void *__default_alloc<threads>::refill(size_t n)
{
        int nobjs = __NOBJS;
        char *chunk = chunk_alloc(n, nobjs);  //从内存池获取内存
        if (nobjs == 1)  //只能分配一块，则直接返回给调用者
                return chunk;

        obj *volatile *my_free_list;
        obj *result, *next_obj, *current_obj;

        result = (obj *)chunk;
        my_free_list = free_list + FREELIST_INDEX(n);

        *my_free_list = next_obj = (obj *)(chunk + n);
        for (int i = 1; i < nobjs - 1; i++)  //将剩下的区块添加进链表
        {
                current_obj = next_obj;
                next_obj = (obj *)(char *)(next_obj + n);
                current_obj->free_list_link = next_obj;
        }

        //最后一块
        current_obj = next_obj;
        current_obj->free_list_link = NULL;

        return result;
}

template <bool threads>
char *__default_alloc<threads>::chunk_alloc(size_t size, int& nobjs)
{
        size_t total_size = size * nobjs;
        char *result;
        size_t size_left = end_free - start_free;

        if (size_left >= total_size)  //内存池剩余空间满足需求
        {
                result = start_free;
                start_free += total_size;
                return result;
        }
        else if (size_left >= size)  //剩余空间不能全部满足，但至少满足一块
        {
                nobjs = size_left / size;
                result = start_free;
                start_free += nobjs * size;
                return result;
        }
        else  //连一个区块都无法满足
        {
                if (size_left > 0)  //将残余内存分配给其他合适的链表
                {
                        obj *volatile *my_free_list = free_list + FREELIST_INDEX(size_left);
                        ((obj *)start_free)->free_list_link = *my_free_list;  //在头部插入
                        *my_free_list = (obj *)start_free;
                }

                size_t bytes_to_get = 2 * total_size + ROUND_UP(heap_size >> 4);
                start_free = (char *)malloc(bytes_to_get);
                if (start_free == NULL)  //堆空间不足
                {
                        int i;
                        obj *volatile *my_free_list;
                        obj *p;
                        for (i = size; i < __MAX_BYTES; i++)
                        {
                                my_free_list = free_list + FREELIST_INDEX(i);
                                p = *my_free_list;
                                if (p != NULL)
                                {
                                        *my_free_list = p->free_list_link;
                                        start_free = (char *)p;
                                        end_free = start_free + i;
                                        return chunk_alloc(size, nobjs);
                                }
                        }
                        end_free = NULL;
                        //调用第一级配置器
                        start_free = (char *)malloc_alloc::allocate(bytes_to_get);
                }
                heap_size += bytes_to_get;
                end_free = start_free + heap_size;
                return chunk_alloc(size, nobjs);
        }
}

#ifdef __USE_MALLOC
typedef malloc_alloc alloc
#else
typedef __default_alloc<false> alloc;
#endif

template <typename T, typename Alloc>
class simple_alloc
{
public:
        static T *allocate(size_t n)
        {
                return n == 0 ? 0 : (T*)Alloc::allocate(n * sizeof(T));
        }

        static T *alocate(void)
        {
                return (T*)Alloc::allocate(sizeof(T));
        }

        static void deallocate(T *p, size_t n)
        {
                if (n != 0)
                        Alloc::deallocate(p, n * sizeof(T));
        }

        static void deallocate(T *p)
        {
                Alloc::deallocate(p, sizeof(T));
        }
};


}

#endif
