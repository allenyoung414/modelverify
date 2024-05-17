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
 * File: TraceBufferWriter.c
 * Description: 
 *       This file contains an implementation of TraceBufferWriter.
 */

#include <stdlib.h>

#include "TraceBufferWriter.h"


TraceWriter * TraceBufferWriter_create(StringBuffer *sb)
{
  TraceWriter * tr = (TraceWriter *)malloc(sizeof(*tr));
  if(tr != NULL) {
    tr->state = sb;
    tr->delete_state = TraceBufferWriter_delete_state;
    tr->puts = TraceBufferWriter_puts;
    tr->flush = TraceBufferWriter_flush;
	tr->refCnt = 0;
  }
  return tr;
}

void TraceBufferWriter_delete_state(void *sb)
{
  destroy((StringBuffer*)sb);
}

void TraceBufferWriter_puts(void *sb, String *str)
{
  if(sb != NULL && str != NULL)
	appendString_StringBuffer( r((StringBuffer *)sb), str);
}

void TraceBufferWriter_flush(void *sb) {}

