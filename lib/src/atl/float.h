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
 * File: float.h
 * Description:
 *       This file declares the specification type representing float values.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_FLOAT_H__
#define __ATL_FLOAT_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __SEC__
  #pragma SEC file
  specification typedef float Float;
#else
  typedef float Float;
  extern const Type type_Float;
#endif

/*
 * Constructs a newly allocated Float object
 * that represents the primitive float argument.
 */
Float *create_Float( float d );

/*
 * Returns the value of this Float as an float.
 */
float value_Float( Float *d );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
