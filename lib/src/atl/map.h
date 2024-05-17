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
 * File: map.h
 * Description:
 *       This file declares the specification type representing
 *       an abstract map container.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_MAP_H__
#define __ATL_MAP_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __SEC__
  #pragma SEC file
  specification typedef struct Map Map;
#else
  typedef struct Map Map;
  extern const Type type_Map;
#endif


/*
 * Create empty map of specified type.
 */
Map *create_Map( const Type* key_type, const Type* val_type );

/*
 * Removes all mappings from this map.
 */
void clear_Map( Map *self );

/*
 * Returns true if this map contains a mapping for the specified key.
 */
bool containsKey_Map( Map *self, Object *key );

/*
 * Returns true if this map maps one or more keys to the specified value.
 */
bool containsValue_Map( Map *self, Object *value );

/*
 * Returns the value to which this map maps the specified key.
 */
Object *get_Map( Map *self, Object *key );
          
/*
 * Returns the key that this map maps to the specified value.
 */
Object *getKey_Map( Map *self, Object *value );
          
/*
 * Returns true if this map contains no key-value mappings.
 */
bool isEmpty_Map( Map *self );
 
/*
 * Associates the specified value with the specified key in this map.
 */
Object *put_Map( Map *self, Object *key, Object *value );

/*
 * Copies all of the mappings from the specified map to this map.
 */
void putAll_Map( Map *self, Map *t );

/*
 * Removes the mapping for this key from this map if present.
 */
Object *remove_Map( Map *self, Object *key );

/*
 * Returns the number of key-value mappings in this map.
 */
int size_Map( Map *self );

/*
 * Returns key for entry at specified position in the map.
 */
Object *key_Map( Map *self, int index );


/*
 * Returns the type desc of keys
 */
Type *keyType_Map (Map *self);


/*
 * Returns the type desc of values
 */
Type *valueType_Map (Map *self);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
