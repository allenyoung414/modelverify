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
 * File: TraceMultiWriter.h 
 * Description: 
 *       This is a header file of TraceMultiWriter.
 */

#ifndef __TRACE_MULTIWRITER_H__
#define __TRACE_MULTIWRITER_H__

#include "TraceWriteController.h"
#include "TraceList.h"
#include "TraceBool.h"
#include <string.h>


/********************************************************************/
/**                       Trace Multi Writer                       **/
/********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

struct _WriterCell
{
  char*        name;
  TraceWriter* writer;
  int          refCnt;
};
typedef struct _WriterCell WriterCell;


struct _TraceMultiWriter_state
{
  TraceList * writers;  // list <WriterCell *>
  TraceList * stack;    // list <list <WriterCell *>>
};
typedef struct _TraceMultiWriter_state TraceMultiWriter_state;


TraceWriteController * TraceMultiWriter_create();


#ifdef __cplusplus
}
#endif

#endif /* __TRACE_MULTIWRITER_H__ */
