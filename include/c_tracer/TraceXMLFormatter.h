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
 * File: TraceXMLFormatter.h 
 * Description: 
 *       This is a header file of TraceXMLFormatter.
 */

#ifndef __TRACE_XML_FORMATTER__
#define __TRACE_XML_FORMATTER__

#include "TraceFormatter.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * indentation class
 */
struct _Indent 
{
  int  indent_size;
  int  cur_indent;
  char indent_char;
};
typedef struct _Indent Indent;

struct _TraceXMLFormatter_state 
{
  unsigned int indent_size;
  unsigned int cur_indent;
  char indent_char;

  Indent indent;
};
typedef struct _TraceXMLFormatter_state TraceXMLFormatter_state;


TraceFormatter * TraceXMLFormatter_create(int indent_size, char indent_char);
TraceFormatter * TraceXMLFormatter_default_create();



#ifdef __cplusplus
}
#endif

#endif /* __TRACE_XML_FORMATTER__ */
