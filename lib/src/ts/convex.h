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
 * PROJECT:     CTesK
 * FILE:        convex.h
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    26 May 2003
 * DESCRIPTION: Contains functions needed
 *              for work with convex types.
 *              It is used by generated code only.
 * SCOPE:       It is an interface header.
 */

#ifndef TS_CONVEX_H
#define TS_CONVEX_H

#include <stddef.h>
#include "utils/boolean.h"
#include "atl/string.h"
#include "atl/object.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                         SEC Type Kind                          **/
/********************************************************************/
typedef
enum SECTypeKind
{
  simpleType,
  structType,
  pointerType,
  arrayType,
  specType,
  subType
} SECTypeKind;


/********************************************************************/
/**                          Type Desc                             **/
/********************************************************************/
typedef struct SECTypeDesc SECTypeDesc;
struct SECTypeDesc
{
  SECTypeKind kind;
  ToString    customToString;
  ToXML       customToXML;
};


/********************************************************************/
/**                       Simple Type Desc                         **/
/********************************************************************/
typedef struct SimpleTypeDesc SimpleTypeDesc;
struct SimpleTypeDesc
{
  SECTypeKind kind;
  ToString    customToString;
  ToXML       customToXML;
  const char* name;
  size_t      sizeOf;
  const char *format;
};


/********************************************************************/
/**                       Struct Type Desc                         **/
/********************************************************************/
typedef struct FieldDesc FieldDesc;
struct FieldDesc
{
  const char*  name;
  unsigned int offset;
  SECTypeDesc* type_desc;
};

typedef struct StructTypeDesc StructTypeDesc;
struct StructTypeDesc
{
  SECTypeKind kind;
  ToString    customToString;
  ToXML       customToXML;
  const char* name;
  size_t      sizeOf;
  int         numFields;
  FieldDesc*  fields;
};


/********************************************************************/
/**                        Pointer Type Desc                       **/
/********************************************************************/
typedef struct PointerTypeDesc PointerTypeDesc;
struct PointerTypeDesc
{
  SECTypeKind  kind;
  ToString     customToString;
  ToXML        customToXML;
  int          level;
  SECTypeDesc* base;
};


/********************************************************************/
/**                         Array Type Desc                        **/
/********************************************************************/
typedef struct ArrayTypeDesc ArrayTypeDesc;
struct ArrayTypeDesc
{
  SECTypeKind  kind;
  ToString     customToString;  // always NULL
  ToXML        customToXML;     // always NULL
  size_t       sizeOfElement;
  int          numElements;
  SECTypeDesc* base;
};


/********************************************************************/
/**                        Spec Type Desc                          **/
/********************************************************************/
typedef struct SpecTypeDesc SpecTypeDesc;
struct SpecTypeDesc
{
  SECTypeKind kind;
  ToString    customToString;  // always NULL
  ToXML       customToXML;     // always NULL
  const Type* type;
};


/********************************************************************/
/**                         Sub Type Desc                          **/
/********************************************************************/
typedef bool(*InvariantCheckerPtr)(void*);

typedef struct SubTypeDesc SubTypeDesc;
struct SubTypeDesc
{
  SECTypeKind         kind;
  ToString            customToString;
  ToXML               customToXML;
  const char*         name;
  SECTypeDesc*        base;
  InvariantCheckerPtr invariant_checker;
};

SECTypeDesc* ts_base_type_desc(SECTypeDesc* type_desc);


/********************************************************************/
/**                        Copy Function                           **/
/********************************************************************/
void ts_start_copy_sectype();
void ts_copy_sectype(SECTypeDesc* desc,void* dst,void* src,bool weak);
void ts_finish_copy_sectype();


/********************************************************************/
/**                       Equals Function                          **/
/********************************************************************/
bool ts_equals_sectype(SECTypeDesc* desc,void* left,void* right);


/********************************************************************/
/**                      ToString, ToXML and dispatching           **/
/********************************************************************/
String* ts_to_string_sectype(SECTypeDesc* desc,void* ptr);
String* ts_to_XML_sectype( SECTypeDesc* desc,void* ptr );
String* ts_to_trace_sectype(SECTypeDesc* desc,void* ptr);

bool ts_start_to_XML();
String* create_single_XML_entry(const char *kind, const char *type, String *value);
void ts_finish_to_XML();


/********************************************************************/
/**                   Check Invariant Function                     **/
/********************************************************************/
bool ts_check_invariant_sectype(SECTypeDesc* desc,void* ptr);


/********************************************************************/
/**                      Enumerate Function                        **/
/********************************************************************/
void ts_enumerate_sectype(SECTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par);


/********************************************************************/
/**                       Destroy Function                         **/
/********************************************************************/
void ts_start_destroy_sectype();
void ts_destroy_sectype(SECTypeDesc* desc,void* ptr,bool weak);
void ts_finish_destroy_sectype();

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
