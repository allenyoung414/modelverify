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
 * File: set.c
 * Description:
 *       This file defines the specification type representing
 *       an abstract set container.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/set.h>
#include <atl/object_int.h>
#include <atl/array.h>
#include <atl/list.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <string.h>

typedef struct SetHeader
{
    int   ref_cnt;
    int   capacity;
    int   size;
    Type *elem_type;
}
SetHeader;

struct Set {
    SetHeader *header;
};

#define header_Set( set ) ( set->header )

static void init_Set( Set *set, va_list *arg_list )
{
Type *elem_type = va_arg( *arg_list, Type *);
int capacity = va_arg( *arg_list, int );

  header_Set( set ) = create_Array( sizeof( SetHeader )
                                  , sizeof( Object * )
                                  , capacity
                                  );

  header_Set( set )->size = 0;
  header_Set( set )->elem_type = elem_type;
}

static void copy_Set( Set *src, Set *dst )
{
  clone_Array( header_Set( dst ) = header_Set( src ) );
}

static void deep_copy_Set( Set *src, Set *dst )
{
int i;

  /* copy header */
  header_Set( dst ) = create_Array( sizeof( SetHeader ),
                                    sizeof( Object* ),
                                    header_Set( src )->capacity
                                  );
  header_Set( dst )->size      = header_Set( src )->size;
  header_Set( dst )->elem_type = header_Set( src )->elem_type;

  /* copy data */
  for( i = 0; i < header_Set( dst )->size; i++ )
   {
    ((Object **)(header_Set( dst )+1))[i] = r_hard2weak( clone_Object( r( ((Object **)(header_Set( src )+1))[i] ) ) );
   }
}

static int compare_Set( Set *left, Set *right )
{
    int i;
    bool res = true;
    int size = header_Set( left )->size;

/*     assertion(    header_Set( left )->elem_type  */
/*                == header_Set( right )->elem_type */
/*              , FORMAT( "compare_Set: element types are different: '%s' and '%s'" ) */
/*              , header_Set( left )->elem_type->name */
/*              , header_Set( right )->elem_type->name */
/*              ); */

    if(  size != header_Set( right )->size )
    {
        return size - header_Set( right )->size;
    }

    for( i = 0; i < size && res; res = contains_Set( r(right), get_Set( r( left ) , i++ ) ) );

    return !res;
}

static String* to_string_Set( Set* set )
{
StringBuffer *res = create_StringBuffer();
int size = header_Set( set )->size;
int i;

	append_StringBuffer(r(res), "{ ");
    for( i = 0; i < size; i++ )
    {
		if (i > 0)	append_StringBuffer( r(res), ", " );
        appendString_StringBuffer( r(res), toString( get_Set(r(set), i) ) );
    }

    append_StringBuffer( r(res), size>0 ? " }" : "}" );

    return toString(res);
}

static String* to_XML_Set( Set* set )
{
StringBuffer *res = create_StringBuffer();
int size = header_Set( set )->size;
int i;

	appendString_StringBuffer( r(res), format_String("<object kind=\"spec\" type=\"Set\" text=\"Set [%d]\">\n", size) );
    for( i = 0; i < size; i++ ) {
        appendString_StringBuffer( r(res), toXML( get_Set(r(set), i) ) );
    }

	append_StringBuffer( r(res), "</object>" );

    return toString(res);
}

static bool check_invariant_Set(Set* set)
{
int i;
int size = header_Set( set )->size;

  for(i=0; i<size; i++)
   {if (!checkInvariant( get_Set(r(set),i) ))
     {destroy(set);
      return false;
     }
   }
  destroy(set);
  return true;
}

static void enumerate_Set(Set* set, EnumerationCallbackFuncType callback, void* par)
{
int i;
int size = header_Set( set )->size;
Object** buf = (Object**)(header_Set( set ) + 1);

  for(i=0; i<size; i++)
   {
    if (buf[i] != NULL)
     {
      if ( !callback( buf[i], par ) )
        return;
     }
   }
}

static void destroy_Set( Set *set )
{
  if (header_Set( set )->ref_cnt == 1)
   {
    int i;
    int size = header_Set( set )->size;
    Object** buf = (Object**)(header_Set( set ) + 1);
    for( i = 0; i < size; i++ )
     {/* destroy all internal references */
      detach_Object( buf[i] );
      buf[i] = NULL;
     }
   }
  destroy_Array( header_Set( set ) );
}

const Type type_Set
    = { "Set"
      , sizeof(Set)
      , &type_Object
      , (Init)init_Set
      , (Copy)deep_copy_Set
      , (Compare)compare_Set
      , (ToString)to_string_Set
      , (ToXML)to_XML_Set
      , (CheckInvariant)check_invariant_Set
      , (Enumerate)enumerate_Set
      , (Destroy)destroy_Set
      };

Set *create_Set( const Type *elem_type )
{
    return create( &type_Set, elem_type, DEFAULT_INITIAL_SET_SIZE );
}

Set *createCustomized_Set( const Type *elem_type, int initialCapacity )
{
    if (initialCapacity < 0)
		initialCapacity = 0;

    return create( &type_Set, elem_type, initialCapacity );
}

static void make_unique_array_Set( Set *set )
{
    SetHeader *new_header;
    int i;

    if( header_Set( set )->ref_cnt == 1 ) return;

    new_header = create_Array( sizeof( SetHeader )
                             , sizeof( Object * )
                             , header_Set( set )->capacity
                             );

    new_header->size = header_Set( set )->size;
    new_header->elem_type = header_Set( set )->elem_type;

    for( i = 0; i < new_header->size; i++ )
    {
        ((Object **)(new_header+1))[i] = r_hard2weak( get_Set( r( set ), i ) );
    }

    header_Set( set )->ref_cnt--;
    header_Set( set ) = new_header;
}

static void expand_Set( Set *set )
{
    if( header_Set( set )->size < header_Set( set )->capacity ) return;

    /* make_unique_array_Set( set ); */

    header_Set( set ) = expand_Array( header_Set( set )
                                    , sizeof( SetHeader )
                                    , sizeof( Object * )
                                    );
}

static void remove_internal_Set( Set *self, int index )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( remove_Set, &type_Set, self );

    assertion( 0 <= index && index < header_Set( self )->size
             , FORMAT( "remove_Set: set index %d is out of bounds [0, %d[" )
             , index
             , header_Set( self )->size
             );

    make_unique_array_Set(self);

    buf = (Object **)(header_Set( self ) + 1);

    detach_Object( buf[index] );

    memmove( &buf[index]
           , &buf[index + 1]
           , ( --header_Set( self )->size - index ) * sizeof( Object * )
           );

    destroy( self );
}

bool add_Set( Set *self, Object *ref )
{
    Object **buf;

    CHECK_REFERENCE(add_Set, ref);

    CHECK_TYPE_COMPATIBLE( add_Set, &type_Set, self );
    CHECK_TYPE_COMPATIBLE( add_Set, header_Set( self )->elem_type, ref );

    if ( contains_Set( r( self ), r( ref ) ) ) {
      destroy( self );
      destroy( ref );
      return false;
    }

    make_unique_array_Set(self);

    expand_Set( self );

    buf = (Object **)(header_Set( self ) + 1);

    buf[header_Set( self )->size++] = ref;

    r_hard2weak( ref );
    destroy( self );
    return true;
}

int size_Set( Set *self )
{
    int res;

    CHECK_TYPE_COMPATIBLE( size_Set, &type_Set, self );

    res = header_Set( self )->size;

    destroy( self );

    return res;
}

bool isEmpty_Set( Set *self )
{
    bool res;

    CHECK_TYPE_COMPATIBLE( isEmpty_Set, &type_Set, self );

    res = ( header_Set( self )->size == 0 ) ? true : false;

    destroy( self );

    return res;
}

bool contains_Set( Set *self, Object *ref )
{
    int i;
    int size;

    CHECK_TYPE_COMPATIBLE( contains_Set, &type_Set, self );

/*     assertion( header_Set( self )->elem_type != 0 */
/*              , FORMAT( "contains_Set: don't call when elem_type(set) == 0" ) */
/*              ); */

/*     CHECK_TYPE( contains_Set, header_Set( self )->elem_type, ref ); */

    size = header_Set( self )->size;

    for( i = 0; i < size; i++ )
    {
        if( compare( get_Set( r( self ), i ), r( ref ) ) == 0 )
        {
            destroy( self );
            destroy( ref );
            return true;
        }
    }
    destroy( self );
    destroy( ref );
    return false;
}

bool remove_Set( Set *self, Object *ref )
{
    int index = -1, i, size;

    CHECK_TYPE_COMPATIBLE( remove_Set, &type_Set, self );
/*     CHECK_TYPE( remove_Set, header_Set( self )->elem_type, ref ); */

    size = header_Set( self )->size;

    for ( i = 0; (i < size) && (index < 0); i++ )
      if ( compare( r( ref ), get_Set( r( self ), i ) ) == 0 )
        index = i;

    if ( index >= 0 )
      remove_internal_Set( self, index );

    /* destroy( self ); in remove_internal_Set */
    destroy( ref );

    return index >= 0;
}

bool containsAll_Set( Set *self, Set *set )
{
    int size, i;

    CHECK_TYPE_COMPATIBLE( containsAll_Set, &type_Set, self );
    CHECK_TYPE_COMPATIBLE( containsAll_Set, &type_Set, set );

    size = header_Set( set )->size;

    if ( header_Set( self )->size < size )
     {
      destroy( self );
      destroy( set );

      return false;
     }

    for ( i = 0; i < size; i++ )
        if ( !contains_Set( r( self ), get_Set( r( set ), i ) ) ) {
            destroy( self );
            destroy( set );

            return false;
        }

    destroy( self );
    destroy( set );

    return true;
}

bool addAll_Set( Set *self, Set *set )
{
    bool res = false;
    int i,size;

    CHECK_TYPE_COMPATIBLE( addAll_Set, &type_Set, self );
    CHECK_TYPE_COMPATIBLE( addAll_Set, &type_Set, set );

    size = header_Set( set )->size;

    for ( i = 0; i < size; i++ )
      res = add_Set( r( self ), get_Set( r( set ), i ) ) || res;

    destroy( self );
    destroy( set );

    return res;
}

bool retainAll_Set( Set *self, Set *set )
{
    bool res = false;
    int i;

    CHECK_TYPE_COMPATIBLE( retainAll_Set, &type_Set, self );
    CHECK_TYPE_COMPATIBLE( retainAll_Set, &type_Set, set );

    for ( i = 0; i < header_Set( self )->size; i++ )
      if ( !contains_Set( r( set ), get_Set( r( self ), i ) ) )
       {
        remove_internal_Set( r( self ), i-- );
        res = true;
      }

    destroy( self );
    destroy( set );

    return res;
}

bool removeAll_Set( Set *self, Set *set )
{
    bool res = false;
    int i;

    CHECK_TYPE_COMPATIBLE( removeAll_Set, &type_Set, self );
    CHECK_TYPE_COMPATIBLE( removeAll_Set, &type_Set, set );

    if (self == set)
     {
      res = (header_Set( self )->size > 0);
      clear_Set( r(self) );
     }
    else
     {
      for ( i = 0; i < header_Set( set )->size; i++ )
        res = remove_Set( r( self ), get_Set( r( set ), i ) ) || res;
     }

    destroy( self );
    destroy( set );

    return res;
}

void clear_Set( Set *self )
{
    int i = 0;
    int size;

    CHECK_TYPE_COMPATIBLE( clear_Set, &type_Set, self );

    size = header_Set( self )->size;

    if( header_Set( self )->ref_cnt > 1 )
     {
      Type *elem_type = header_Set( self )->elem_type;
      header_Set( self )->ref_cnt--;
      header_Set( self ) = create_Array( sizeof( SetHeader )
                                       , sizeof( Object * )
                                       , CONTAINER_INITIAL_CAPACITY
                                       );
      header_Set( self )->size = 0;
      header_Set( self )->elem_type = elem_type;
     }
    else
     {
      Object** buf = (Object**)(header_Set( self ) + 1);
      for( i = 0; i < size; i++ )
       {/* destroy all internal references */
        detach_Object( buf[i] );
       }
      header_Set( self )->size = 0;
     }

    destroy( self );
}

bool equals_Set( Set *self, Set *set )
{
  CHECK_TYPE_COMPATIBLE( equals_Set, &type_Set, self );
  CHECK_TYPE_COMPATIBLE( equals_Set, &type_Set, set );

  return compare( r( self ), r( set ) ) == 0;
}

List *toList_Set( Set *self )
{
Set *lst;

  CHECK_TYPE_COMPATIBLE( toList_Set, &type_Set, self );

  lst = create_Set( header_Set( self )->elem_type );
  copy( self, r(lst) );

  header( lst )->type = &type_List;

  return (List *)lst;
}

Object* get_Set( Set *self, int index )
{
Object **buf = (Object **)(header_Set( self ) + 1);
Object *res;

  CHECK_TYPE_COMPATIBLE( get_Set, &type_Set, self );

  assertion( 0 <= index && index < header_Set( self )->size
           , FORMAT( "get_Set: set index %d is out of bounds [0, %d[" )
           , index
           , header_Set( self )->size
           );

  res = r( buf[index] );

  destroy( self );

  return res;
}

Type *elemType_Set (Set *self)
{
    Type *t = header_Set(self)->elem_type;

    CHECK_TYPE_COMPATIBLE( elemType_Set, &type_Set, self );

    destroy(self);
    return t;
}
