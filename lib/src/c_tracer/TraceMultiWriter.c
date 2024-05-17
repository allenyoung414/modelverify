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
 * File: TraceMultiWriter.c
 * Description: 
 *       This file contains an implementation of TraceMultiWriter.
 */

extern int tracer_debug_level;

#include <string.h>
#include <stdlib.h>

#include "TraceMultiWriter.h"
#include "TraceList.h"


#define TRACE_MULTIWRITER_DEBUG_LEVEL  3



/********************************************************************/
/**                           Writer Cell                          **/
/********************************************************************/
static WriterCell* TraceMultiWriter_createWriterCell(const char* name,TraceWriter* writer)
{
  WriterCell* wc;

  wc = (WriterCell *)malloc(sizeof(*wc));
  if(wc != NULL) {
    wc->name   = tracer_cloneString(name);
    wc->writer = writer;
    wc->refCnt = 1;
  }
  return wc;
}

static void TraceMultiWriter_deleteWriterCell(WriterCell* wc)
{
  if(wc != NULL) {
    if (wc->name != NULL) free(wc->name);
    if (wc->writer != NULL && wc->writer->refCnt <= 0) TraceWriter_delete(wc->writer);
    free(wc);
  }
}


/********************************************************************/
/**                       Auxiliary Functions                      **/
/********************************************************************/
static TraceBool delete_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  if(item != NULL) {
    TraceMultiWriter_deleteWriterCell((WriterCell *)(item->data));
    TraceList_remove(list, item);
  }
  return tr_false;
}

static TraceBool delete_list_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  if(item) {
    TraceList_itr((TraceList *)(item->data), delete_func, NULL, NULL);
    TraceList_remove(list, item);
  }
  return tr_false;
}

static TraceBool find_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  const char* name = (const char *)arg1;
  TraceListitem** pResult = (TraceListitem**)arg2;
  WriterCell* cell;

  if((item != NULL) && (item->data != NULL)) {
    cell = (WriterCell* )(item->data);
    if((name != NULL) && (cell->name != NULL))
      if (strcmp(cell->name,name) == 0) {
         *pResult = item;
         return tr_true;
      }
  }

  return tr_false;
}

static TraceBool out_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  WriterCell* cell;
  String *str = (String *)arg1;

  if((item != NULL) && (item->data != NULL)) {
    cell = (WriterCell* )(item->data);
    if( cell->writer && cell->writer->puts && str )
      (*cell->writer->puts)(cell->writer->state, r(str));
  }
  return tr_false;
}

static TraceBool flush_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  WriterCell* cell;

  if((item != NULL) && (item->data != NULL)) {
    cell = (WriterCell* )(item->data);
    if( cell->writer && cell->writer->flush )
      (*cell->writer->flush)(cell->writer->state);
  }
  return tr_false;
}

static TraceBool is_enabled_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
  WriterCell* cell;
  TraceBool* pResult = (TraceBool *)arg2; 

  if((item != NULL) && (item->data != NULL)) {
    cell = (WriterCell* )(item->data);
    if(cell->name != NULL) {
      *pResult = tr_true;
      return tr_true;
    }
  }

  return tr_false;
}


/********************************************************************/
/**                       Trace Multi Writer                       **/
/********************************************************************/
TraceBool TraceMultiWriter_state_valid(TraceMultiWriter_state* st)
{
  if((NULL == st->writers) || (NULL == st->stack)) return tr_false;
  return tr_true;
}

TraceBool TraceMultiWriter_valid(TraceWriteController *mr)
{
  if( (NULL == mr) || 
	  (NULL == mr->state) || 
	  (NULL == mr->delete_state) || 
	  (NULL == mr->puts) || 
	  (NULL == mr->flush) || 
	  (NULL == mr->addWriter) || 
	  (NULL == mr->hasWriter) || 
	  (NULL == mr->removeWriter) || 
	  (NULL == mr->isTraceEnabled) || 
	  (NULL == mr->pushWriters) || 
	  (NULL == mr->popWriters) )
	return tr_false;

  return TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr->state);
}

static TraceMultiWriter_state * TraceMultiWriter_state_create()
{
TraceMultiWriter_state *mr_st;

  mr_st = (TraceMultiWriter_state *)malloc(sizeof(*mr_st));
  if(mr_st != NULL) { 
    mr_st->writers = TraceList_create();
    mr_st->stack = TraceList_create();
    if((NULL == mr_st->writers)||(NULL == mr_st->stack)) { 
      if(mr_st->writers) free(mr_st->writers);
      if(mr_st->stack)   free(mr_st->stack);
      free(mr_st); 
      return NULL;
    }
  }
  return mr_st;
}

void TraceMultiWriter_state_delete(void *st)
{
  TraceMultiWriter_state * mr_st = (TraceMultiWriter_state *)st;

  if(mr_st != NULL) {
    TraceList_itr(mr_st->writers, delete_func, NULL, NULL);
    TraceList_itr(mr_st->stack, delete_list_func, NULL, NULL);
    free(mr_st->writers);
    free(mr_st->stack);
    free(mr_st);
  }
}


TraceListitem* TraceMultiWriter_findWriter(TraceMultiWriter_state* mr, const char* name) 
{
TraceListitem* res = NULL;

  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid(mr) ) return NULL;
  TraceList_itr(mr->writers, find_func, name, &res);
  return res;
}

void TraceMultiWriter_puts(void* mr, String * str) 
{
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr) ) return;
  if(str != NULL) {
    TraceList_itr(((TraceMultiWriter_state *)mr)->writers, out_func, str, NULL);
	destroy(str);	// out_func does not change refcount
  }
}

void TraceMultiWriter_flush(void* mr)
{
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr) ) return;
  TraceList_itr(((TraceMultiWriter_state *)mr)->writers, flush_func, NULL, NULL);
}

TraceBool TraceMultiWriter_addWriter(void* mr, const char* name,TraceWriter* writer)
{
TraceListitem* item;
WriterCell* cell;

  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr) ) return tr_false;

  item = TraceMultiWriter_findWriter((TraceMultiWriter_state *)mr, name);
  if (item != NULL && item->data != NULL) {
    cell = (WriterCell *)(item->data);
    cell->refCnt++;
    if (writer != NULL)
      TraceWriter_delete(writer);
    return tr_true;
  }
  cell = TraceMultiWriter_createWriterCell(name,writer);
  if(cell != NULL) {
    TraceList_add_front(((TraceMultiWriter_state *)mr)->writers, cell);
	if (writer != NULL) writer->refCnt ++;
    return tr_true;
  }

  return tr_false;
}

TraceBool TraceMultiWriter_hasWriter(void* mr, const char* name) 
{
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr) ) return tr_false;
  return ( (TraceMultiWriter_findWriter((TraceMultiWriter_state *)mr, name) != NULL) ? tr_true : tr_false );
}

TraceBool TraceMultiWriter_removeWriter(void* mr, const char* name, String * finalizer) 
{
TraceListitem* item;
WriterCell* cell;

  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid((TraceMultiWriter_state *)mr) ) return tr_false;

  item = TraceMultiWriter_findWriter((TraceMultiWriter_state *)mr, name);
  if ((item != NULL) && (item->data != NULL)) {
    cell = (WriterCell *)(item->data);
    cell->refCnt--;
    if (cell->refCnt <= 0) {
	  if(cell->writer && cell->writer->puts && finalizer) {
        (*cell->writer->puts)(cell->writer->state, finalizer);
		cell->writer->refCnt --;
	  }
      TraceList_remove(((TraceMultiWriter_state *)mr)->writers, item);
      TraceMultiWriter_deleteWriterCell( cell );
    }
    return tr_true;
  }

  return tr_false;
}

TraceBool TraceMultiWriter_isTraceEnabled(void* mr)
{
TraceBool res = tr_false;
TraceMultiWriter_state *st;

  st = (TraceMultiWriter_state *)mr;
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid(st) ) return tr_false;
  TraceList_itr(st->writers, is_enabled_func, NULL, &res);
  return res;
}

void TraceMultiWriter_pushWriters(void* mr)
{
TraceMultiWriter_state *st;

  st = (TraceMultiWriter_state *)mr;
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid(st) ) return;
  TraceList_add_front(st->stack, st->writers->head);
  TraceList_clear(st->writers); 
}

void TraceMultiWriter_popWriters(void* mr)
{
TraceMultiWriter_state *st;

  st = (TraceMultiWriter_state *)mr;
  if( (tracer_debug_level >= TRACE_MULTIWRITER_DEBUG_LEVEL) && !TraceMultiWriter_state_valid(st) ) return;
  TraceList_itr(st->writers, delete_func, NULL, NULL); 
  st->writers->head = st->stack->head->data;
  st->writers->tail = st->writers->head;
  TraceList_del_front(st->stack);
}

TraceWriteController * TraceMultiWriter_create()
{
  TraceWriteController *mr;
  TraceMultiWriter_state *mr_st;

  mr = (TraceWriteController *)malloc(sizeof(*mr));
  if(mr != NULL) {
	mr_st = TraceMultiWriter_state_create();
	if(mr_st == NULL) { free(mr); return NULL; }

	mr->state = mr_st;
	mr->delete_state = TraceMultiWriter_state_delete;
	mr->puts = TraceMultiWriter_puts;
	mr->flush = TraceMultiWriter_flush;
	mr->addWriter = TraceMultiWriter_addWriter;
	mr->hasWriter = TraceMultiWriter_hasWriter;
	mr->removeWriter = TraceMultiWriter_removeWriter;
	mr->isTraceEnabled = TraceMultiWriter_isTraceEnabled;
	mr->pushWriters = TraceMultiWriter_pushWriters;
	mr->popWriters = TraceMultiWriter_popWriters;
    mr->valid = TraceMultiWriter_valid;
  }
  return mr;
}

