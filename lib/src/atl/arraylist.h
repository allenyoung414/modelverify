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

#ifndef __ATL_ARRAYLIST_H__
#define __ATL_ARRAYLIST_H__

#include <utils/boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Destructor)(void* element);

typedef struct ArrayList
{
  int        size;
  char**     data;
  int        blocks_number;
  int        element_size;
  int        allocBy;
  Destructor element_destructor;
} ArrayList;

typedef bool (*ArrayListComparator)(void* element, void* key);

void init_ArrayList( ArrayList* array, int element_size, int allocBy, Destructor destructor );

void* get_ArrayList( ArrayList* array, int index );

void* find_ArrayList( ArrayList* array, ArrayListComparator comparator, void* key );

void* append_ArrayList( ArrayList* array );

void remove_ArrayList( ArrayList* array, void* element );

void clear_ArrayList( ArrayList* array );

void destroy_ArrayList( ArrayList* array );

#ifdef __cplusplus
}
#endif

#endif
