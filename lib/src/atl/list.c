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
 * File: list.c
 * Description:
 *       This file defines the specification type representing
 *       an abstract list container.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/list.h>

#include <atl/array.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <string.h>

typedef struct
{
    int   ref_cnt;
    int   capacity;
    int   size;
    Type *elem_type;
} ListHeader;

struct List {
    ListHeader *header;
};

#define header_List( lst ) (lst->header)

static void init_List( List *lst, va_list *arg_list )
{
    Type *elem_type = va_arg( *arg_list, Type *);
	int capacity = va_arg( *arg_list, int );

    header_List( lst ) = create_Array( sizeof( ListHeader )
                                     , sizeof( Object * )
                                     , capacity
                                     );

    header_List( lst )->size = 0;
    header_List( lst )->elem_type = elem_type;
}

static void copy_List( List *src, List *dst )
{
  clone_Array( header_List( dst ) = header_List( src ) );
}

static void deep_copy_List( List *src, List *dst )
{
int i;

  /* copy header */
  header_List( dst ) = create_Array( sizeof( ListHeader ),
                                     sizeof( Object* ),
                                     header_List( src )->capacity
                                   );
  header_List( dst )->size      = header_List( src )->size;
  header_List( dst )->elem_type = header_List( src )->elem_type;

  /* copy data */
  for( i = 0; i < header_List( dst )->size; i++ )
   {
    ((Object **)(header_List( dst )+1))[i] = r_hard2weak( clone_Object( r( ((Object **)(header_List( src )+1))[i] ) ) );
   }
}

static int compare_List( List *left, List *right )
{
int i;
int size = header_List( left )->size;

/*     assertion(    header_List( left )->elem_type  */
/*                == header_List( right )->elem_type */
/*              , FORMAT( "compare_List: element types are different: '%s' and '%s'" ) */
/*              , header_List( left )->elem_type->name */
/*              , header_List( right )->elem_type->name */
/*              ); */

/*     assertion( header_List( left )->elem_type != NULL */
/*              , FORMAT( "compare_List: element types are undefined" ) */
/*              ); */

  if(  size != header_List( right )->size )
   {
    return size - header_List( right )->size;
   }

  for( i = 0; i < size; i++ )
   {
    int res = compare( get_List( r( left ) , i )
                     , get_List( r( right ), i )
                     );
    if( res != 0 ) return res;
   }
  return 0;
}

static String *to_string_List( List *lst )
{
StringBuffer *res = create_StringBuffer();
int size = header_List( lst )->size;
int i;

  append_StringBuffer(r(res), "< ");

  for( i = 0; i < size; i++ )
  {
    if (i > 0)	append_StringBuffer( r(res), ", " );
    appendString_StringBuffer( r(res), toString( get_List(r(lst), i) ) );
  }

  append_StringBuffer( r(res), size>0 ? " >" : ">" );

  return toString(res);
}

static String *to_XML_List( List *lst )
{
StringBuffer *res = create_StringBuffer();
int size = header_List( lst )->size;
int i;

	appendString_StringBuffer( r(res), format_String( "<object kind=\"spec\" type=\"List\" text=\"List [%d]\">\n", size) );

	for( i = 0; i < size; i++ ) {
		appendString_StringBuffer( r(res), toXML( get_List(r(lst), i) ) );
	}

	append_StringBuffer( r(res), "</object>" );

	return toString(res);
}

static bool check_invariant_List( List *lst )
{
int i;
int size = header_List( lst )->size;

  for(i=0; i<size; i++)
   {if (!checkInvariant( get_List(r(lst),i) ))
     {destroy(lst);
      return false;
     }
   }
  destroy(lst);
  return true;
}

static void enumerate_List( List *lst, EnumerationCallbackFuncType callback, void* par )
{
int i;
int size = header_List( lst )->size;
Object** buf = (Object**)(header_List( lst ) + 1);

  for(i=0;i<size;i++)
   {
    if (buf[i] != NULL)
     {
      if ( !callback( buf[i], par ) )
        return;
     }
   }
}

static void destroy_List( List *lst )
{
  if (header_List( lst )->ref_cnt == 1)
   {
    int i;
    int size = header_List( lst )->size;
    Object** buf = (Object**)(header_List( lst ) + 1);
    for( i = 0; i < size; i++ )
     {/* destroy all internal references */
      detach_Object( buf[i] );
      buf[i] = NULL;
     }
   }
  destroy_Array( header_List( lst ) );
}

const Type type_List
    = { "List"
      , sizeof(List)
      , &type_Object
      , (Init)init_List
      , (Copy)deep_copy_List
      , (Compare)compare_List
      , (ToString)to_string_List
      , (ToXML)to_XML_List
      , (CheckInvariant)check_invariant_List
      , (Enumerate)enumerate_List
      , (Destroy)destroy_List
      };

List *create_List( const Type *elem_type )
{
    return create( &type_List, elem_type, DEFAULT_INITIAL_LIST_SIZE );
}

List *createCustomized_List( const Type *elem_type, int initialCapacity )
{
    if (initialCapacity < 0)
		initialCapacity = 0;

    return create( &type_List, elem_type, initialCapacity );
}

static void make_unique_array_List( List *lst )
{
ListHeader *new_header;
int i;

  if( header_List( lst )->ref_cnt == 1 ) return;

  new_header = create_Array( sizeof( ListHeader )
                           , sizeof( Object * )
                           , header_List( lst )->capacity
                           );

  new_header->size = header_List( lst )->size;
  new_header->elem_type = header_List( lst )->elem_type;

  for( i = 0; i < new_header->size; i++ )
   {
    ((Object **)(new_header+1))[i] = r_hard2weak( get_List( r( lst ), i ) );
   }

  header_List( lst )->ref_cnt--;
  header_List( lst ) = new_header;
}

static void expand_List( List *lst )
{
    if( header_List( lst )->size < header_List( lst )->capacity ) return;

    /* make_unique_array_List( lst ); */

    header_List( lst ) = expand_Array( header_List( lst )
                                     , sizeof( ListHeader )
                                     , sizeof( Object * )
                                     );
}

void add_List( List *self, int index, Object *ref )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( add_List, &type_List, self );
    CHECK_TYPE_COMPATIBLE( add_List, header_List( self )->elem_type, ref );

    assertion( 0 <= index && index <= header_List( self )->size
             , FORMAT( "add_List: list index %d is out of bounds [0, %d]" )
             , index
             , header_List( self )->size
             );

    make_unique_array_List(self);

    expand_List( self );

    buf = (Object **)(header_List( self ) + 1);

    memmove( buf + index + 1
           , buf + index
           , ( header_List( self )->size - index ) * sizeof(Object*)
           );

    buf[index] = ref;
    header_List( self )->size++;

    r_hard2weak( ref );
    destroy( self );
}

void append_List( List *self, Object *ref )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( append_List, &type_List, self );
    CHECK_TYPE_COMPATIBLE( append_List, header_List( self )->elem_type, ref );

    make_unique_array_List(self);

    expand_List( self );

    buf = (Object **)(header_List( self ) + 1);
    buf[header_List( self )->size] = ref;
    header_List( self )->size++;

    r_hard2weak( ref );
    destroy( self );
}

void clear_List( List *self )
{
int i = 0;
int size = header_List( self )->size;

  CHECK_TYPE_COMPATIBLE( clear_List, &type_List, self );

  if( header_List( self )->ref_cnt > 1 )
   {
    Type *elem_type = header_List( self )->elem_type;
    header_List( self )->ref_cnt--;
    header_List( self ) = create_Array( sizeof( ListHeader )
                                      , sizeof( Object * )
                                      , CONTAINER_INITIAL_CAPACITY
                                      );
    header_List( self )->size = 0;
    header_List( self )->elem_type = elem_type;
   }
  else
   {
    Object** buf = (Object **)(header_List( self ) + 1);
    for( i = 0; i < size; i++ )
     {/* destroy all internal references */
      detach_Object( buf[i] );
     }
    header_List( self )->size = 0;
   }

  destroy( self );
}

bool contains_List( List *self, Object *ref )
{
    int i;
    int size = header_List( self )->size;

    CHECK_TYPE_COMPATIBLE( contains_List, &type_List, self );

/*     assertion( header_List( self )->elem_type != 0 */
/*              , FORMAT( "contains_List: don't call when elem_type(list) == 0" ) */
/*              ); */

    CHECK_TYPE_COMPATIBLE( contains_List, header_List( self )->elem_type, ref );

    for( i = 0; i < size; i++ )
    {
        if( compare( get_List( r( self ), i ), r( ref ) ) == 0 )
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

Object* get_List( List *self, int index )
{
    Object **buf = (Object **)(header_List( self ) + 1);
    Object *res;

    CHECK_TYPE_COMPATIBLE( get_List, &type_List, self );

    assertion( 0 <= index && index < header_List( self )->size
             , FORMAT( "get_List: list index %d is out of bounds [0, %d[" )
             , index
             , header_List( self )->size
             );

    res = r( ((Object **)buf)[index] );

    destroy( self );

    return res;
}

int indexOf_List( List *self, Object *ref )
{
    int i;
    int size = header_List( self )->size;

    CHECK_TYPE_COMPATIBLE( indexOf_List, &type_List, self );

/*     assertion( header_List( self )->elem_type != 0 */
/*              , FORMAT( "indexOf_List: don't call when elem_type(list) == 0" ) */
/*              ); */

    CHECK_TYPE_COMPATIBLE( indexOf_List, header_List( self )->elem_type, ref );

    for( i = 0; i < size; i++ )
    {
        if( compare( get_List( r( self ), i ), r( ref ) ) == 0 )
        {
            destroy( self );
            destroy( ref );
            return i;
        }
    }
    destroy( self );
    destroy( ref );
    return -1;
}

bool isEmpty_List( List *self )
{
    bool res;

    CHECK_TYPE_COMPATIBLE( isEmpty_List, &type_List, self );

    res = ( header_List( self )->size == 0 ) ? true : false;

    destroy( self );

    return res;
}

int lastIndexOf_List( List *self, Object *ref )
{
    int i;

    CHECK_TYPE_COMPATIBLE( lastIndexOf_List, &type_List, self );

/*     assertion( header_List( self )->elem_type != 0 */
/*              , FORMAT( "lastIndexOf_List: don't call when elem_type(list) == 0" ) */
/*              ); */

    CHECK_TYPE_COMPATIBLE( lastIndexOf_List, header_List( self )->elem_type, ref );

    for( i = header_List( self )->size - 1; i >= 0; i-- )
    {
        if( compare( get_List( r( self ), i ), r( ref ) ) == 0 )
        {
            destroy( self );
            destroy( ref );
            return i;
        }
    }
    destroy( self );
    destroy( ref );
    return -1;
}

void remove_List( List *self, int index )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( remove_List, &type_List, self );

    assertion( 0 <= index && index < header_List( self )->size
             , FORMAT( "remove_List: list index %d is out of bounds [0, %d[" )
             , index
             , header_List( self )->size
             );

    make_unique_array_List(self);

    buf = (Object **)(header_List( self ) + 1);

    detach_Object( buf[index] );

    memmove( buf + index
           , buf + index + 1
           , ( header_List( self )->size - index - 1 ) * sizeof( Object * )
           );

    header_List( self )->size--;

    destroy( self );
}

void set_List( List *self, int index, Object *ref )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( set_List, &type_List, self );
    CHECK_TYPE_COMPATIBLE( set_List, header_List( self )->elem_type, ref );

    assertion( 0 <= index && index < header_List( self )->size
             , FORMAT( "remove_List: list index %d is out of bounds [0, %d[" )
             , index
             , header_List( self )->size
             );

    make_unique_array_List( self );

    buf = (Object **)(header_List( self ) + 1);
    detach_Object( buf[index] );
    buf[index] = ref;

    r_hard2weak( ref );
    destroy( self );
}

int size_List( List *self )
{
    int res;

    CHECK_TYPE_COMPATIBLE( size_List, &type_List, self );

    res = header_List( self )->size;

    destroy( self );

    return res;
}

List *subList_List( List *self, int fromIndex, int toIndex )
{
    List *res = create_List( header_List( self )->elem_type );
    int i;

    CHECK_TYPE_COMPATIBLE( subList_List, &type_List, self );

    assertion(    0 <= fromIndex
               && 0 <= toIndex
               && fromIndex <= header_List( self )->size
               && toIndex <= header_List( self )->size
             , FORMAT( "subList_List: wrong fromIndex=%d or toIndex=%d for list of size=%d" )
             , fromIndex
             , toIndex
             , header_List( self )->size
             );

    for( i = fromIndex; i < toIndex; i++ )
    {
        append_List( r(res), get_List( r( self ), i ) );
    }

    destroy(self);
    return res;
}


void addAll_List(List *self, int index, List *other) {
    int size, i;

    CHECK_TYPE_COMPATIBLE(addAll_List, &type_List, self);
    CHECK_TYPE_COMPATIBLE(addAll_List, &type_List, other);

/*     assertion(    header_List(self)->elem_type == 0 */
/*                || header_List(self)->elem_type == header_List( other )->elem_type, */
/*                FORMAT("addAll_List: '%s' type required, '%s' found"), */
/*                header_List(self)->elem_type ? header_List(self)->elem_type->name : "<null>", */
/*                header_List(other)->elem_type ? header_List(other)->elem_type->name : "<null>" */
/*              ); */

    size = header_List( other )->size;

    for (i = size; i > 0; add_List(r(self), index, get_List(r(other), --i)));

    destroy(other);
    destroy(self);
}

void appendAll_List(List *self, List *other) {
    int size, i;

    CHECK_TYPE_COMPATIBLE(appendAll_List, &type_List, self);
    CHECK_TYPE_COMPATIBLE(appendAll_List, &type_List, other);

    size = header_List( other )->size;

    for (i = 0; i < size; append_List(r(self), get_List(r(other), i++)));

    destroy(other);
    destroy(self);
}

Set *toSet_List(List *self) {
    int size, i;
    Set *set;

    CHECK_TYPE_COMPATIBLE(toSet_List, &type_List, self);

/*     assertion( header_List(self)->elem_type != 0, */
/*                "toSet_List: untyped list" */
/*              ); */

    size = header_List( self )->size;
    set = create_Set(header_List(self)->elem_type);

    for (i = 0; i < size; add_Set(r(set), get_List(r(self), i++)));

    destroy(self);

    return set;
}

Type *elemType_List (List *self) {
    Type *t = header_List(self)->elem_type;
    destroy(self);
    return t;
}
