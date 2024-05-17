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
 * File: TraceList.c
 * Description:
 *       This file contains an implementation of TraceList.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TraceList.h"


char* tracer_cloneString(const char* str)
{
char* res;
  if (str == NULL)	return NULL;
  res = (char *)malloc(strlen(str)+1);
  if (res == NULL)	return NULL;
  strcpy(res,str);
  return res;
}

TracePair * TracePair_create(const char *first, const char *second)
{
  TracePair *tp = (TracePair *)malloc(sizeof(*tp));
  if(tp != NULL) {
    tp->first = (char *)first;
    tp->second = (char *)second;
  }
  return tp;
}

TracePair * TracePair_create_clone(const char *first, const char *second)
{
  TracePair *tp = (TracePair *)malloc(sizeof(*tp));
  if(tp != NULL) {
    tp->first = tracer_cloneString(first);
    tp->second = tracer_cloneString(second);
    if(   (NULL != first && NULL == tp->first)
       || (NULL != second && NULL == tp->second)
      )
    {
      TracePair_free(tp);
      return NULL;
    }
  }
  return tp;
}

void TracePair_free(TracePair *tp)
{
  if(tp != NULL)
  {
    if(NULL != tp->first) free(tp->first);
    if(NULL != tp->second) free(tp->second);
    free(tp);
  }
}


TraceList * TraceList_create(void)
{
    TraceList *lst = (TraceList *)malloc(sizeof(*lst));
    if(lst != NULL) {
		lst->head = lst->tail = NULL;
	}
    return lst;
}

void TraceList_clear(TraceList *lst)
{
  if(lst)
  {
    lst->head = NULL;
    lst->tail = NULL;
  }
}

/*
 * Add an element to the head of the list
 */
TraceListitem *TraceList_add_front(TraceList *list, void *data)
{
TraceListitem *item;

    if(!list) return NULL;

    item = (TraceListitem *)malloc(sizeof(*item));

    if (item == NULL)
      return item;

    item->data = data;
    item->prev = NULL;
    item->next = list->head;
    if (list->head)
      list->head->prev = item;
    list->head = item;
    if (list->tail == NULL)
      list->tail = item;
    return item;
}

/*
 * Add an element to the tail of the list
 */
TraceListitem *TraceList_add_back(TraceList *list, void *data)
{
TraceListitem *item;

    if(!list) return NULL;

    item = (TraceListitem *)malloc(sizeof(*item));

    if (item == NULL)
      return item;

    item->data = data;
    item->next = NULL;
    item->prev = list->tail;
    if( list->tail != NULL )
      list->tail->next = item;
    list->tail = item;
    if( list->head == NULL )
      list->head = item;
    return item;
}

/*
 * Remove an element from the head of the list.
 * Return this element.
 */
void *TraceList_del_front(TraceList *list)
{

TraceListitem *item;

void *data;

    if(!list) return NULL;

    item = list->head;

    if (item == NULL)
      return NULL;
    data = item->data;

    list->head = list->head->next;
    if (list->head)
      list->head->prev = NULL;
    if (list->tail == item)
      list->tail = NULL;
    free(item);
    return data;
}

/*
 * Remove an element from the tail of the list.
 * Return this element.
 */
void *TraceList_del_back (TraceList *list)
{
TraceListitem *item;
void *data;

     if(!list) return NULL;

     item = list->tail;

     if (item == NULL)
	 return NULL;
     data = item->data;

     list->tail = list->tail->prev;
     if (list->tail)
	   list->tail->next = NULL;
     if (list->head == item)
	   list->head = NULL;
     free (item);
     return data;
}

/*
 * Remove an element from the list.
 */
void TraceList_remove(TraceList *list, TraceListitem *item)
{
     if(!list || !item)	return;

     if (item->prev)
	   item->prev->next = item->next;
     if (item->next)
	   item->next->prev = item->prev;
     if (item == list->head)
	   list->head = item->next;
     if (item == list->tail)
	   list->tail = item->prev;
     free(item);
}


/*
 * Iterate through all the items in a list.
 */
void TraceList_itr(TraceList *list, TraceBool (*func)(TraceList *list, TraceListitem *item, const void *arg1, void *arg2), const void *arg1, void *arg2)
{
TraceListitem *item, *tmp;

  if(list) {
	  for(item = list->head; item != NULL; ) {
		  tmp = item->next;
          if (tr_true == (*func)(list, item, arg1, arg2))
             break;
          item = tmp;
	  }
  }
}

