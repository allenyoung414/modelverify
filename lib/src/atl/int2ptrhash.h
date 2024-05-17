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

#ifndef __ATL_INT2PTRHASH_H__
#define __ATL_INT2PTRHASH_H__

#include <utils/boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                     Hash Map: int to Pointer                   **/
/********************************************************************/

typedef struct Int2PtrHashMapElement Int2PtrHashMapElement;
struct Int2PtrHashMapElement
{
  int   key;
  void* value;
  Int2PtrHashMapElement* next;
};

typedef struct Int2PtrHashMap Int2PtrHashMap;
struct Int2PtrHashMap
{
  int is_not_plain;
  int size;
  Int2PtrHashMapElement* data;
};

void init_Int2PtrHashMap( Int2PtrHashMap* map, int size, int min_size );

void reinit_Int2PtrHashMap( Int2PtrHashMap* map );

void destroy_Int2PtrHashMap( Int2PtrHashMap* map );

void* get_Int2PtrHashMap( Int2PtrHashMap* map, int key );

/*
 * put_Int2PtrHashMap puts a new pair to the map if the key is a new one
 * but does not override existing pair.
 */
void* put_Int2PtrHashMap( Int2PtrHashMap* map, int key, void* value );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
