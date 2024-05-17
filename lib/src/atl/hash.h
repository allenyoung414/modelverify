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
 * File: hash.h
 * Description:
 *       This file contains data structures that needs for
 *       processing cycles by references.
 *       It is an internal header file of CTesK Abstract Type Library.
 */

#ifndef __ATL_HASH_H__
#define __ATL_HASH_H__

#include <utils/boolean.h>
#include <utils/assertion.h>

#include <stdlib.h>
#include <string.h>


#define ATL_HASH_SIZE   128

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************/
/**                           Hash Map                             **/
/********************************************************************/
typedef struct void_HashMapElement void_HashMapElement;
struct void_HashMapElement
{
  void* key;
  void* value;
  void_HashMapElement* next;
};

typedef struct void_HashMap void_HashMap;
struct void_HashMap
{
  int is_not_plain;
  void_HashMapElement data[ATL_HASH_SIZE];
};

void  atl_reinit_hash_map( void_HashMap* map );
void* atl_get_hash_map( void_HashMap* map, void* key);
void  atl_put_hash_map( void_HashMap* map, void* key, void* value );


/********************************************************************/
/**                      Two Pointer Hash Set                      **/
/********************************************************************/
typedef struct TwoPointerHashSetElement TwoPointerHashSetElement;
struct TwoPointerHashSetElement
{
  void* ptr1;
  void* ptr2;
  TwoPointerHashSetElement* next;
};

typedef struct TwoPointerHashSet TwoPointerHashSet;
struct TwoPointerHashSet
{
  TwoPointerHashSetElement data[ATL_HASH_SIZE];
};

bool atl_add_two_pointer_hash_set( TwoPointerHashSet* set, void* ptr1, void* ptr2 );
void atl_remove_two_pointer_hash_set(TwoPointerHashSet* set, void* ptr1, void* ptr2 );


/********************************************************************/
/**                           Hash Set                             **/
/********************************************************************/
typedef struct HashSetElement HashSetElement;
struct HashSetElement
{
  void*           ptr;
  HashSetElement* next;
};

typedef struct HashSet HashSet;
struct HashSet
{
  int is_not_plain;
  HashSetElement data[ATL_HASH_SIZE];
};

void atl_reinit_hash_set( HashSet* set );
bool atl_add_hash_set( HashSet* set, void* ptr );
void atl_remove_hash_set( HashSet* set, void* ptr );
bool atl_contains_hash_set( HashSet* set, void* ptr );


/********************************************************************/
/**                     Hash Map: Pointer to int                   **/
/********************************************************************/
typedef struct IntegerHashMapElement IntegerHashMapElement;
struct IntegerHashMapElement
{
  void* key;
  int   value;
  IntegerHashMapElement* next;
};

typedef struct IntegerHashMap IntegerHashMap;
struct IntegerHashMap
{
  int is_not_plain;
  IntegerHashMapElement data[ATL_HASH_SIZE];
};

void atl_reinit_integer_hash_map( IntegerHashMap* map );
int* atl_get_integer_hash_map( IntegerHashMap* map, void* key );
void atl_put_integer_hash_map( IntegerHashMap* map, void* key, int value );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
