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
 * File: long.h
 * Description:
 *       This file declares the specification types representing 
 *       signed and unsigned long integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_LONG_H__
#define __ATL_LONG_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __SEC__
  #pragma SEC file
  specification typedef signed long Long;
  specification typedef unsigned long ULong;
#else
  typedef signed long Long;
  extern const Type type_Long;
  typedef unsigned long ULong;
  extern const Type type_ULong;
#endif


/*
 * Constructs a newly allocated Long object
 * that represents the primitive long argument.
 */
Long *create_Long( long i );

/*
 * Constructs a newly allocated ULong object
 * that represents the primitive unsigned long argument.
 */
ULong *create_ULong( unsigned long i );

/*
 * Returns the value of this Long as an long.
 */
long value_Long( Long *i );

/*
 * Returns the value of this ULong as an unsigned long.
 */
unsigned long value_ULong( ULong *i );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
