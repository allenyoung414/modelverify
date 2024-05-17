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

#include <atl/int2ptrhash.h>
#include <utils/assertion.h>

///////////////////////////////////////////////////////////////////////
///                     Hash Map: int to Pointer                     //
///////////////////////////////////////////////////////////////////////

void init_Int2PtrHashMap( Int2PtrHashMap* map, int size, int min_size)
{
  assertion( (map != NULL), "init_Int2PtrHashMap: map is NULL" );

  map->is_not_plain = 0;

  // Memory optimization: if no memory block of the requested size is available,
  // the test system tries to reduce size of the hash map
  do
   {map->data = (Int2PtrHashMapElement*)calloc(size, sizeof(Int2PtrHashMapElement));
    if (map->data != NULL)
      break;
    size /= 2;
   }
  while(size >= min_size);

  map->size = size;

  assertion( map->data != 0, FORMAT( "init_Int2PtrHashMap: No memory" ) );
}

void reinit_Int2PtrHashMap( Int2PtrHashMap* map )
{
Int2PtrHashMapElement *tmp,*coffin;
int i;

  assertion( (map != NULL), "reinit_Int2PtrHashMap: map is NULL" );

  if (map->is_not_plain)
   {for(i=0;i<map->size;i++)
     {for(tmp=map->data[i].next;tmp!=NULL;)
       {coffin = tmp;
        tmp = tmp->next;
        free(coffin);
       }
     }
    map->is_not_plain = 0;
   }
  memset(map->data,0,map->size*sizeof(Int2PtrHashMapElement));
}

void destroy_Int2PtrHashMap( Int2PtrHashMap* map )
{
Int2PtrHashMapElement *tmp,*coffin;
int i;

  assertion( (map != NULL), "destroy_Int2PtrHashMap: map is NULL" );

  if (map->is_not_plain)
   {for(i=0;i<map->size;i++)
     {for(tmp=map->data[i].next;tmp!=NULL;)
       {coffin = tmp;
        tmp = tmp->next;
        free(coffin);
       }
     }
    map->is_not_plain = 0;
   }

   free(map->data);
   map->data = NULL;
}

void* get_Int2PtrHashMap( Int2PtrHashMap* map, int key )
{
Int2PtrHashMapElement* tmp;

  // returns NULL even if (tmp->key == key) is wrongly true
  for(tmp=&(map->data[key%map->size]);tmp!=NULL;tmp=tmp->next)
    if (tmp->key == key)
      return tmp->value;
  return NULL;
}

//
// put_Int2PtrHashMap puts a new pair to the map if the key is a new one
// but does not override existing pair.
//
void* put_Int2PtrHashMap( Int2PtrHashMap* map, int key, void* value )
{
Int2PtrHashMapElement *base,*tmp;

  base = &(map->data[key%map->size]);
  if (base->value == NULL)
   {base->key   = key;
    base->value = value;
    return value;
   }

  for(tmp=base;tmp!=NULL;tmp=tmp->next)
    if (tmp->key == key)
      return tmp->value;

  tmp = (Int2PtrHashMapElement*)malloc(sizeof(Int2PtrHashMapElement));
  assertion( tmp != NULL, "put_Int2PtrHashMap: not enough memory" );

  tmp->key   = key;
  tmp->value = value;
  tmp->next  = base->next;
  base->next = tmp;
  map->is_not_plain = 1;
  return value;
}
