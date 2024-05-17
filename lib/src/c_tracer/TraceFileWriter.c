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
 * File: TraceFileWriter.c
 * Description: 
 *       This file contains an implementation of TraceFileWriter.
 */

#include <stdio.h>
#include <stdlib.h>

#include "TraceFileWriter.h"


TraceWriter * TraceFileWriter_create(FILE *file)
{
  TraceWriter * tr = (TraceWriter *)malloc(sizeof(*tr));
  if(tr != NULL) {
    tr->state = file;
    tr->delete_state = TraceFileWriter_delete_state;
    tr->puts = TraceFileWriter_puts;
    tr->flush = TraceFileWriter_flush;
	tr->refCnt = 0;
  }
  return tr;
}

void TraceFileWriter_delete_state(void *file)
{
  if(file != NULL) {
    fflush((FILE *)file);
    fclose((FILE *)file);
  }
}

void TraceFileWriter_puts(void *file, String *str)
{
	if(file != NULL && str != NULL) {
		fputs(toCharArray_String(r(str)), (FILE *)file);
		destroy(str);
	}
}

void TraceFileWriter_flush(void *file)
{
  if(file != NULL) fflush((FILE *)file);
}

