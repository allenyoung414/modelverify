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
 * File: hash.c
 * Description:
 *       This file implements functions that needs for
 *       processing cycles by references.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/hash.h>


/********************************************************************/
/**                        Hash Functions                          **/
/********************************************************************/
static unsigned atl_pointer_hash_function(void* ptr)
{
unsigned i,res;
  
  res = 0;
  for(i=0;i<sizeof(ptr);i++)
    res ^= ((char*)&ptr)[i];
  return res % ATL_HASH_SIZE;
}

static unsigned atl_two_pointer_hash_function(void* ptr1,void* ptr2)
{
unsigned i,res;
  
  res = 0;
  for(i=0;i<sizeof(ptr1);i++)
    res ^= ((char*)&ptr1)[i];
  for(i=0;i<sizeof(ptr2);i++)
    res ^= ((char*)&ptr2)[i];
  return res % ATL_HASH_SIZE;
}


/********************************************************************/
/**                           Hash Map                             **/
/********************************************************************/
void atl_reinit_hash_map(void_HashMap* map)
{
void_HashMapElement *tmp,*coffin;
int i;
  
  if (map->is_not_plain)
   {
    for(i=0;i<ATL_HASH_SIZE;i++)
     {for(tmp=map->data[i].next;tmp!=NULL;)
       {coffin = tmp;
        tmp = tmp->next;
        free(coffin);
       }
     }
    map->is_not_plain = 0;
   }
  memset(map->data,0,sizeof(map->data));
}

void* atl_get_hash_map(void_HashMap* map,void* key)
{
void_HashMapElement* tmp;
  
  for(tmp=&(map->data[atl_pointer_hash_function(key)]);tmp!=NULL;tmp=tmp->next)
    if (tmp->key == key)
      return tmp->value;
  return NULL;
}

void atl_put_hash_map(void_HashMap* map,void* key,void* value)
{
void_HashMapElement *base,*tmp;
  
  base = &(map->data[atl_pointer_hash_function(key)]);
  if (base->key == NULL)
   {base->key   = key;
    base->value = value;
    return;
   }
  tmp = (void_HashMapElement*)malloc(sizeof(void_HashMapElement));
  assertion( tmp != NULL, "atl_put_hash_map: not enough memory" );
  
  tmp->key   = key;
  tmp->value = value;
  tmp->next  = base->next;
  base->next = tmp;
  map->is_not_plain = 1;
}


/********************************************************************/
/**                      Two Pointer Hash Set                      **/
/********************************************************************/
bool atl_add_two_pointer_hash_set(TwoPointerHashSet* set,void* ptr1,void* ptr2)
{
TwoPointerHashSetElement *base,*tmp;
  
  base = &(set->data[atl_two_pointer_hash_function(ptr1,ptr2)]);
  if (base->ptr1 == NULL)
   {base->ptr1 = ptr1;
    base->ptr2 = ptr2;
    return true;
   }
  for(tmp=base;tmp!=NULL;tmp=tmp->next)
    if ((tmp->ptr1 == ptr1) && (tmp->ptr2 == ptr2))
      return false;
  
  tmp = (TwoPointerHashSetElement*)malloc(sizeof(TwoPointerHashSetElement));
  assertion( tmp != NULL, "atl_add_two_pointer_hash_set: not enough memory" );
  
  tmp->ptr1 = ptr1;
  tmp->ptr2 = ptr2;
  tmp->next  = base->next;
  base->next = tmp;
  return true;
}

void atl_remove_two_pointer_hash_set(TwoPointerHashSet* set,void* ptr1,void* ptr2)
{
TwoPointerHashSetElement *base,*tmp;
  
  base = &(set->data[atl_two_pointer_hash_function(ptr1,ptr2)]);
  if ((base->ptr1 == ptr1) && (base->ptr2 == ptr2))
   {if (base->next == NULL)
     {base->ptr1 = NULL;
      base->ptr2 = NULL;
      return;
     }
    tmp = base->next;
    base->ptr1 = tmp->ptr1;
    base->ptr2 = tmp->ptr2;
    base->next = tmp->next;
    free( tmp );
    return;
   }
  for(tmp=base;tmp!=NULL;tmp=tmp->next)
   {if ((tmp->ptr1 == ptr1) && (tmp->ptr2 == ptr2))
     {base->next = tmp->next;
      free( tmp );
      break;
     }
    base = tmp;
   }
}


/********************************************************************/
/**                           Hash Set                             **/
/********************************************************************/
void atl_reinit_hash_set(HashSet* set)
{
HashSetElement *tmp,*coffin;
int i;
  
  if (set->is_not_plain)
   {
    for(i=0;i<ATL_HASH_SIZE;i++)
     {for(tmp=set->data[i].next;tmp!=NULL;)
       {coffin = tmp;
        tmp = tmp->next;
        free(coffin);
       }
     }
    set->is_not_plain = 0;
   }
  memset(set->data,0,sizeof(set->data));
}

bool atl_add_hash_set(HashSet* set,void* ptr)
{
HashSetElement *base,*tmp;
  
  base = &(set->data[atl_pointer_hash_function(ptr)]);
  if (base->ptr == NULL)
   {base->ptr = ptr;
    return true;
   }
  for(tmp=base;tmp!=NULL;tmp=tmp->next)
    if (tmp->ptr == ptr)
      return false;
  
  tmp = (HashSetElement*)malloc(sizeof(HashSetElement));
  assertion( tmp != NULL, "add_hash_set: not enough memory" );
  
  tmp->ptr   = ptr;
  tmp->next  = base->next;
  base->next = tmp;
  set->is_not_plain = 1;
  return true;
}

void atl_remove_hash_set(HashSet* set,void* ptr)
{
HashSetElement *base,*tmp;
  
  base = &(set->data[atl_pointer_hash_function(ptr)]);
  if (base->ptr == ptr)
   {if (base->next == NULL)
     {base->ptr = NULL;
      return;
     }
    tmp = base->next;
    base->ptr  = tmp->ptr;
    base->next = tmp->next;
    free( tmp );
    return;
   }
  for(tmp=base;tmp!=NULL;tmp=tmp->next)
   {if (tmp->ptr == ptr)
     {base->next = tmp->next;
      free( tmp );
      break;
     }
    base = tmp;
   }
}

bool atl_contains_hash_set( HashSet* set, void* ptr )
{
HashSetElement* tmp;
  
  tmp = &(set->data[atl_pointer_hash_function(ptr)]);
  for(;tmp!=NULL;tmp=tmp->next)
   {if (tmp->ptr == ptr)
      return true;
   }
  return false;
}


/********************************************************************/
/**                     Hash Map: Pointer to int                   **/
/********************************************************************/
void atl_reinit_integer_hash_map(IntegerHashMap* map)
{
IntegerHashMapElement *tmp,*coffin;
int i;
  
  if (map->is_not_plain)
   {for(i=0;i<ATL_HASH_SIZE;i++)
     {for(tmp=map->data[i].next;tmp!=NULL;)
       {coffin = tmp;
        tmp = tmp->next;
        free(coffin);
       }
     }
    map->is_not_plain = 0;
   }
  memset(map->data,0,sizeof(map->data));
}

int* atl_get_integer_hash_map(IntegerHashMap* map,void* key)
{
IntegerHashMapElement* tmp;
  
  for(tmp=&(map->data[atl_pointer_hash_function(key)]);tmp!=NULL;tmp=tmp->next)
    if (tmp->key == key)
      return &tmp->value;
  return NULL;
}

void atl_put_integer_hash_map(IntegerHashMap* map,void* key,int value)
{
IntegerHashMapElement *base,*tmp;
  
  base = &(map->data[atl_pointer_hash_function(key)]);
  if (base->key == NULL)
   {base->key   = key;
    base->value = value;
    return;
   }
  tmp = (IntegerHashMapElement*)malloc(sizeof(IntegerHashMapElement));
  assertion( tmp != NULL, "put_integer_hash_map: not enough memory" );
  
  tmp->key   = key;
  tmp->value = value;
  tmp->next  = base->next;
  base->next = tmp;
  map->is_not_plain = 1;
}

