/* 
 * File: array.h
 * Description:
 *       This file declares auxiliary array type.
 *       It is an internal header file of CTesK Abstract Type Library.
 *
 * Copyright (c) 2001-2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#ifndef __ATL_ARRAY_H__
#define __ATL_ARRAY_H__

#include <stdlib.h>

#define CONTAINER_INITIAL_CAPACITY 0

#ifdef __cplusplus
extern "C" {
#endif

typedef 
struct ArrayHeader
{
    int ref_cnt;
    int capacity;
}
ArrayHeader;

typedef void *Array;

#define header_Array( array ) ((ArrayHeader*)array)

Array create_Array( size_t header_size, size_t elem_size, size_t capacity );

void clone_Array( Array array );

void destroy_Array( Array array );

Array expand_Array( Array array, size_t header_size, size_t elem_size );

#ifdef __cplusplus
}
#endif

#endif
