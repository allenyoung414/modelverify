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
#ifndef __ATL_HASHMAP_H__
#define __ATL_HASHMAP_H__

#include <atl/object.h>


typedef int (*HashFunc)(Object *obj);


#ifdef __SEC__
  #pragma SEC file
  specification typedef struct HashMap HashMap;
#else
  typedef struct HashMap HashMap;
  extern const Type type_HashMap;
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Create empty hash map for given types. The map will resize automatically
 * increasing the number of its buckets when it becomes full.
 */
HashMap *create_HashMap( const Type* key_type, const Type* val_type,
						int initialCapacity, HashFunc key_hash_func );

/*
 * Create empty hash map for given types with given maximal number
 * of buckets (the number of contained elements is not limited).
 */
HashMap *createLimited_HashMap( const Type* key_type, const Type* val_type,
						int initialCapacity, int maxCapacity, HashFunc key_hash_func );

/*
 * Removes all mappings from this map.
 */
void clear_HashMap( HashMap *self );

/*
 * Returns true if this map contains a mapping for the specified key.
 */
bool containsKey_HashMap( HashMap *self, Object *key );

/*
 * Returns the value to which this map maps the specified key.
 */
Object *get_HashMap( HashMap *self, Object *key );
          
/*
 * Returns true if this map contains no key-value mappings.
 */
bool isEmpty_HashMap( HashMap *self );
 
/*
 * Associates the specified value with the specified key in this map.
 * Supports NULL values, but not NULL keys.
 * @return Value previously associated with that key.
 * DO NOT FORGET TO DESTROY IT!!!
 */
Object *put_HashMap( HashMap *self, Object *key, Object *value );

/*
 * The same as put_HashMap(), but silently destroy()'s a previous value
 * assotiated with the key.
 */
void silentPut_HashMap( HashMap *self, Object *key, Object *value );

/*
 * Removes the mapping for this key from this map if present.
 * @return Value previously associated with that key (may be NULL)
 * or NULL if there was none.
 * DO NOT FORGET TO DESTROY IT!!!
 */
Object *remove_HashMap( HashMap *self, Object *key );

/*
 * The same as remove_HashMap(), but silently destroy()'s a previous value
 * assotiated with the key.
 */
void silentRemove_HashMap( HashMap *self, Object *key );

/*
 * Returns the number of key-value mappings in this map.
 */
int size_HashMap( HashMap *self );


/* Iteration.
Normal usage (skip calls to r() in SEC code):
	HashMapIterator iter = iterator_HashMap( r(my_HashMap) );
	while ( hasNext_HashMapIterator(iter) ) {
		Object *key = nextKey_HashMapIterator(iter);
		Object *value = get_HashMap( r(my_HashMap), r(key) );
		... use key and value here ...
		destroy(key);
		destroy(value);
	}
	free_HashMapIterator(iter);

Notes:
 1. Iterator is not Object, so it should never be r()'ed or destroy()'ed.
 2. Iteration order is not specified and may sometimes be confusing.
 3. If a HashMap is modified during an iteration over it, the results are unpredictable.
 4. Do not forget to call free_HashMapIterator().
 */

typedef struct _HashMapIterator *HashMapIterator; /* Not Object! */

HashMapIterator iterator_HashMap( HashMap *self /* Needs r() */ );

bool hasNext_HashMapIterator( HashMapIterator iter /* No r() */ );

Object* /*result needs destroy()*/ nextKey_HashMapIterator( HashMapIterator iter /* No r() */ );

void free_HashMapIterator( HashMapIterator iter );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __HASH_MAP_H__ */
