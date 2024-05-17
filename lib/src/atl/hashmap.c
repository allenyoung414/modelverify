/*
 * Copyright 2002-2010 ISP RAS (http://www.ispras.ru), UniTESK Lab (http://www.unitesk.com)
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
#include <string.h>

#include <atl/array.h>
#include <atl/list.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <atl/hashmap.h>


/* Auto-resizing parameters */
#define THRESHOLD(map)	map->header->capacity
#define MAX_CAPACITY	( ((~0U >> 2) + 1) / sizeof(HashMapEntry *) )	/* == 01000....000 bytes */


/* Implementation details */

#define TABLE(map)					((HashMapEntry**)(map->header + 1))
#define INDEX(hash,map)				( (hash & (~0U >> 1)) % map->header->capacity )
#define INDEXTABLE(hash,capacity)	( (hash & (~0U >> 1)) % capacity )

typedef struct HashMapEntry
{
	int hash;
    Object *key;
    Object *val;
	struct HashMapEntry *next;
} HashMapEntry;

typedef struct HashMapHeader
{
    int ref_cnt;
    int capacity;
    int max_capacity;
} HashMapHeader;

struct HashMap {
    HashMapHeader *header;
    Type *key_type;
    Type *val_type;
	HashFunc key_hash_func;
	int size;
};

struct _HashMapIterator {
	HashMapEntry **table;
	int table_size;
	HashMapEntry *current;
};


#ifdef DEBUG_HASHMAP
#include <atl/object_int.h>
void LOG_HASHMAP_DEBUG( char *func, HashMap *self )
{
Header *hdr;

	if (self == NULL) {
		printf( "%s: map == NULL\n", func ); fflush(stdout); exit(1);
	}

	hdr = header(self);

	if ( hdr->type == NULL || hdr->type->name == NULL ) {
		printf( "%s: map = %p, TYPE IS INVALID!\n", func, self ); fflush(stdout); exit(1);
	}

	printf( "%s: map = %p, type = %s, hrc = %i, wrc = %i\n",
		func, self, hdr->type->name,
		hdr->hard_ref_cnt, hdr->weak_ref_cnt );
}
#else
#define LOG_HASHMAP_DEBUG(func,self)
#endif


static HashMapEntry *next_entry( HashMapIterator iter );
static void init_iterator( struct _HashMapIterator *iter, HashMap *map );


static HashMapEntry *entry_for( HashMap *self, Object *key )
{
int hash = self->key_hash_func( r(key) );
int index = INDEX(hash,self);
HashMapEntry *current = TABLE(self) [index];

	while( current != NULL ) {
		if ( current->hash == hash && !compare( r(current->key), r(key) ) )
            return current;
		current = current->next;
    }

	return NULL;
}

static void init_HashMap( HashMap *map, va_list *arg_list )
{
Type *key_type = va_arg( *arg_list, Type *);
Type *val_type = va_arg( *arg_list, Type *);
int capacity = va_arg( *arg_list, int );
int maxCapacity = va_arg( *arg_list, int );
HashFunc key_hash_func = va_arg( *arg_list, HashFunc );

	LOG_HASHMAP_DEBUG( "init_HashMap", map );

    map->header = create_Array( sizeof(HashMapHeader)
                              , sizeof(HashMapEntry*)
                              , capacity
                              );
	map->header->max_capacity = maxCapacity;
	memset( TABLE(map), 0, capacity * sizeof(HashMapEntry*) );

    map->key_type = key_type;
    map->val_type = val_type;
	map->key_hash_func = key_hash_func;
	map->size = 0;
}

static void copy_HashMap( HashMap *src, HashMap *dst, bool deep )
{
int size = src->header->capacity;
struct _HashMapIterator iter;

	LOG_HASHMAP_DEBUG( "copy_HashMap: src", src );
	LOG_HASHMAP_DEBUG( "copy_HashMap: dst", dst );

	/* copy header */
	dst->header = create_Array( sizeof(HashMapHeader)
                              , sizeof(HashMapEntry*)
                              , size
                              );
	memset( TABLE(dst), 0, size * sizeof(HashMapEntry*) );

    dst->key_type = src->key_type;
    dst->val_type = src->val_type;
	dst->key_hash_func = src->key_hash_func;
	dst->size = 0;

	/* copy data */
	init_iterator( &iter, src );
	while ( hasNext_HashMapIterator( &iter )) {
		HashMapEntry *src_entry = next_entry( &iter );
		Object *key = r(src_entry->key);
		Object *val = r(src_entry->val);
		if (deep) {
			key = clone_Object(key);
			val = clone_Object(val);
		}
		put_HashMap( r(dst), key, val );
	}
}

static void shallow_copy_HashMap( HashMap *src, HashMap *dst )
{
	copy_HashMap( src, dst, false );
}

static void deep_copy_HashMap( HashMap *src, HashMap *dst )
{
	copy_HashMap( src, dst, true );
}

static int compare_HashMap( HashMap *left, HashMap *right )
{
int res;
struct _HashMapIterator iter;

	LOG_HASHMAP_DEBUG( "compare_HashMap: left", left );
	LOG_HASHMAP_DEBUG( "compare_HashMap: right", right );

	assertion( left->key_type == right->key_type
			, FORMAT( "compare_HashMap: key types are different: '%s' and '%s'" )
			, left ->key_type->name
			, right->key_type->name
			);
	assertion( left->val_type == right->val_type
			, FORMAT( "compare_HashMap: value types are different: '%s' and '%s'" )
			, left ->val_type->name
			, right->val_type->name
			);

	res = left->size - right->size;
	if (res != 0)	return res;

	init_iterator( &iter, left );
	while ( res == 0 && hasNext_HashMapIterator( &iter )) {
		HashMapEntry *entry = next_entry( &iter );
		if ( ! containsKey_HashMap( r(right), r(entry->key) ) )
			res = 1;

		if (res == 0)
			res = compare( r(entry->val), get_HashMap(r(right), r(entry->key)) );
	}

	return res;
}

static String *to_string_HashMap( HashMap *map )
{
StringBuffer *res;
struct _HashMapIterator iter;
bool isFirst;

	LOG_HASHMAP_DEBUG( "to_string_HashMap", map );

	if ( map->size == 0) { /* empty map */
		return create_String( "[ ]" );
	}

	res = create_StringBuffer();
	append_StringBuffer( r(res), "[ { " );

	init_iterator( &iter, map );
	isFirst = true;
	while ( hasNext_HashMapIterator( &iter )) {
		HashMapEntry *entry = next_entry( &iter );
		if (!isFirst) {
			append_StringBuffer( r(res), " }, { " );
		} else {
			isFirst = false;
		}
		appendString_StringBuffer( r(res), toString(r(entry->key)) );
		append_StringBuffer( r(res), " => " );
		appendString_StringBuffer( r(res), toString(r(entry->val)) );
	}

    append_StringBuffer( r(res), " } ]" );
    return toString(res);
}

static String *to_XML_HashMap( HashMap *map )
{
StringBuffer *res;
struct _HashMapIterator iter;

	LOG_HASHMAP_DEBUG( "to_XML_HashMap", map );

	if ( map->size == 0) { /* empty map */
		return create_String( "<object kind=\"spec\" type=\"Map\" text=\"HashMap\"/>" );
	}

	res = create_StringBuffer();
	appendFormat_StringBuffer( r(res),
		"<object kind=\"spec\" type=\"Map\" text=\"HashMap [%d]\">\n", map->size );

	init_iterator( &iter, map );
	while ( hasNext_HashMapIterator( &iter )) {
		HashMapEntry *entry = next_entry( &iter );

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

static void do_clear_HashMap( HashMap *map )
{
int i;
HashMapEntry **data = TABLE(map);
int table_size = map->header->capacity;

    for( i = 0; i < table_size; i++ ) {
		while( data[i] != NULL ) {
			HashMapEntry *coffin = data[i];
			data[i] = coffin->next;
			detach_Object( coffin->key );
			detach_Object( coffin->val );
			free(coffin);
		}
    }

	map->size = 0;
}

static void enumerate_HashMap( HashMap *map, EnumerationCallbackFuncType callback, void* par )
{
struct _HashMapIterator iter;

	init_iterator( &iter, map );
	while( hasNext_HashMapIterator( &iter )) {
		HashMapEntry *entry = next_entry( &iter );
		if ( !callback( entry->key, par ) || !callback( entry->val, par ) )
			break;
	}
}

static void destroy_HashMap( HashMap *map )
{
	LOG_HASHMAP_DEBUG( "destroy_HashMap", map );

/*  if (self->header->ref_cnt == 1)  */
	  do_clear_HashMap( map );

	destroy_Array( map->header );
}

const Type type_HashMap
    = { "HashMap"
      , sizeof(HashMap)
      , &type_Object
      , (Init)init_HashMap
      , (Copy)deep_copy_HashMap /*shallow_copy_HashMap*/
      , (Compare)compare_HashMap
      , (ToString)to_string_HashMap
      , (ToXML)to_XML_HashMap
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_HashMap
      , (Destroy)destroy_HashMap
      };


/****************** Public interface ******************/

HashMap *create_HashMap( const Type* key_type, const Type* val_type,
						int initialCapacity, HashFunc key_hash_func )
{
HashMap *res;

	if (initialCapacity < 2)
		initialCapacity = 2;
	else if (initialCapacity > MAX_CAPACITY)
		initialCapacity = MAX_CAPACITY;

	res = create( &type_HashMap, key_type, val_type, initialCapacity, MAX_CAPACITY, key_hash_func );

	LOG_HASHMAP_DEBUG( "create_HashMap", res );
	return res;
}

HashMap *createLimited_HashMap( const Type* key_type, const Type* val_type,
						int initialCapacity, int maxCapacity, HashFunc key_hash_func )
{
HashMap *res;

	if (initialCapacity < 2)
		initialCapacity = 2;
	else if (initialCapacity > MAX_CAPACITY)
		initialCapacity = MAX_CAPACITY;

	if (maxCapacity < initialCapacity)
		maxCapacity = initialCapacity;
	else if (maxCapacity > MAX_CAPACITY)
		maxCapacity = MAX_CAPACITY;

	res = create( &type_HashMap, key_type, val_type, initialCapacity, maxCapacity, key_hash_func );

	LOG_HASHMAP_DEBUG( "createLimited_HashMap", res );
	return res;
}

void clear_HashMap( HashMap *self )
{
	LOG_HASHMAP_DEBUG( "clear_HashMap", self );

	do_clear_HashMap( self );
	destroy(self);
}

bool containsKey_HashMap( HashMap *self, Object *key )
{
HashMapEntry *entry;

	LOG_HASHMAP_DEBUG( "containsKey_HashMap", self );

    CHECK_TYPE_COMPATIBLE( containsKey_HashMap, &type_HashMap, self );
    CHECK_TYPE_COMPATIBLE( containsKey_HashMap, self->key_type, key );

	entry = entry_for( self, key );

    destroy( self );
    destroy( key );
    return ( entry != NULL );
}

Object *get_HashMap( HashMap *self, Object *key )
{
HashMapEntry *entry;

	LOG_HASHMAP_DEBUG( "get_HashMap", self );

    CHECK_TYPE_COMPATIBLE( get_HashMap, &type_HashMap, self );
    CHECK_TYPE_COMPATIBLE( get_HashMap, self->key_type, key );

	entry = entry_for( self, key );

    destroy( self );
    destroy( key );
    return ( entry == NULL ? NULL : r(entry->val) );
}

bool isEmpty_HashMap( HashMap *self )
{
bool res;

	LOG_HASHMAP_DEBUG( "isEmpty_HashMap", self );
	CHECK_TYPE_COMPATIBLE( isEmpty_HashMap, &type_HashMap, self );

	res = self->size == 0 ? true : false;

	destroy( self );
	return res;
}

static void resize( HashMap* map, int newCapacity )
{
HashMapHeader *oldHeader = map->header;
HashMapEntry **oldTable = (HashMapEntry**)(oldHeader + 1), **newTable;
int i;

	LOG_HASHMAP_DEBUG( "resize", map );

	map->header = create_Array( sizeof(HashMapHeader), sizeof(HashMapEntry*), newCapacity);
	map->header->max_capacity = oldHeader->max_capacity;
	newTable = TABLE(map);
	memset( newTable, 0, newCapacity * sizeof(HashMapEntry*) );
	
	for (i = 0; i < oldHeader->capacity; i++) {
		while( oldTable[i] != NULL ) {
			HashMapEntry *entry = oldTable[i];
			int newIndex = INDEXTABLE( entry->hash, newCapacity );
			oldTable[i] = entry->next;
			entry->next = newTable[ newIndex ];
			newTable[ newIndex ] = entry;
		}
	}

	destroy_Array( oldHeader );
}

Object* put_HashMap( HashMap* self, Object* key, Object* value )
{
int hash, index;
HashMapEntry *current;

	LOG_HASHMAP_DEBUG( "put_HashMap", self );

    CHECK_TYPE_COMPATIBLE( put_HashMap, &type_HashMap, self );
    CHECK_TYPE_COMPATIBLE( put_HashMap, self->key_type, key );
	if (value != NULL) {
		CHECK_TYPE_COMPATIBLE( put_HashMap, self->val_type, value );
	}

	hash = self->key_hash_func( r(key) );
	index = INDEX(hash,self);

	current = TABLE(self) [index];

/*	printf("*** put_HashMap: %s ***\n", current==NULL ? "empty bucket" : "occupied bucket" ); */

	while( current != NULL ) {
		if ( current->hash == hash && !compare( r(current->key), r(key) ) ) {
	        Object* res = r_weak2hard( current->val );
	        current->val = r_hard2weak( value );
			
			destroy(key);
			destroy(self);
            return res;
        }
		current = current->next;
    }

	if (self->size >= THRESHOLD(self) && self->header->capacity < self->header->max_capacity ) {
		int newCapacity = self->header->capacity * 2;
		if (newCapacity > self->header->max_capacity)
			newCapacity = self->header->max_capacity;

		resize( self, newCapacity );
		index = INDEX(hash,self);	/* Index can change after changing capacity */
	}

	current = malloc( sizeof(HashMapEntry) );
	assertion( current != NULL, "put_HashMap(): out of memory" );

	current->hash = hash;
	current->key = r_hard2weak( key );
	current->val = r_hard2weak( value );

	/* Add new element to the beginning, because that is easier. */
	current->next = TABLE(self) [index];
	TABLE(self) [index] = current;

	self->size++;

	destroy(self);
	return NULL;
}

void silentPut_HashMap( HashMap* self, Object* key, Object* value )
{
	destroy( put_HashMap(self, key, value) );
}

Object* remove_HashMap( HashMap *self, Object *key )
{
int hash, index;
HashMapEntry **pcurrent;

	LOG_HASHMAP_DEBUG( "remove_HashMap", self );

    CHECK_TYPE_COMPATIBLE( remove_HashMap, &type_HashMap, self );
    CHECK_TYPE_COMPATIBLE( remove_HashMap, self->key_type, key );

	hash = self->key_hash_func( r(key) );
	index = INDEX(hash,self);
	pcurrent = TABLE(self) + index;

	while( *pcurrent != NULL ) {
		if ( (*pcurrent)->hash == hash && !compare( r((*pcurrent)->key), r(key) ) ) {
	        Object *res;
			HashMapEntry *coffin = *pcurrent;

			*pcurrent = coffin->next;

			detach_Object( coffin->key );
			res = r_weak2hard( coffin->val );
			free(coffin);
			self->size--;

			destroy( self );
			destroy( key );
            return res;
        }
		pcurrent = & (*pcurrent)->next;
    }

    destroy( self );
    destroy( key );
    return NULL;
}

void silentRemove_HashMap( HashMap *self, Object *key )
{
	destroy( remove_HashMap(self, key) );
}

int size_HashMap( HashMap *self )
{
int res;

	LOG_HASHMAP_DEBUG( "size_HashMap", self );
	CHECK_TYPE_COMPATIBLE( size_HashMap, &type_HashMap, self );

	res = self->size;

	destroy( self );
	return res;
}


/************************* Iteration *************************/

static HashMapEntry *find_entry( HashMapEntry **table, int table_size, int start_index )
{
int i;
	for (i = start_index; i < table_size; i++) {
		if (table[i] != NULL)
			return table[i];
	}
	return NULL;
}

static HashMapEntry *get_next_entry( HashMapEntry **table, int table_size, HashMapEntry *current )
{
	if (current == NULL)	return NULL;

	if (current->next != NULL)
		return current->next;

	return find_entry( table, table_size, INDEXTABLE(current->hash, table_size) + 1 );
}

static void init_iterator( struct _HashMapIterator *iter, HashMap *map )
{
	iter->table = TABLE(map);
	iter->table_size = map->header->capacity;
	iter->current = find_entry( iter->table, iter->table_size, 0 );
}

HashMapIterator iterator_HashMap( HashMap *self )
{
HashMapIterator res;

	LOG_HASHMAP_DEBUG( "iterator_HashMap", self );

    CHECK_TYPE_COMPATIBLE( iterator_HashMap, &type_HashMap, self );
	/*
    assertion( header( self )->hard_ref_cnt + header( self )->weak_ref_cnt > 1,
               "iterator_HashMap: self has no other references, iterator will be invalid"
             );
	*/

	res = malloc( sizeof(struct _HashMapIterator) );
	assertion( res != NULL, "iterator_HashMap(): out of memory" );
	init_iterator( res, self );

	destroy(self);
	return res;
}

bool hasNext_HashMapIterator( HashMapIterator iter )
{
	return iter->current != NULL;
}

static HashMapEntry *next_entry( HashMapIterator iter )
{
HashMapEntry *res;

	if (iter->current == NULL)	return NULL;

	res = iter->current;
	iter->current = get_next_entry( iter->table, iter->table_size, iter->current );

	return res;
}

Object *nextKey_HashMapIterator( HashMapIterator iter )
{
	HashMapEntry *entry = next_entry( iter );
	return entry==NULL ? NULL : r(entry->key);
}

void free_HashMapIterator( HashMapIterator iter )
{
	free(iter);
}
