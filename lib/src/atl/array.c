/* 
 * File: array.c
 * Description:
 *       This file contains an implementation of auxiliary array type.
 *       It ia a part of CTesK Abstract Type Library.
 *
 * Copyright (c) 2001-2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

/*
 * =============================================================================
 *
 * Auxiliary array type
 *
 * =============================================================================
 */

#include <atl/array.h>
#include <utils/assertion.h>

Array create_Array( size_t header_size, size_t elem_size, size_t capacity )
{
    Array res = malloc( header_size + elem_size*capacity );

#ifdef _DEBUG_REFCNT
    printf( "create_Array %p: element size %d, capacity %d, total size %d\n", res, elem_size, capacity, header_size + elem_size*capacity );
#endif

    assertion( res != 0, FORMAT( "create_Array: No memory" ) );

    header_Array( res )->ref_cnt  = 1;
    header_Array( res )->capacity = capacity;

    return res;
}

void clone_Array( Array array )
{
    header_Array( array )->ref_cnt++;

#ifdef _DEBUG_REFCNT
    printf( "clone_Array %p %d\n", array, header_Array( array )->ref_cnt );
#endif
}

void destroy_Array( Array array )
{
#ifdef _DEBUG_REFCNT
    printf( "destroy_Array %p %d\n", array, header_Array( array )->ref_cnt - 1 );
#endif

    if( --(header_Array( array )->ref_cnt) == 0 )
    {
        free( array );
    }
}

Array expand_Array( Array array, size_t header_size, size_t elem_size )
{
	int old_capacity = header_Array(array)->capacity;
	int new_capacity = (old_capacity + 1) * 2;
    Array res = realloc( array
                       , header_size + elem_size * new_capacity
                       );
    assertion( res != 0, FORMAT( "expand_Array: No memory" ) );

#ifdef _DEBUG_REFCNT
    printf( "expand_Array %p to %p capacity %d %d\n"
          , array
          , res
          , new_capacity
          , header_Array( res )->ref_cnt 
          );
#endif

    header_Array( res )->capacity = new_capacity;
    return res;
}


