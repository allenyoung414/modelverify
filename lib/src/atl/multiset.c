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
#include <atl/multiset.h>
#include <atl/object_int.h>
#include <atl/array.h>
//#include <common/multiset.h>
//#include <common/object_int.h>
//#include <common/array.h>
#include <atl/list.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <string.h>

typedef struct MultiSetHeader
{
    int   ref_cnt;
    int   capacity;
    int   size;
    Type *elem_type;
}
MultiSetHeader;

struct MultiSet {
    MultiSetHeader *header;
};

#define header_MultiSet( set ) ( set->header )

static void init_MultiSet( MultiSet *set, va_list *arg_list )
{
Type *elem_type = va_arg( *arg_list, Type *);
  header_MultiSet( set ) = create_Array( sizeof( MultiSetHeader )
                                       , sizeof( Object * )
                                       , CONTAINER_INITIAL_CAPACITY
                                       );

  header_MultiSet( set )->size = 0;
  header_MultiSet( set )->elem_type = elem_type;
}

static void copy_MultiSet( MultiSet *src, MultiSet *dst )
{
  clone_Array( header_MultiSet( dst ) = header_MultiSet( src ) );
}

static void deep_copy_MultiSet( MultiSet *src, MultiSet *dst )
{
int i;

  /* copy header */
  header_MultiSet( dst ) = create_Array( sizeof( MultiSetHeader ),
                                         sizeof( Object* ),
                                         header_MultiSet( src )->capacity
                                       );
  header_MultiSet( dst )->size      = header_MultiSet( src )->size;
  header_MultiSet( dst )->elem_type = header_MultiSet( src )->elem_type;

  /* copy data */
  for( i = 0; i < header_MultiSet( dst )->size; i++ )
   {
    ((Object **)(header_MultiSet( dst )+1))[i] = clone_Object( r( ((Object **)(header_MultiSet( src )+1))[i] ) );
   }
}

static int compare_MultiSet( MultiSet *left, MultiSet *right )
{
int i,j,cnt;
Object **buff_left,**buff_right;
Object* current;
int size = header_MultiSet( left )->size;

  // Check size
  if(  size != header_MultiSet( right )->size )
   {
    return size - header_MultiSet( right )->size;
   }

  buff_left  = (Object **)(header_MultiSet( left ) + 1);
  buff_right = (Object **)(header_MultiSet( right ) + 1);
  for( i = 0; i < size; i++ )
   {cnt = 0;
    current = buff_left[i];
    // Count number of the current element in the right object
    for( j = 0; j < size; j++ )
     {
      if (compare( r(current), r(buff_right[j]) ) == 0)
        cnt--;
     }
    if (cnt == 0)
      return -1;
    // Count number of the current element in the left object
    for( j = 0; j < size; j++ )
     {
      if (j == i)
       {cnt++;
        continue;
       }
      if (compare( r(current), r(buff_left[j]) ) == 0)
        cnt++;
     }
    if (cnt != 0)
      return cnt;
   }

  return 0;
}

static String* to_string_MultiSet( MultiSet* set )
{
StringBuffer *res = create_StringBuffer();
int size = header_MultiSet( set )->size;
int i;

	append_StringBuffer(r(res), "< ");

    for( i = 0; i < size; i++ )
    {
		if (i > 0)	append_StringBuffer( r(res), ", " );
        appendString_StringBuffer( r(res), toString( get_MultiSet(r(set), i) ) );
    }

    append_StringBuffer( r(res), size>0 ? " >" : ">" );

    return toString(res);
}

static String* to_XML_MultiSet( MultiSet* set )
{
StringBuffer *res = create_StringBuffer();
int size = header_MultiSet( set )->size;
int i;

	appendString_StringBuffer(r(res), format_String("<object kind=\"spec\" type=\"MultiSet\" text=\"MultiSet [%d]\">\n", size) );

    for( i = 0; i < size; i++ ) {
        appendString_StringBuffer( r(res), toXML( get_MultiSet(r(set), i) ) );
    }

	append_StringBuffer( r(res), "</object>" );

    return toString(res);
}

static bool check_invariant_MultiSet(MultiSet* set)
{
int i;
int size = header_MultiSet( set )->size;

  for(i=0; i<size; i++)
   {if (!checkInvariant( get_MultiSet(r(set),i) ))
     {destroy(set);
      return false;
     } 
   }
  destroy(set);
  return true;
}

static void enumerate_MultiSet(MultiSet* set, EnumerationCallbackFuncType callback, void* par)
{
int i;
int size = header_MultiSet( set )->size;
Object** buf = (Object**)(header_MultiSet( set ) + 1);

  for( i = 0; i < size; i++ )
   {
    if (buf[i] != NULL)
     {
      if ( !callback( buf[i], par ) )
        return;
     }
   }
}

static void destroy_MultiSet( MultiSet *set )
{
  if (header_MultiSet( set )->ref_cnt == 1)
   {
    int i;
    int size = header_MultiSet( set )->size;
    Object** buf = (Object**)(header_MultiSet( set ) + 1);
    for( i = 0; i < size; i++ )
     {/* destroy all internal references */
      detach_Object( buf[i] );
      buf[i] = NULL;
     }
   }
  destroy_Array( header_MultiSet( set ) );
}

const Type type_MultiSet
    = { "MultiSet"
      , sizeof(MultiSet)
      , &type_Object
      , (Init)init_MultiSet
      , (Copy)deep_copy_MultiSet
      , (Compare)compare_MultiSet
      , (ToString)to_string_MultiSet
      , (ToXML)to_XML_MultiSet
      , (CheckInvariant)check_invariant_MultiSet
      , (Enumerate)enumerate_MultiSet
      , (Destroy)destroy_MultiSet
      };

MultiSet *create_MultiSet( const Type *elem_type )
{
    return create( &type_MultiSet, elem_type );
}

static void make_unique_array_MultiSet( MultiSet *set )
{
    MultiSetHeader *new_header;
    int i;

    if( header_MultiSet( set )->ref_cnt == 1 ) return;
    
    new_header = create_Array( sizeof( MultiSetHeader )
                             , sizeof( Object * )
                             , header_MultiSet( set )->capacity
                             );

    new_header->size = header_MultiSet( set )->size;
    new_header->elem_type = header_MultiSet( set )->elem_type;

    for( i = 0; i < new_header->size; i++ )
    {
        ((Object **)(new_header+1))[i] = r_hard2weak( get_MultiSet( r( set ), i ) );
    }

    header_MultiSet( set )->ref_cnt--;
    header_MultiSet( set ) = new_header;
}

static void expand_MultiSet( MultiSet *set )
{
    if( header_MultiSet( set )->size < header_MultiSet( set )->capacity ) return;

    // make_unique_array_MultiSet( set );

    header_MultiSet( set ) = expand_Array( header_MultiSet( set )
                                    , sizeof( MultiSetHeader )
                                    , sizeof( Object * )
                                    );
}

void remove_internal_MultiSet( MultiSet *self, int index )
{
    Object **buf;

    CHECK_TYPE_COMPATIBLE( remove_MultiSet, &type_MultiSet, self );

    assertion( 0 <= index && index < header_MultiSet( self )->size
             , FORMAT( "remove_MultiSet: multiset index %d is out of bounds [0, %d[" )
             , index
             , header_MultiSet( self )->size
             );

    make_unique_array_MultiSet(self);
    
    buf = (Object **)(header_MultiSet( self ) + 1);

    detach_Object( buf[index] );

    memmove( &buf[index]
           , &buf[index + 1]
           , ( --header_MultiSet( self )->size - index ) * sizeof( Object * )
           );

    destroy( self );
}

int add_MultiSet( MultiSet *self, Object *ref )
{
    Object **buf;

    CHECK_REFERENCE(add_MultiSet, ref);
    
    CHECK_TYPE_COMPATIBLE( add_MultiSet, &type_MultiSet, self );
    CHECK_TYPE_COMPATIBLE( add_MultiSet, header_MultiSet( self )->elem_type, ref );

    make_unique_array_MultiSet(self);
    
    expand_MultiSet( self );
    
    buf = (Object **)(header_MultiSet( self ) + 1);

    buf[header_MultiSet( self )->size++] = ref;

    r_hard2weak( ref );
    destroy( self );
    return true;
}

int size_MultiSet( MultiSet *self )
{
    int res;

    CHECK_TYPE_COMPATIBLE( size_MultiSet, &type_MultiSet, self );

    res = header_MultiSet( self )->size;

    destroy( self );

    return res;
}

bool isEmpty_MultiSet( MultiSet *self )
{
    bool res;

    CHECK_TYPE_COMPATIBLE( isEmpty_MultiSet, &type_MultiSet, self );

    res = ( header_MultiSet( self )->size == 0 ) ? true : false;

    destroy( self );

    return res;
}

int contains_MultiSet( MultiSet *self, Object *ref )
{
int i;
int size,res;
Object** buf;

    CHECK_TYPE_COMPATIBLE( contains_MultiSet, &type_MultiSet, self );

    size = header_MultiSet( self )->size;
    buf = (Object**)(header_MultiSet( self ) + 1);
    res = 0;
    
    for( i = 0; i < size; i++ )
     {
      if( compare( r( buf[i] ), r( ref ) ) == 0 ) 
       {
        res++;
       }
     }
    destroy( self );
    destroy( ref );
    return res;
}

bool remove_MultiSet( MultiSet *self, Object *ref )
{
    int index = -1, i, size;

    CHECK_TYPE_COMPATIBLE( remove_MultiSet, &type_MultiSet, self );
/*     CHECK_TYPE( remove_MultiSet, header_MultiSet( self )->elem_type, ref ); */

    size = header_MultiSet( self )->size;

    for ( i = 0; (i < size) && (index < 0); i++ )
      if ( compare( r( ref ), get_MultiSet( r( self ), i ) ) == 0 )
        index = i;

    if ( index >= 0 )
      remove_internal_MultiSet( self, index );

    /* destroy( self ); in remove_internal_MultiSet */
    destroy( ref );

    return index >= 0;
}

int removeCount_MultiSet( MultiSet *self, Object *ref, int count )
{
    int i, size, res;

    CHECK_TYPE_COMPATIBLE( remove_MultiSet, &type_MultiSet, self );
/*     CHECK_TYPE( remove_MultiSet, header_MultiSet( self )->elem_type, ref ); */

    size = header_MultiSet( self )->size;

    res = 0;
    for ( i = 0; i < size; i++ )
     {
      if ( compare( r( ref ), get_MultiSet( r( self ), i ) ) == 0 )
       {
        remove_internal_MultiSet( r(self), i );
        res++;
        if( count > 0 && res >= count )
        {
            break;
        }
        i--;
        size--;
       }
     }

    destroy( self );
    destroy( ref );

    return res;
}

int removeFull_MultiSet( MultiSet *self, Object *ref )
{
    return removeCount_MultiSet( self, ref, -1 );
}

int removeAll_MultiSet( MultiSet *self, MultiSet *set )
{
Set* elems;
Object* obj;
int i, size, res;

    elems = toSet_MultiSet( r(set) );
    size = size_Set( r(elems) );

    res = 0;
    for ( i = 0; i < size; i++ )
    {
        obj = get_Set( r(elems), i );
        res += removeCount_MultiSet( r(self), r(obj), contains_MultiSet( r(set), r(obj) ) );
        destroy(obj);
    }

    destroy( self );
    destroy( set );
    destroy( elems );
    return res;
}

bool containsAll_MultiSet( MultiSet *self, MultiSet *set )
{
    int size, i;
  
    CHECK_TYPE_COMPATIBLE( containsAll_MultiSet, &type_MultiSet, self );
    CHECK_TYPE_COMPATIBLE( containsAll_MultiSet, &type_MultiSet, set );

    size = header_MultiSet( set )->size;
        
    if ( header_MultiSet( self )->size < size )
     {
      destroy( self );
      destroy( set );

      return false;
     }

    for ( i = 0; i < size; i++ )
     {
        if ( contains_MultiSet( r(self), get_MultiSet( r(set), i ) ) <
             contains_MultiSet( r(set), get_MultiSet( r(set), i ) )
           )
         {
            destroy( self );
            destroy( set );
      
            return false;
         }
     }

    destroy( self );
    destroy( set );

    return true;
}

bool addAll_MultiSet( MultiSet *self, MultiSet *set )
{
    int i,size;
  
    CHECK_TYPE_COMPATIBLE( addAll_MultiSet, &type_MultiSet, self );
    CHECK_TYPE_COMPATIBLE( addAll_MultiSet, &type_MultiSet, set );

    size = header_MultiSet( set )->size;

    for ( i = 0; i < size; i++ )
     {
      add_MultiSet( r( self ), get_MultiSet( r( set ), i ) );
     }
  
    destroy( self );
    destroy( set );

    return (size > 0);
}

bool retainAll_MultiSet( MultiSet *self, MultiSet *set )
{
bool res = false;
Set* elems;
Object* obj;
int i, size, self_i_count, set_i_count;
  
    CHECK_TYPE_COMPATIBLE( retainAll_MultiSet, &type_MultiSet, self );
    CHECK_TYPE_COMPATIBLE( retainAll_MultiSet, &type_MultiSet, set );

    elems = toSet_MultiSet( r(self) );
    size = size_Set( r(elems) );

    for ( i = 0; i < size; i++ )
    {
        obj = get_Set( r(elems), i );
        self_i_count = contains_MultiSet( r(self), r(obj) );
        set_i_count = contains_MultiSet( r(set), r(obj) );
        if( self_i_count > set_i_count )
        {
            removeCount_MultiSet( r(self), r(obj), self_i_count - set_i_count );
            res = true;
        }
        destroy(obj);
    }

    destroy( self );
    destroy( set );
    destroy( elems );

    return res;
}

void clear_MultiSet( MultiSet *self )
{
    int i = 0;
    int size;
    
    CHECK_TYPE_COMPATIBLE( clear_MultiSet, &type_MultiSet, self );

    size = header_MultiSet( self )->size;

    if( header_MultiSet( self )->ref_cnt > 1 )
     {
      Type *elem_type = header_MultiSet( self )->elem_type;
      header_MultiSet( self )->ref_cnt--;
      header_MultiSet( self ) = create_Array( sizeof( MultiSetHeader )
                                            , sizeof( Object * )
                                            , CONTAINER_INITIAL_CAPACITY
                                            );
      header_MultiSet( self )->size = 0;
      header_MultiSet( self )->elem_type = elem_type;
     }
    else
     {
      Object** buf = (Object**)(header_MultiSet( self ) + 1);
      for( i = 0; i < size; i++ )
       {/* destroy all internal references */
        detach_Object( buf[i] );
       }
      header_MultiSet( self )->size = 0;
     }

    destroy( self );
}

bool equals_MultiSet( MultiSet *self, MultiSet *set )
{
  CHECK_TYPE_COMPATIBLE( equals_MultiSet, &type_MultiSet, self );
  CHECK_TYPE_COMPATIBLE( equals_MultiSet, &type_MultiSet, set );

  return compare( r( self ), r( set ) ) == 0;
}

List *toList_MultiSet( MultiSet *self )
{
MultiSet *lst;

  CHECK_TYPE_COMPATIBLE( toList_MultiSet, &type_MultiSet, self );

  lst = create_MultiSet( header_MultiSet( self )->elem_type );
  copy( self, r(lst) );

  /* (const Type *)header( lst )->type = &type_List; */
  {
   Header* h = header( lst );
   Type** header_type = (Type**)&(h->type);
   *header_type = (Type*)&type_List;
  }
    
  return (List *)lst;
}

Set *toSet_MultiSet( MultiSet *self )
{
Set *set;
int size, i;

  CHECK_TYPE_COMPATIBLE( toSet_MultiSet, &type_MultiSet, self );

  set = create_Set( header_MultiSet( self )->elem_type );
  size = header_MultiSet( self )->size;

  for ( i = 0; i < size; i++ )
  {
    add_Set( r( set ), get_MultiSet( r( self ), i ) );
  }

  destroy( self );
  return set;
}

Object* get_MultiSet( MultiSet *self, int index )
{
Object **buf = (Object **)(header_MultiSet( self ) + 1);
Object *res;

  CHECK_TYPE_COMPATIBLE( get_MultiSet, &type_MultiSet, self );

  assertion( 0 <= index && index < header_MultiSet( self )->size
           , FORMAT( "get_MultiSet: set index %d is out of bounds [0, %d[" )
           , index
           , header_MultiSet( self )->size
           );

  res = r( buf[index] );

  destroy( self );

  return res;
}

Type *elemType_MultiSet(MultiSet *self)
{
    Type *t = header_MultiSet(self)->elem_type;

    CHECK_TYPE_COMPATIBLE( elemType_MultiSet, &type_MultiSet, self );

    destroy(self);
    return t;
}
