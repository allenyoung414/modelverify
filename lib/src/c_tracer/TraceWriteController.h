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
 * File: TraceWriteController.h 
 * Description: 
 *       This is a header file of TraceWriteController.
 */

#ifndef __TRACE_WRITE_CONTROLLER_H__
#define __TRACE_WRITE_CONTROLLER_H__


#include "TraceWriter.h"
#include "TraceBool.h"
#include <atl/string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TraceWriteController TraceWriteController;
struct _TraceWriteController
{
  void * state;
  void (*delete_state)(void *state);
  void (*puts)(void *state, String *str);
  void (*flush)(void *state);

  TraceBool (*addWriter)(void *state, const char* name, TraceWriter* writer);
  TraceBool (*hasWriter)(void *state, const char* name);
  TraceBool (*removeWriter)(void *state, const char* name, String *finalizer);
  TraceBool (*isTraceEnabled)(void *state);

  void (*pushWriters)(void *state);
  void (*popWriters)(void *state);

  TraceBool (*valid)(TraceWriteController *tc);
};


void TraceWriteController_delete(TraceWriteController *tc);


#ifdef __cplusplus
}
#endif

#endif /* __TRACE_WRITE_CONTROLLER_H__ */
