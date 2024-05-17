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
 * FILE:        system.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    18 April 2005
 * DESCRIPTION: Unified platform-independent
 *              interface for different concepts
 *              such as semaphores
 */

#include <ts/system.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>


/********************************************************************/
/**                          Sleep Function                        **/
/********************************************************************/
/* CTesK Source Start */
#ifdef WIN32

#include <windows.h>

void sleep_msec(time_t msecs)
{
  Sleep(msecs);
}

#else
/* CTesK Source End */

void sleep_msec(time_t msecs)
{
struct timespec tx;

  tx.tv_sec = msecs/1000;
  tx.tv_nsec = (msecs%1000)*1000000;
  nanosleep(&tx,NULL);
}

/* CTesK Source Start */
#endif
/* CTesK Source End */


/********************************************************************/
/**                           Semaphores                           **/
/********************************************************************/
#if (defined __linux__) || (defined _POSIX_SEMAPHORES)

/* POSIX semaphores */
#include <semaphore.h>

#define SEMA(sema)   ((sem_t*)sema)

TSSemaphore ts_create_semaphore(void)
{
sem_t* sema;

  sema = (sem_t*)malloc( sizeof(sem_t) );
  assertion( sema != NULL, "ts_create_semaphore: not enough memory" );

  if (sem_init(sema, 0, 1) != 0)
   {free(sema);
    assertion( false, "ts_create_semaphore: sem_init failed" );
   }
  return sema;
}

bool ts_close_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_close_semaphore: invalid semaphore" );

  sem_wait(SEMA(sema));
  return true;
}

bool ts_open_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_open_semaphore: invalid semaphore" );

  if (sem_post(SEMA(sema)) != 0)
    return false;
  return true;
}

void ts_destroy_semaphore(TSSemaphore sema)
{
  if (sema != NULL)  
   {sem_destroy(SEMA(sema));
    free(sema);
   }
}

#elif defined __MACH__

/* mach semaphores */
#include <mach/semaphore.h>
#include <mach/mach_init.h>

#define SEMA(sema)   ((semaphore_t*) (sema))

TSSemaphore ts_create_semaphore(void)
{
semaphore_t* sema;

  sema = SEMA(malloc(sizeof(semaphore_t) ));
  assertion( sema != NULL, "ts_create_semaphore: not enough memory" );

  if (semaphore_create(current_task(), sema, 0, 1) != 0)
   {free(sema);
    assertion( false, "ts_create_semaphore: semaphore_create failed" );
   }
  return sema;
}

bool ts_close_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_close_semaphore: invalid semaphore" );

  semaphore_wait(*SEMA(sema));
  return true;
}

bool ts_open_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_open_semaphore: invalid semaphore" );

  if (semaphore_signal(*SEMA(sema)) != 0)
    return false;
  return true;
}

void ts_destroy_semaphore(TSSemaphore sema)
{
  if (sema != NULL)  
   {semaphore_destroy(current_task(), *SEMA(sema));
    free(sema);
   }
}

/* CTesK Source Start */
#elif defined(WIN32)

#include <windows.h>

#define WIN32_SEMA(sema)   (*((HANDLE*)sema))

TSSemaphore ts_create_semaphore(void)
{
HANDLE sema;
HANDLE* res;

  sema = CreateSemaphore(NULL,1,1,NULL);
  assertion( sema != NULL, "ts_create_semaphore: CreateSemaphore failed (%d)", GetLastError() );
  
  res = (HANDLE*)malloc( sizeof(HANDLE) );
  assertion( res != NULL, "ts_create_semaphore: not enough memory" );

  *res = sema;
  return res;
}

bool ts_close_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_close_semaphore: invalid semaphore" );

  return (WaitForSingleObject(WIN32_SEMA(sema),INFINITE) != WAIT_FAILED);
}

bool ts_open_semaphore(TSSemaphore sema)
{
  assertion( sema != NULL, "ts_open_semaphore: invalid semaphore" );

  return (ReleaseSemaphore(WIN32_SEMA(sema),1,NULL) != 0);
}

void ts_destroy_semaphore(TSSemaphore sema)
{
  if (sema != NULL)  
   {CloseHandle(WIN32_SEMA(sema));
    free(sema);
   }
}
/* CTesK Source End */

#endif


/********************************************************************/
/**                       Exception Handling                       **/
/********************************************************************/

void fire_exception_event(void);


/* ===================================== *
 * POSIX Exception Handling
 * ===================================== */
#if defined(__GNUC__)
#include <string.h>

#ifndef __CYGWIN__
#include <execinfo.h>
#endif

static struct sigaction ts_previous_sigint_sigaction;
static struct sigaction ts_previous_sigsegv_sigaction;
static struct sigaction ts_previous_sigterm_sigaction;
static PosixExceptionHandler ts_posix_exception_handler;

static void ts_exception_handler(int signo, siginfo_t* info, void* context)
{
struct sigaction action;
struct sigaction* previous_action;
sigset_t sigset, oldset;

  ts_posix_exception_handler(info,context); 
  
  switch (signo)
   {
    case SIGINT :
      previous_action = &ts_previous_sigint_sigaction;
      break;
    case SIGSEGV :
      previous_action = &ts_previous_sigsegv_sigaction;
      break;
    case SIGTERM :
      previous_action = &ts_previous_sigterm_sigaction;
      break;
    default :
      return;
   }
  sigaction(signo, previous_action, &action);

  sigemptyset(&sigset);
  sigaddset(&sigset, signo);
  sigprocmask(SIG_UNBLOCK, &sigset, &oldset);
  kill(getpid(), signo);
  sigprocmask(SIG_SETMASK, &oldset, NULL);
  sigaction(signo, &action, NULL);
}

void start_exception_protected_section(PosixExceptionHandler exception_handler)
{
struct sigaction action;

  ts_posix_exception_handler = exception_handler;

  memset(&action, 0, sizeof(sigaction));
  action.sa_sigaction = ts_exception_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  sigaction(SIGINT,  &action, &ts_previous_sigint_sigaction );
  sigaction(SIGSEGV, &action, &ts_previous_sigsegv_sigaction);
  sigaction(SIGTERM, &action, &ts_previous_sigterm_sigaction);
}

void end_exception_protected_section(void)
{
  sigaction(SIGINT,  &ts_previous_sigint_sigaction,  NULL);
  sigaction(SIGSEGV, &ts_previous_sigsegv_sigaction, NULL);
  sigaction(SIGTERM, &ts_previous_sigterm_sigaction, NULL);
}

#ifndef __CYGWIN__
char* ts_print_backtrace(char* prefix)
{
void*  bt[128];
char** bt_strings;
int bt_size;
size_t res_size;
char* res;
int i,j;

    bt_size = backtrace( bt, sizeof(bt) / sizeof(void *) );
    bt_strings = backtrace_symbols( bt, bt_size );
    if (bt_strings == NULL)
    {
        return NULL;
    }

    res_size = (prefix != NULL ? strlen(prefix) : 0) + 128;
    for( i = 0; i < bt_size; i++ )
     {
        res_size = res_size + strlen(bt_strings[i]) + 2;
     }
    res = malloc(res_size);
    if (res == NULL)
    {
        return NULL;
    }
    j = sprintf( res, "%s\n\n########## Backtrace ##########\n", prefix );
    for( i = 0; i < bt_size; i++ )
     {
        j = j + sprintf( res + j, "%s\n", bt_strings[i] );
	 }
    free( bt_strings );
    return res;
}
#endif

void ts_default_exception_handler(siginfo_t* info, void* context)
{
#ifndef __CYGWIN__
  char* backtrace_info;
#endif
  static char buff[256];
  
  fire_exception_event();

  switch (info->si_signo)
   {
    case SIGINT :
      traceSystemInfo("Test has been terminated by signal SIGINT");
      closeTrace();
      break;
    case SIGSEGV :
      #if !defined(SEGV_MAPERR) || !defined(SEGV_ACCERR)
        sprintf( buff, "Test has been terminated by signal SIGSEGV at address '%p'", info->si_addr );
      #else
        if (info->si_code == SEGV_MAPERR)
          sprintf( buff, "Invalid memory address '%p'", info->si_addr );
        else if (info->si_code == SEGV_ACCERR)
          sprintf( buff, "Invalid permissions for memory at '%p'", info->si_addr );
        else
          sprintf( buff, "Unknown memory access violation at '%p'", info->si_addr );
      #endif
      
#ifndef __CYGWIN__
      backtrace_info = ts_print_backtrace(buff);
      if (backtrace_info != NULL)
      {
          traceInternalError(backtrace_info);
          free(backtrace_info);
      }
      else
      {
          traceInternalError(buff);
      }
#else
          traceInternalError(buff);
#endif
      
      closeTrace();
      break;
    case SIGTERM :
      traceSystemInfo("Test has been terminated by signal SIGTERM");
      closeTrace();
      break;
    default :
      sprintf( buff, "Unexpected signal '%d'", info->si_signo );
      traceInternalError(buff);
      closeTrace();
      break;
   }
} 


/* CTesK Source Start */
/* ===================================== *
 * Win32 Exception Handling
 * ===================================== */
#elif defined(_MSC_VER)

int ts_default_exception_handler(LPEXCEPTION_POINTERS info)
{
static char buff[256];

  switch (info->ExceptionRecord->ExceptionCode)
   {
    case EXCEPTION_BREAKPOINT :
    case EXCEPTION_SINGLE_STEP :
      return EXCEPTION_CONTINUE_SEARCH;
    case EXCEPTION_ACCESS_VIOLATION :
      fire_exception_event();
      sprintf( buff, "The instruction at '0x%p' referenced memory at '0x%p'. The memory could not be '%s'.",
                                info->ExceptionRecord->ExceptionAddress,
                                info->ExceptionRecord->ExceptionInformation[1],
                                info->ExceptionRecord->ExceptionInformation[0] ? "written" : "read"
             );
      traceExceptionInfo(buff);
      traceInternalError("Memory access violation");
      closeTrace();
      break;
    default :
      fire_exception_event();
      sprintf( buff, "Exception 0x%lX occurs at 0x%p",
                                info->ExceptionRecord->ExceptionCode, 
                                info->ExceptionRecord->ExceptionAddress
             );
      traceInternalError(buff);
      closeTrace();
      break;
   }
  return EXCEPTION_CONTINUE_SEARCH;
}
/* CTesK Source End */

#endif


/********************************************************************/
/**               User Callback On Exception Handler               **/
/********************************************************************/
/* User callback function is called by ts_default_exception_handler() */

static ExceptionEventHandler exception_event_handler = NULL;

ExceptionEventHandler ts_set_exception_event_handler(ExceptionEventHandler handler)
{
ExceptionEventHandler old_handler;

    old_handler = exception_event_handler;
    exception_event_handler = handler;
    return old_handler;
}

void fire_exception_event(void)
{
  if (exception_event_handler != NULL)
    exception_event_handler();
}
