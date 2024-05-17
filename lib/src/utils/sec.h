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
 * File: sec.h
 * Description: 
 *       This file defines macroses hiding compiler specific keywords.
 */

#ifndef __SEC_H__
#define __SEC_H__

#ifdef __SEC__

#if defined __GNUC__

#define   asm         __asm__
#define __asm         __asm__
#define __alignof     __alignof__
#define __attribute   __attribute__
#define __complex     _Complex
#define __complex__   _Complex
#define __const       const
#define __const__     const
#define __imag        __imag__
#define __inline      inline
#define __inline__    inline
#define __real        __real__
#define __restrict    __restrict__
#define __signed      signed
#define __signed__    signed
#define __typeof      typeof
#define __typeof__    typeof
#define __volatile    volatile
#define __volatile__  volatile

/* CTesK Source Start */
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
/* Microsoft Visual Studio .Net */

#define __w64
/* CTesK Source End */

#endif

#endif

#endif
