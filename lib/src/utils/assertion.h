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
 * File: assertion.h
 * Description: 
 *       This file declares assertion functions.
 */

#ifndef ASSERTION_H
#define ASSERTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utils/boolean.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FORMAT( format ) __FILE__ "(%d): " format "\n", __LINE__


/********************************************************************/
/**                    Assertion Trap Handlers                     **/
/********************************************************************/

/* Function that is called when assertion traps */
typedef void (*AssertionExit)(const char *format, va_list args);

/**
 * Call specified function when assertion traps.
 * The 'atAssertion' function is passed the address of a function (func) that
 * is called when assertion traps.
 * Return value is the previous handler.
 * If an assertion is trapped in the code of the handler then the application
 * unconditionally exits.
 * If handler returns then application exits.
 */
AssertionExit atAssertion(AssertionExit func);
AssertionExit atFault(AssertionExit func);


/**
 * The functons returns true iff a custom assertion trap handler has been setup
 */
bool isCustomAssertionExit(void);


/********************************************************************/
/**                            Assertions                          **/
/********************************************************************/
void assertion (int expr, const char* format, ...);
void fault (int expr, const char* format, ...);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
