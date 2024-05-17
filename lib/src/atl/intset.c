/*
 * Copyright 2002-2009 ISP RAS (http://www.ispras.ru), UniTESK Lab (http://www.unitesk.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * File: intset.c
 * Description:
 *       This file defines the specification type representing
 *       sets of integers.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/intset.h>

#include <atl/array.h>
#include <atl/integer.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>

#include <stdio.h>
#include <string.h>

typedef struct IntSetHeader
{
    int          ref_cnt;
    int          capacity;
    unsigned int upper;
}
IntSetHeader;

struct IntSet {
    IntSetHeader *header;
};

#define header_IntSet( set ) ( (set)->header )

#define CHUNK_SIZE ( sizeof(unsigned long) )
#define CHUNK_BIT_SIZE ( CHUNK_SIZE << 3 )

#define CHUNK(i) ( ( i ) >> 5 )
#define BIT(i)   ( ( i ) & ( CHUNK_BIT_SIZE - 1 ) )

#define NUMBER(chunk, bit) ( ( ( chunk ) << 5 ) + ( bit ) )

#define ARRAY(set) ( (unsigned long *)( header_IntSet( set ) + 1 ) )
#define CAPACITY(set) ( header_IntSet( set )->capacity )

#define SET(set, i) ( ARRAY( set )[ CHUNK( i ) ] |= ( 1 << BIT( i ) ) )
#define UNSET(set, i) ( ARRAY( set )[ CHUNK( i ) ] &= ~( 1 << BIT( i ) ) )
#define GET(set, i) ( ( ARRAY( set )[ CHUNK( i ) ] >> BIT( i ) ) & 1 )

#define FULL_MASK ( (unsigned long)-1 )
#define MASK(upper) ( FULL_MASK >> ( CHUNK_BIT_SIZE - 1 - BIT( upper ) ) )

static void init_IntSet( IntSet *set, va_list *arg_list )
{
unsigned int upper = va_arg( *arg_list, unsigned int );
unsigned long *buf;
unsigned int i;

  header_IntSet( set ) = create_Array( sizeof( IntSet )
                                     , sizeof( IntSetHeader )
                                     , ( CHUNK( upper ) + 1 ) * CHUNK_SIZE
                                     );

  header_IntSet( set )->upper = upper;

  buf = ARRAY( set );
  for ( i = 0; i <= CHUNK(upper); buf[i++] = 0 );
}

static void copy_IntSet( IntSet *src, IntSet *dst )
{
    clone_Array( header_IntSet( dst ) = header_IntSet( src ) );
}

static void deep_copy_IntSet( IntSet* src, IntSet* dst )
{
  /* copy header */
  header_IntSet( dst ) = create_Array( sizeof( IntSet ),
                                       sizeof( IntSetHeader ),
                                       header_IntSet( src )->capacity
                                  );
  header_IntSet( dst )->upper = header_IntSet( src )->upper;

  /* copy data */
  memcpy( ARRAY( dst ), ARRAY( src ), CAPACITY( dst ) * CHUNK_SIZE );
}

static int compare_IntSet( IntSet *left, IntSet *right )
{
    unsigned int upper = header_IntSet( left )->upper;
    int upper_idx = CHUNK( upper );

    if( upper != header_IntSet( right )->upper )
    {
        return upper - header_IntSet( right )->upper;
    }

    return memcmp( ARRAY( left )
                 , ARRAY( right )
                 , upper_idx * CHUNK_SIZE
                 )
        || (    ( ARRAY( left )[upper_idx]  & MASK( upper ) )
             != ( ARRAY( right )[upper_idx] & MASK( upper ) )
           );
}

static String *to_string_IntSet( IntSet *set )
{
    StringBuffer *res = create_StringBuffer();
    unsigned int upper = header_IntSet( set )->upper;
    unsigned int i, j, bit;
    unsigned long *buf = ARRAY( set );
    bool first = true;
    char num[12];
	
	append_StringBuffer(r(res), "{ ");

    for( i = 0; i < CHUNK(upper); i++ )
        for ( j = 0, bit = 1; j < CHUNK_BIT_SIZE; j++, bit <<= 1 )
            if ( buf[i] & bit )
            {
                sprintf( num, "%d", NUMBER( i , j ) );
                if( ! first )	append_StringBuffer( r(res), ", " );
                appendString_StringBuffer( r(res), create_String(num) );
                first = false;
            }

    for ( j = 0, bit = 1; j <= BIT( upper ); j++, bit <<= 1 )
        if ( buf[i] & bit )
        {
            sprintf( num, "%d", NUMBER( i , j ) );
            if( ! first )	append_StringBuffer( r(res), ", " );
            appendString_StringBuffer( r(res), create_String(num) );
            first = false;
        }

	append_StringBuffer( r(res), first ? "}" : " }" );

    return toString( res );
}

static String *to_XML_IntSet( IntSet *set )
{
    StringBuffer *res = create_StringBuffer();
    unsigned int upper = header_IntSet( set )->upper;
    unsigned int i, j, bit;
    unsigned long *buf = ARRAY( set );

	append_StringBuffer(r(res), "<object kind=\"spec\" type=\"IntSet\" text=\"IntSet\">");

    for( i = 0; i < CHUNK(upper); i++ )
        for ( j = 0, bit = 1; j < CHUNK_BIT_SIZE; j++, bit <<= 1 )
            if ( buf[i] & bit )
            {
                appendString_StringBuffer( r(res), format_String("<object kind=\"spec\" type=\"IntSet.element\" text=\"%d\"/>", NUMBER( i , j )));
            }

    for ( j = 0, bit = 1; j <= BIT( upper ); j++, bit <<= 1 )
        if ( buf[i] & bit )
        {
            appendString_StringBuffer( r(res), format_String("<object kind=\"spec\" type=\"IntSet.element\" text=\"%d\"/>", NUMBER( i , j )));
        }

	append_StringBuffer(r(res), "</object>");

    return toString( res );
}

static void destroy_IntSet( IntSet *set )
{
    destroy_Array( header_IntSet( set ) );
}

const Type type_IntSet
    = { "IntSet"
      , sizeof(IntSet)
      , &type_Object
      , (Init)init_IntSet
      , (Copy)deep_copy_IntSet
      , (Compare)compare_IntSet
      , (ToString)to_string_IntSet
      , (ToXML)to_XML_IntSet
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_IntSet
      };

IntSet *create_IntSet( unsigned int upper )
{
    return (IntSet *)create( &type_IntSet, upper );
}

static void make_unique_array_IntSet( IntSet *set )
{
    IntSetHeader *old_header = header_IntSet( set ), *new_header;

    if( old_header->ref_cnt == 1 ) return;

    new_header = create_Array( sizeof( IntSet )
                             , sizeof( IntSetHeader )
                             , header_IntSet( set )->capacity
                             );

    new_header->upper = header_IntSet( set )->upper;

    memcpy( new_header+1
          , ARRAY( set )
          , CAPACITY( set ) * CHUNK_SIZE
          );

    header_IntSet( set )->ref_cnt--;
    header_IntSet( set ) = new_header;
}

bool add_IntSet( IntSet *self, unsigned int i )
{
bool res;

  CHECK_TYPE_COMPATIBLE( add_IntSet, &type_IntSet, self );
  assertion( i <= header_IntSet( self )->upper
           , FORMAT( "add_IntSet: list index %d is out of bounds [0, %d]" )
           , i
           , header_IntSet( self )->upper
           );

  res = (GET(self,i) != 0);

  if (res)
   {destroy( self );
    return false;
   }

  make_unique_array_IntSet( self );
  SET( self, i );

  destroy( self );
  return true;
}

/*
 * Returns upper bound of specified set
 */
unsigned int upper_IntSet(IntSet* self)
{
unsigned int res;

  CHECK_TYPE_COMPATIBLE( upper_IntSet, &type_IntSet, self );

  res = header_IntSet( self )->upper;

  destroy( self );

  return res;
}

/*
 * Returns the number of elements in this set.
 */
size_t size_IntSet( IntSet *self )
{
    size_t res = 0;
    unsigned int i, j, bit, upper, upper_chunk, upper_bit;
    unsigned long *buf;

    CHECK_TYPE_COMPATIBLE( size_IntSet, &type_IntSet, self );

    upper = header_IntSet( self )->upper;
    upper_chunk = CHUNK(upper);
    upper_bit = BIT(upper);
    buf = ARRAY( self );

    for( i = 0; i < upper_chunk; i++ )
        for ( j = 0, bit = 1; j < CHUNK_BIT_SIZE; j++, bit <<= 1 )
            if ( buf[i] & bit )
                res++;
    for ( j = 0, bit = 1; j <= upper_bit; j++, bit <<= 1 )
        if ( buf[upper_chunk] & bit )
            res++;

    destroy( self );

    return res;
}

bool isEmpty_IntSet( IntSet *self )
{
    bool res = true;
    unsigned int i, upper, upper_chunk, upper_bit;
    unsigned long upper_chunk_mask;
    unsigned long *buf;

    CHECK_TYPE_COMPATIBLE( isEmpty_IntSet, &type_IntSet, self );

    upper = header_IntSet( self )->upper;
    upper_chunk = CHUNK( upper );
    upper_bit = BIT( upper );
    upper_chunk_mask = MASK( upper );
    buf = ARRAY( self );

    for( i = 0; i < upper_chunk && ( res = ( ( buf[i] & FULL_MASK) == 0 ) ); i++ );
    res = res && ( ( buf[upper_chunk] & upper_chunk_mask ) == 0 );

    destroy( self );

    return res;
}

bool contains_IntSet( IntSet *self, unsigned int i )
{
    int res;

    CHECK_TYPE_COMPATIBLE( contains_IntSet, &type_IntSet, self );
    assertion( i <= header_IntSet( self )->upper
             , FORMAT( "contains_IntSet: list index %d is out of bounds [0, %d]" )
             , i
             , header_IntSet( self )->upper
             );

    res = GET( self, i );

    destroy( self );
    return res;
}

bool remove_IntSet( IntSet *self, unsigned int i )
{
    int res;

    CHECK_TYPE_COMPATIBLE( remove_IntSet, &type_IntSet, self );
    assertion( i <= header_IntSet( self )->upper
             , FORMAT( "remove_IntSet: list index %d is out of bounds [0, %d]" )
             , i
             , header_IntSet( self )->upper
             );

    res = GET( self, i );
    UNSET( self, i );

    destroy( self );

    return res;
}

typedef bool (*IntSetOperateFunction)(unsigned long *i1, unsigned long *i2, unsigned long mask);
typedef bool (*IntSetModifyFunction)(unsigned long *i, unsigned long mask);

static bool operateAll_IntSet( IntSet *self, IntSet *set, IntSetOperateFunction f, bool for_each )
{
bool res = for_each;
unsigned int upper, upper_chunk, i;
unsigned long *buf1, *buf2;

  CHECK_TYPE_COMPATIBLE( operateAll_IntSet, &type_IntSet, self );
  CHECK_TYPE_COMPATIBLE( operateAll_IntSet, &type_IntSet, set );

  upper = header_IntSet( self )->upper;
  upper_chunk = CHUNK( upper );
  buf1 = ARRAY( self );
  buf2 = ARRAY( set );

  assertion( upper == header_IntSet( set )->upper
           , FORMAT( "operateAll_IntSet: upper bounds differ (%d and %d)\n" )
           , upper
           , header_IntSet( set )->upper
           );

  if (for_each)
   {for ( i = 0; i < upper_chunk; i++ )
      res = f( &buf1[i], &buf2[i], FULL_MASK ) && res;
    res = f( &buf1[upper_chunk], &buf2[upper_chunk], MASK(upper) ) && res;
   }
  else
   {for ( i = 0; i < upper_chunk; i++ )
      res = f( &buf1[i], &buf2[i], FULL_MASK ) || res;
    res = f( &buf1[upper_chunk], &buf2[upper_chunk], MASK(upper) ) || res;
   }

  return res;
}

static bool modifyAll_IntSet( IntSet *self, IntSetModifyFunction f )
{
    bool res = false;
    int upper, upper_chunk, i;
    unsigned long *buf;

    CHECK_TYPE_COMPATIBLE( modifyAll_IntSet, &type_IntSet, self );

    upper = header_IntSet( self )->upper;
    upper_chunk = CHUNK( upper );
    buf = ARRAY( self );

    for ( i = 0; i < upper_chunk; i++ )
        res = f( &buf[i], FULL_MASK ) || res;
    res = f( &buf[upper_chunk], MASK(upper) ) || res;

    return res;
}

static bool containsFunction_IntSet(unsigned long *i1, unsigned long *i2, unsigned long mask) {
    return ( ( *i1 | *i2 ) & mask ) == ( *i1 & mask );
}

static bool addFunction_IntSet(unsigned long *i1, unsigned long *i2, unsigned long mask) {
    unsigned int i = *i1 & mask;
    *i1 = ( *i1 | *i2 ) & mask;
    return i != *i1;
}

static bool retainFunction_IntSet(unsigned long *i1, unsigned long *i2, unsigned long mask) {
    unsigned int i = *i1 & mask;
    *i1 = ( *i1 & *i2 ) & mask;
    return i != *i1;
}

static bool removeFunction_IntSet(unsigned long *i1, unsigned long *i2, unsigned long mask) {
    unsigned int i = *i1 & mask;
    *i1 = ( *i1 & ~*i2 ) & mask;
    return i != *i1;
}

static bool clearFunction_IntSet(unsigned long *i, unsigned long mask) {
    *i = 0;
    return false;
}

static bool fillFunction_IntSet(unsigned long *i, unsigned long mask) {
    *i = FULL_MASK & mask;
    return false;
}

static bool invertFunction_IntSet(unsigned long *i, unsigned long mask) {
    *i = ( ~*i ) & mask;
    return false;
}

bool containsAll_IntSet( IntSet *self, IntSet *set )
{
    bool res = operateAll_IntSet( self, set, containsFunction_IntSet, true );
    destroy( self );
    destroy( set );
    return res;
}

bool addAll_IntSet( IntSet *self, IntSet *set )
{
    bool res = operateAll_IntSet( self, set, addFunction_IntSet, false );
    destroy( self );
    destroy( set );
    return res;
}

bool retainAll_IntSet( IntSet *self, IntSet *set )
{
    bool res = operateAll_IntSet( self, set, retainFunction_IntSet, false );
    destroy( self );
    destroy( set );
    return res;
}

bool removeAll_IntSet( IntSet *self, IntSet *set )
{
    bool res = operateAll_IntSet( self, set, removeFunction_IntSet, false );
    destroy( self );
    destroy( set );
    return res;
}

void clear_IntSet( IntSet *self )
{
    modifyAll_IntSet( self, clearFunction_IntSet );
    destroy( self );
}

void fill_IntSet( IntSet *self )
{
    modifyAll_IntSet( self, fillFunction_IntSet );
    destroy( self );
}

void invert_IntSet( IntSet *self )
{
    modifyAll_IntSet( self, invertFunction_IntSet );
    destroy( self );
}

/*
 * Returns the element at the specified position in this set treated as list
 * NOTE: Order of elements is unpredictable
 */
unsigned int get_IntSet( IntSet *self, int index )
{
int upper, upper_chunk, upper_bit, i, j, cnt;
unsigned long bit;
unsigned long *buf;
unsigned int res = 0;

  CHECK_TYPE_COMPATIBLE( get_IntSet, &type_IntSet, self );
  assertion( index >= 0
           , FORMAT( "get_IntSet: index %d is less then zero" )
           , index
           );

  upper = header_IntSet( self )->upper;
  upper_chunk = CHUNK( upper );
  upper_bit = BIT( upper );
  buf = ARRAY( self );
  cnt = 0;

  for ( i = 0; i < upper_chunk; i++ )
    for ( j = 0, bit = 1; j < CHUNK_BIT_SIZE; j++, res++, bit <<= 1 )
      if ( buf[i] & bit )
       {if (cnt == index)
         {destroy( self );
          return res;
         }
        cnt++;
       }
  for ( j = 0, bit = 1; j <= upper_bit; j++, res++, bit <<= 1 )
    if ( buf[i] & bit )
     {if (cnt == index)
       {destroy( self );
        return res;
       }
      cnt++;
     }

  assertion( false
           , FORMAT( "get_IntSet: index %d is greater then or equal to size of the set %d" )
           , index
           , cnt
           );

  return res;
}

/*
 * Returns set that contains all elements from this set
 */
Set *toSet_IntSet( IntSet *self )
{
  CHECK_TYPE_COMPATIBLE( toSet_IntSet, &type_IntSet, self );
  return toSet_List (toList_IntSet (self));
}

/*
 * Returns list that contains all elements from this set
 */
List *toList_IntSet( IntSet *self )
{
    List *res = create_List( &type_UInteger );
    int upper, upper_chunk, upper_bit, i, j, k;
    unsigned long bit;
    unsigned long *buf;

    CHECK_TYPE_COMPATIBLE( toList_IntSet, &type_IntSet, self );

    upper = header_IntSet( self )->upper;
    upper_chunk = CHUNK( upper );
    upper_bit = BIT( upper );
    buf = ARRAY( self );

    for ( i = 0, k = 0; i < upper_chunk; i++ )
        for ( j = 0, bit = 1; j < CHUNK_BIT_SIZE; j++, k++, bit <<= 1 )
            if ( buf[i] & bit )
                append_List( r(res), create_UInteger( k ) );
    for ( j = 0, bit = 1; j <= upper_bit; j++, k++, bit <<= 1 )
        if ( buf[i] & bit )
            append_List( r(res), create_UInteger( k ) );

    destroy( self );

    return res;
}
