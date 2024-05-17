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
 * File: map.c
 * Description:
 *       This file defines the specification type representing
 *       an abstract map container.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/map.h>

#include <atl/array.h>
#include <atl/list.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <string.h>

typedef struct MapHeader
{
    int   ref_cnt;
    int   capacity;
    int   size;
    Type *key_type;
    Type *val_type;
}
MapHeader;

struct Map {
    MapHeader *header;
};

typedef
struct
{
    Object *key;
    Object *val;
}
MapEntry;

#define header_Map( map ) ( ( map )->header )

static MapEntry *entry_Map( Map *self, int index )
{
MapEntry *buf = (MapEntry*)(header_Map( self ) + 1);

  assertion( 0 <= index && index < header_Map( self )->size
           , FORMAT( "entry_Map: map index %d is out of bounds [0, %d[" )
           , index
           , header_Map( self )->size
           );

  return buf + index;
}

static void init_Map( Map *map, va_list *arg_list )
{
    Type *key_type = va_arg( *arg_list, Type *);
    Type *val_type = va_arg( *arg_list, Type *);

    header_Map( map ) = create_Array( sizeof( MapHeader )
                                    , sizeof( MapEntry )
                                    , CONTAINER_INITIAL_CAPACITY
                                    );

    header_Map( map )->size = 0;
    header_Map( map )->key_type = key_type;
    header_Map( map )->val_type = val_type;
}

static void copy_Map( Map *src, Map *dst )
{
  clone_Array( header_Map( dst ) = header_Map( src ) );
}

static void deep_copy_Map( Map *src, Map *dst )
{
int i;

  /* copy header */
  header_Map( dst ) = create_Array( sizeof( MapHeader ),
                                    sizeof( MapEntry ),
                                    header_Map( src )->capacity
                                  );
  header_Map( dst )->size     = header_Map( src )->size;
  header_Map( dst )->key_type = header_Map( src )->key_type;
  header_Map( dst )->val_type = header_Map( src )->val_type;

  /* copy data */
  for( i = 0; i < header_Map( dst )->size; i++ )
   {MapEntry* dst_entry = ((MapEntry*)(header_Map( dst )+1)) + i;
    MapEntry* src_entry = ((MapEntry*)(header_Map( src )+1)) + i;

    dst_entry->key = r_hard2weak( clone_Object( r(src_entry->key) ) );
    dst_entry->val = r_hard2weak( clone_Object( r(src_entry->val) ) );
   }
}

static int compare_Map( Map *left, Map *right )
{
    int i;
    int size = header_Map( left )->size;

/*     assertion(    header_Map( left  )->key_type  */
/*                == header_Map( right )->key_type */
/*              , FORMAT( "compare_Map: key types are different: '%s' and '%s'" ) */
/*              , header_Map( left  )->key_type->name */
/*              , header_Map( right )->key_type->name */
/*              ); */

/*     assertion(    header_Map( left  )->val_type  */
/*                == header_Map( right )->val_type */
/*              , FORMAT( "compare_Map: value types are different: '%s' and '%s'" ) */
/*              , header_Map( left  )->val_type->name */
/*              , header_Map( right )->val_type->name */
/*              ); */

    if(  size != header_Map( right )->size )
    {
        return 1;
    }

    for( i = 0; i < size; i++ )
    {
        MapEntry *entry = entry_Map( left, i );

        if(    !containsKey_Map( r( right ), r( entry->key ) )
            || compare( get_Map( r( right ), r( entry->key ) )
                      , r( entry->val )
                      )
          )
        {
            return 1;
        }
    }

    return 0;
}

static String *to_string_Map( Map *map )
{
StringBuffer *res;
int i;
int size = header_Map( map )->size;

    if (size == 0) { /* empty map */
      return create_String( "[ ]" );
    }

    res = create_StringBuffer();
	append_StringBuffer( r(res), "[ { " );

    for( i = 0; i < size; i++ ) {
      MapEntry* entry = entry_Map( map, i );

      if( i > 0 ) {
        append_StringBuffer( r(res), " }, { " );
      }
	  appendString_StringBuffer( r(res), toString(r(entry->key)) );
      append_StringBuffer( r(res), " => " );
	  appendString_StringBuffer( r(res), toString(r(entry->val)) );
     }

    append_StringBuffer( r(res), " } ]" );
    return toString(res);
}

static String *to_XML_Map( Map *map )
{
StringBuffer *res;
int i;
int size = header_Map( map )->size;

    if (size == 0) { /* empty map */
		return create_String( "<object kind=\"spec\" type=\"Map\" text=\"Map\"/>" );
    }

	res = create_StringBuffer();
    appendString_StringBuffer( r(res),
		format_String("<object kind=\"spec\" type=\"Map\" text=\"Map [%d]\">\n", header_Map(map)->size) );

    for( i = 0; i < size; i++ )
    {
		MapEntry* entry = entry_Map( map, i );
		append_StringBuffer( r(res), "<object kind=\"spec\" type=\"Map.Entry\" text=\"entry\">\n" );

		append_StringBuffer( r(res), "<label text=\"key\"/>" );
	    appendString_StringBuffer( r(res), toXML(r(entry->key)) );

		append_StringBuffer( r(res), "\n<label text=\"value\"/>" );
	    appendString_StringBuffer( r(res), toXML(r(entry->val)) );

		append_StringBuffer( r(res), "\n</object>\n" );
	}

	append_StringBuffer( r(res), "</object>\n" );

    return toString(res);
}


static bool check_invariant_Map(Map* map)
{
int i;
int size = header_Map( map )->size;
MapEntry* entry;

  for(i=0; i<size; i++)
   {entry = entry_Map(map,i);
    if (!checkInvariant( r(entry->key) ))
     {destroy(map);
      return false;
     }
    if (!checkInvariant( r(entry->val) ))
     {destroy(map);
      return false;
     }
   }
  destroy(map);
  return true;
}

static void enumerate_Map(Map* map,EnumerationCallbackFuncType callback, void* par)
{
int i;
int size = header_Map( map )->size;
MapEntry* entry;

  for( i = 0; i < size; i++ )
   {entry = ((MapEntry*)(header_Map( map )+1)) + i;
    if ( !callback( entry->key, par ) )
      return;
    if ( !callback( entry->val, par ) )
      return;
   }
}

static void destroy_Map( Map *map )
{
  if (header_Map( map )->ref_cnt == 1)
   {
    int i;
    int size = header_Map( map )->size;
    for( i = 0; i < size; i++ )
     {/* destroy all internal references */
      MapEntry* entry = entry_Map( map, i );
      detach_Object( entry->key );
      entry->key = NULL;
      detach_Object( entry->val );
      entry->val = NULL;
     }
   }
  destroy_Array( header_Map( map ) );
}

const Type type_Map
    = { "Map"
      , sizeof(Map)
      , &type_Object
      , (Init)init_Map
      , (Copy)deep_copy_Map
      , (Compare)compare_Map
      , (ToString)to_string_Map
      , (ToXML)to_XML_Map
      , (CheckInvariant)check_invariant_Map
      , (Enumerate)enumerate_Map
      , (Destroy)destroy_Map
      };

Map *create_Map( const Type *key_type, const Type *val_type )
{
    return create( &type_Map, key_type, val_type );
}

static void make_unique_array_Map( Map *map )
{
    MapHeader *new_header;
    int i;

    if( header_Map( map )->ref_cnt == 1 ) return;

    new_header = create_Array( sizeof( MapHeader )
                             , sizeof( MapEntry )
                             , header_Map( map )->capacity
                             );

    new_header->size = header_Map( map)->size;
    new_header->key_type = header_Map( map )->key_type;
    new_header->val_type = header_Map( map )->val_type;

    for( i = 0; i < new_header->size; i++ )
    {
        MapEntry *dst_entry = ((MapEntry*)(new_header+1)) + i;
        MapEntry *src_entry = entry_Map( map, i );
        dst_entry->key = r_weak( src_entry->key );
        dst_entry->val = r_weak( src_entry->val );
    }

    header_Map( map )->ref_cnt--;
    header_Map( map ) = new_header;
}

static void expand_Map( Map *map )
{
    if( header_Map( map )->size < header_Map( map )->capacity ) return;

    make_unique_array_Map( map );

    header_Map( map ) = expand_Array( header_Map( map )
                                    , sizeof( MapHeader )
                                    , sizeof( MapEntry )
                                    );
}

void clear_Map( Map *self )
{
    int i = 0;
    int size;

    CHECK_TYPE_COMPATIBLE( clear_Map, &type_Map, self );

    size = header_Map( self )->size;

    if( header_Map( self )->ref_cnt > 1 )
    {
        Type *key_type = header_Map( self )->key_type;
        Type *val_type = header_Map( self )->val_type;

        header_Map( self )->ref_cnt--;
        header_Map( self ) = create_Array( sizeof( MapHeader )
                                         , sizeof( MapEntry )
                                         , CONTAINER_INITIAL_CAPACITY
                                         );

        header_Map( self )->size = 0;

        header_Map( self )->key_type = key_type;
        header_Map( self )->val_type = val_type;
    } else {
        for( i = 0; i < size; i++ )
        {
            MapEntry *entry = entry_Map( self, i );
            detach_Object( entry->key );
            detach_Object( entry->val );
        }
        header_Map( self )->size = 0;
    }

    destroy( self );
}

bool containsKey_Map( Map *self, Object *key )
{
    int i;
    int size;
    bool res = false;

    CHECK_TYPE_COMPATIBLE( containsKey_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( containsKey_Map, header_Map( self )->key_type, key );

    size = header_Map( self )->size;

    for( i = 0; i < size; i++ )
    {
        if( !compare( r( entry_Map( self, i )->key ), r( key ) ) )
        {
            res = true;
            break;
        }
    }

    destroy( self );
    destroy( key );

    return res;
}

bool containsValue_Map( Map *self, Object *value )
{
    int i;
    int size;
    bool res = false;

    CHECK_TYPE_COMPATIBLE( containsValue_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( containsValue_Map, header_Map( self )->val_type, value );

    size = header_Map( self )->size;

    for( i = 0; i < size; i++ )
    {
        if( !compare( r( entry_Map( self, i )->val ), r( value ) ) )
        {
            res = true;
            break;
        }
    }

    destroy( self );
    destroy( value );

    return res;
}

Object *key_Map( Map *self, int index )
{
    MapEntry * entry;
    Object *res;

    CHECK_TYPE_COMPATIBLE( key_Map, &type_Map, self );

    assertion( 0 <= index && index < header_Map( self )->size
             , FORMAT( "key_Map: map index %d is out of bounds [0, %d[" )
             , index
             , header_Map( self )->size
             );

    entry = entry_Map( self, index );

    res = r( entry->key );

    destroy( self );

    return res;
}

Object *get_Map( Map *self, Object *key )
{
    int i;
    int size;
    Object *res = 0;

    CHECK_TYPE_COMPATIBLE( get_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( get_Map, header_Map( self )->key_type, key );

    size = header_Map( self )->size;

    for( i = 0; i < size; i++ )
    {
        MapEntry *entry = entry_Map( self, i );
        if( !compare( r( entry->key ), r( key ) ) )
        {
            res = r( entry->val );
			break;
        }
    }

    destroy( self );
    destroy( key );

    return res;
}

Object *getKey_Map( Map *self, Object *value )
{
    int i;
    int size;
    Object *res = 0;

    CHECK_TYPE_COMPATIBLE( get_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( get_Map, header_Map( self )->val_type, value );

    size = header_Map( self )->size;

    for( i = 0; i < size; i++ )
    {
        MapEntry *entry = entry_Map( self, i );
        if( !compare( r( entry->val ), r( value ) ) )
        {
            res = r( entry->key );
			break;
        }
    }

    destroy( self );
    destroy( value );

    return res;
}

bool isEmpty_Map( Map *self )
{
bool res;

  CHECK_TYPE_COMPATIBLE( isEmpty_Map, &type_Map, self );

  res = ( header_Map( self )->size == 0 ) ? true : false;

  destroy( self );

  return res;
}

Object* put_Map( Map* self, Object* key, Object* value )
{
int i;
int size = header_Map( self )->size;
MapEntry *entry;

    CHECK_TYPE_COMPATIBLE( put_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( put_Map, header_Map( self )->key_type, key );
    CHECK_TYPE_COMPATIBLE( put_Map, header_Map( self )->val_type, value );

    make_unique_array_Map( self );

    for( i = 0; i < size; i++ )
    {
        entry = entry_Map( self, i );

        if( !compare( r( key ), r( entry->key ) ) )
            break;
    }

    if( i == size )
    {	/* new entry: hold weak reference to key */
        expand_Map( self );
        header_Map( self )->size++;
        entry = entry_Map( self, i );
        entry->key = r_hard2weak( key );
        entry->val = 0;
    }
	else
	{	/* Old entry: key is no longer needed */
		destroy( key );
	}

    {
      /* We already know the entry, no need to recalculate it twice */
      /* MapEntry *entry = entry_Map( self, i );*/
        Object* res = r_weak2hard(entry->val);

        entry->val = r_hard2weak( value );

        destroy( self );

        return res;
    }
}

void putAll_Map( Map *self, Map *t )
{
int i;
int size;

  CHECK_TYPE_COMPATIBLE( putAll_Map, &type_Map, self );
  CHECK_TYPE_COMPATIBLE( putAll_Map, &type_Map, t );

  make_unique_array_Map( self );

  size = header_Map( t )->size;

  for( i = 0; i < size; i++ )
   {
    MapEntry *entry = entry_Map( t, i );
    destroy( put_Map( r( self ), r( entry->key ), r( entry->val ) ) );
   }

  destroy( self );
  destroy( t );
}

Object* remove_Map( Map *self, Object *key )
{
    int i;
    int size;
    Object *res = 0;

    CHECK_TYPE_COMPATIBLE( remove_Map, &type_Map, self );
    CHECK_TYPE_COMPATIBLE( remove_Map, header_Map( self )->key_type, key );

    size = header_Map( self )->size;
    make_unique_array_Map( self );

    for( i = 0; i < size; i++ )
    {
        MapEntry *entry = entry_Map( self, i );

        if( !compare( r( key ), r( entry->key ) ) )
        {
            detach_Object( entry->key );
            res = r_weak2hard(entry->val);

            memmove( entry
                   , entry + 1
                   , ( header_Map( self )->size - i - 1 ) * sizeof(MapEntry)
                   );

            header_Map( self )->size--;
            break;
        }
    }

    destroy( self );
    destroy( key );

    return res;
}

int size_Map( Map *self )
{
    int res = header_Map( self )->size;

    CHECK_TYPE_COMPATIBLE( size_Map, &type_Map, self );

    destroy( self );
    return res;
}

Type *keyType_Map (Map *self)
{
    Type *t = header_Map(self)->key_type;

    CHECK_TYPE_COMPATIBLE( keyType_Map, &type_Map, self );

    destroy(self);
    return t;
}

Type *valueType_Map (Map *self)
{
    Type *t = header_Map(self)->val_type;

    CHECK_TYPE_COMPATIBLE( valueType_Map, &type_Map, self );

    destroy(self);
    return t;
}
