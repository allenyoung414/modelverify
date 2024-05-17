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
 * File: stdint.h
 * Description: 
 *       This file contains standard integer type definition.
 *       It is needed for platforms, which does not support
 *         ISO C99: 7.18 Integer types <stdint.h>
 */

#ifndef UTILS_STDINT_H
#define UTILS_STDINT_H

/* CTesK Source Start */
#if !defined (_MSC_VER)
/* CTesK Source End */

#include <stdint.h>

/* CTesK Source Start */
#endif
/* CTesK Source End */


/* CTesK Source Start */
/********************************************************************/
/**                  Exact-Width Integer Types                     **/
/********************************************************************/
#if defined (_MSC_VER)

typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;

#endif
/* CTesK Source End */


#endif /* UTILS_STDINT_H */
