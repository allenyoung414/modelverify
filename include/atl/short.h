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
 * File: short.h
 * Description:
 *       This file declares the specification types representing 
 *       signed and unsigned short integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_SHORT_H__
#define __ATL_SHORT_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SEC__
  #pragma SEC file
  specification typedef signed short Short;
  specification typedef unsigned short UShort;
#else
  typedef signed short Short;
  extern const Type type_Short;
  typedef unsigned short UShort;
  extern const Type type_UShort;
#endif


/*
 * Constructs a newly allocated Short object
 * that represents the primitive short argument.
 */
Short *create_Short( short i );

/*
 * Constructs a newly allocated UShort object
 * that represents the primitive unsigned short argument.
 */
UShort *create_UShort( unsigned short i );

/*
 * Returns the value of this Short as an short.
 */
short value_Short( Short *i );

/*
 * Returns the value of this UShort as an unsigned short.
 */
unsigned short value_UShort( UShort *i );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
