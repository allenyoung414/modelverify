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
 * FILE:        system.h
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    18 April 2005
 * DESCRIPTION: Unified platform-independent
 *              interface for different concepts
 *              such as semaphores
 * SCOPE:       It is an interface header.
 */

#ifndef TS_SYSTEM_H
#define TS_SYSTEM_H

#include <time.h>
#include "utils/boolean.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                          Sleep Function                        **/
/********************************************************************/
void sleep_msec(time_t msecs);


/********************************************************************/
/**                           Semaphores                           **/
/********************************************************************/
typedef void* TSSemaphore;

TSSemaphore ts_create_semaphore(void);
bool ts_close_semaphore(TSSemaphore sema);
bool ts_open_semaphore(TSSemaphore sema);
void ts_destroy_semaphore(TSSemaphore sema);


/********************************************************************/
/**                       Exception Handling                       **/
/********************************************************************/

/* ===================================== *
 * POSIX Exception Handling
 * ===================================== */
#if defined(__GNUC__)

#include <signal.h>

typedef void (*PosixExceptionHandler)(siginfo_t* info, void* context);

void start_exception_protected_section(PosixExceptionHandler exception_handler);
void end_exception_protected_section(void);


#define START_EXCEPTION_PROTECTED_SECTION(handler)  start_exception_protected_section(handler);
#define END_EXCEPTION_PROTECTED_SECTION()           end_exception_protected_section();

void ts_default_exception_handler(siginfo_t* info, void* context);


/* CTesK Source Start */
/* ===================================== *
 * Win32 Exception Handling
 * ===================================== */
#elif defined(_MSC_VER)

#include <windows.h>

typedef int (*Win32ExceptionHandler)(LPEXCEPTION_POINTERS info);

#define START_EXCEPTION_PROTECTED_SECTION(handler)  { Win32ExceptionHandler ts_exception_handler = handler; __try { 
#define END_EXCEPTION_PROTECTED_SECTION()           } __except (ts_exception_handler(GetExceptionInformation())) {} }

int ts_default_exception_handler(LPEXCEPTION_POINTERS info);
/* CTesK Source End */


/* ===================================== *
 * Absence of Exception Handling
 * ===================================== */
#else

#define START_EXCEPTION_PROTECTED_SECTION(handler)  
#define END_EXCEPTION_PROTECTED_SECTION()          


#ifdef __cplusplus
}
#endif

#endif


/********************************************************************/
/**               User Callback On Exception Handler               **/
/********************************************************************/
/* User callback function is called by ts_default_exception_handler() */

typedef void (*ExceptionEventHandler)(void);

ExceptionEventHandler ts_set_exception_event_handler(ExceptionEventHandler handler);


#endif
