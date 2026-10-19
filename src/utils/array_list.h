#ifndef __ARRAY_LIST_CAVI__
#define __ARRAY_LIST_CAVI__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct{

    float* items;
    size_t size;
    size_t capacity;

}float_al;

typedef struct{

    int* items;
    size_t size;
    size_t capacity;

}int_al;

typedef struct{
    FILE** items;
    size_t size;
    size_t capacity;
}FILE_al;

#define append_al(list,element) \
    do{ \
        if(list.size >= list.capacity){ \
            if(list.capacity == 0) \
                list.capacity = 256; \
            else \
                list.capacity *= 2; \
            list.items = realloc(list.items,list.capacity*sizeof(*list.items)); \
        } \
        list.items[list.size++] = element; \
    }while(0);

#define append_all_al(list_dst,list_src) \
    do{ \
        for(size_t i=0;i<list_src.size;i++){ \
            append_al(list_dst,list_src.items[i]); \
        } \
    }while(0);

#define remove_last_al(list) \
    list.size--;

#define add_al(list,index,element) \
    do{\
        if(list.size >= list.capacity){ \
            if(list.capacity == 0) \
                list.capacity = 256; \
            else \
                list.capacity *= 2; \
            list.items = realloc(list.items,list.capacity*sizeof(*list.items)); \
        } \
        list.size++; \
        for(size_t i=list.size-1;i>index;i--) \
            list.items[i] = list.items[i-1]; \
        list.items[index] = element; \
    }while(0);

#define remove_al(list,index) \
    do{ \
        if(list.size >= list.capacity){ \
            if(list.capacity == 0) \
                list.capacity = 256; \
            else \
                list.capacity *= 2; \
            list.items = realloc(list.items,list.capacity*sizeof(*list.items)); \
        } \
        for(size_t i=index;i<list.size-1;i++) \
            list.items[i] = list.items[i+1]; \
        list.size--; \
    }while(0);

#define set_al(list,index,element) \
    list.items[index] = element;

#define trim_to_size_al(list) \
    do{ \
        list.items = realloc(list.items,list.size*sizeof(*list.items)); \
        list.capacity = list.size; \
    }while(0);

#define clone_al(list,destination) \
    do{ \
        destination.size = list.size; \
        destination.capacity = list.capacity; \
        destination.items = realloc(destination.items, list.size*sizeof(*list.items)); \
        memcpy(destination.items,list.items,list.size*sizeof(*list.items)); \
    }while(0);

#endif
