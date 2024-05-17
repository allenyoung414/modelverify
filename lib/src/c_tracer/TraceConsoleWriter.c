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
 * File: TraceConsoleWriter.c
 * Description: 
 *       This file contains an implementation of TraceConsoleWriter.
 */

#include <stdio.h>
#include <stdlib.h>

#include "TraceConsoleWriter.h"


TraceWriter * TraceConsoleWriter_create()
{
  TraceWriter * tr = (TraceWriter *)malloc(sizeof(*tr));
  if(tr != NULL) {
    tr->state = NULL;
    tr->delete_state = TraceConsoleWriter_delete_state;
    tr->puts = TraceConsoleWriter_puts;
    tr->flush = TraceConsoleWriter_flush;
	tr->refCnt = 0;
  }
  return tr;
}

void TraceConsoleWriter_delete_state(void *arg) {}

void TraceConsoleWriter_puts(void *arg, String *str)
{
  if(str != NULL) {
	fputs(toCharArray_String(r(str)), stdout);
	destroy(str);
  }
}

void TraceConsoleWriter_flush(void *arg)
{
  fflush(stdout);
}
