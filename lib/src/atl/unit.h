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
 * File: unit.h
 * Description:
 *       This file declares the specification type containing the only value.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_UNIT_H__
#define __ATL_UNIT_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SEC__
  #pragma SEC file
  specification typedef void Unit;
#else
  typedef void Unit;
  extern const Type type_Unit;
#endif


/*
 * Constructs a newly allocated Unit object
 * that represents the only value of this type.
 */
Unit* create_Unit();

#ifdef __cplusplus
}
#endif
#endif
