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
 * File: TraceList.h 
 * Description: 
 *       This is a header file of TraceList.
 */


#ifndef __TRACE_LIST_H__
#define __TRACE_LIST_H__

#include "TraceBool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _traceListitem TraceListitem;
struct _traceListitem {
     void *data;
     TraceListitem *prev, *next;
};

struct _tracelist {
     TraceListitem *head, *tail;
};
typedef struct _tracelist TraceList;

struct _tracepair {
   char *first;
   char *second;
};
typedef struct _tracepair TracePair;

char* tracer_cloneString(const char* str);
TracePair * TracePair_create(const char *first, const char *second);
TracePair * TracePair_create_clone(const char *first, const char *second);
void TracePair_free(TracePair *tp);

TraceList * TraceList_create(void);
void TraceList_clear(TraceList *);
TraceListitem *TraceList_add_front(TraceList *list, void *data);
TraceListitem *TraceList_add_back(TraceList *list, void *data);
void TraceList_remove(TraceList *list, TraceListitem *item);
void *TraceList_del_front(TraceList *list);
void *TraceList_del_back(TraceList *list);
void TraceList_itr(TraceList *list, TraceBool (*func)(TraceList *list, TraceListitem *item, const void *arg1, void *arg2), const void *arg1, void *arg2);


#ifdef __cplusplus
}
#endif

#endif /* __TRACE_LIST_H__ */
