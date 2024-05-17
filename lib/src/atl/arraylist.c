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

#include <stdlib.h>
#include <string.h>
#include <atl/arraylist.h>
#include <utils/assertion.h>

/********************************************************************/
/*                      Array List Data Type                        */
/********************************************************************/

void init_ArrayList( ArrayList* array, int element_size, int allocBy, Destructor destructor )
{
  assertion( (array != NULL), "init_ArrayList: array is NULL" );

  array->size = 0;
  array->data = malloc(sizeof(char*));
  assertion( (array->data != NULL), "init_ArrayList: not enough memory" );

  array->data[0] = malloc(element_size*allocBy);
  assertion( (array->data[0] != NULL), "init_ArrayList: not enough memory" );

  array->blocks_number = 1;
  array->element_size = element_size;
  array->allocBy = allocBy;
  array->element_destructor = destructor;
}

void* get_ArrayList( ArrayList* array, int index )
{
  assertion( (array != NULL), "get_ArrayList: array is NULL" );
  assertion( (0 <= index) && (index < array->size), "get_ArrayList: index is out of range [0,%d[", array->size );

  return &(array->data[index/array->allocBy][(index%array->allocBy)*array->element_size]);
}

void* find_ArrayList( ArrayList* array, ArrayListComparator comparator, void* key )
{
char* element;
int i,j;

  assertion( (array != NULL), "find_ArrayList: array is NULL" );
  assertion( (comparator != NULL), "find_ArrayList: comparator is NULL" );

  element = array->data[0];
  j = 0;
  for( i = 0; i < array->size; )
   {
    if (comparator( element, key ))
     {
      return element;
     }
    i++; j++;
    if (j < array->allocBy)
     {
      element = element + array->element_size;
     }
    else
     {
      element = array->data[i/array->allocBy];
      j = 0;
     }
   }
  return NULL;
}

void* append_ArrayList( ArrayList* array )
{
int index;

  assertion( (array != NULL), "append_ArrayList: array is NULL" );

  if (array->size/array->allocBy >= array->blocks_number)
   {
    array->data = realloc( array->data, (array->blocks_number+1)*sizeof(char*) );
    assertion( (array->data != NULL), "append_ArrayList: not enough memory" );

    array->data[array->blocks_number] = malloc(array->element_size*array->allocBy);
    assertion( (array->data[array->blocks_number] != NULL), "append_ArrayList: not enough memory" );

    array->blocks_number++;
   }

  index = array->size++;
  return &(array->data[index/array->allocBy][(index%array->allocBy)*array->element_size]);
}

void remove_ArrayList( ArrayList* array, void* element )
{
int i,index,rest;

  if (array->element_destructor != NULL)
   {
    array->element_destructor( element );
   }
  for( i = 0; i < array->blocks_number; i++ )
   {
    if ((array->data[i] <= (char*)element) && ((char*)element < array->data[i] + array->element_size*array->allocBy))
      break;
   }
  assertion( i < array->blocks_number, "remove_ArrayList: element does not belong to array" );
  index = (((char*)element) - array->data[i])/array->element_size;
  if (i == array->blocks_number - 1)
   {
    rest = (array->size-1)%array->allocBy - index;
    if (rest > 0)
     {
      memmove( element, ((char*)element) + array->element_size, rest*array->element_size );
     }
   }
  else
   {
    rest = array->allocBy - index - 1;
    if (rest > 0)
     {
      memmove( element, ((char*)element) + array->element_size, rest*array->element_size );
     }
    for( i++; i < array->blocks_number - 1; i++ )
     {
      memcpy( array->data[i-1] + (array->allocBy-1)*array->element_size, array->data[i], array->element_size );
      memmove( array->data[i], array->data[i] + array->element_size, (array->allocBy-1)*array->element_size );
     }
    memcpy( array->data[i-1] + (array->allocBy-1)*array->element_size, array->data[i], array->element_size );
    rest = (array->size-1)%array->allocBy;
    memmove( array->data[i], array->data[i] + array->element_size, rest*array->element_size );
   }
  array->size--;

  // Memory economy: deallocate the final block if it is unused
  if(array->size <= array->allocBy*(array->blocks_number - 1))
   {
	array->blocks_number--;
    free(array->data[array->blocks_number]);
	array->data[array->blocks_number] = NULL;
   }
}

void clear_ArrayList( ArrayList* array )
{
int i,j,k;

  assertion( (array != NULL), "clear_ArrayList: array is NULL" );

  if (array->element_destructor != NULL)
   {
    k = 0;
    for( i = 0; i < array->blocks_number; i++ )
     {
      char* element = array->data[i];
      for( j = 0; j < array->allocBy; j++, k++ )
       {
        if (k >= array->size)
          break;
        array->element_destructor( element );
        element = element + array->element_size;
       }
     }
   }

  array->size = 0;
}

void destroy_ArrayList( ArrayList* array )
{
int i,j,k;

  assertion( (array != NULL), "destroy_ArrayList: array is NULL" );

  k = 0;
  for( i = 0; i < array->blocks_number; i++ )
   {
    if (array->element_destructor != NULL)
     {
      char* element = array->data[i];
      for( j = 0; j < array->allocBy; j++, k++ )
       {
        if (k >= array->size)
          break;
        array->element_destructor( element );
        element = element + array->element_size;
       }
     }
    free( array->data[i] );
   }
  free(array->data);
}

