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
 * File: TSEnvironment.c
 * Description:
 *       This file contains an implementation of TSEnvironment.
 */

#include <stdlib.h>
#include <string.h>

#include "TSEnvironment.h"
#include "TraceList.h"

#include <atl/stringbuffer.h>


/* CTesK Source Start */

/* Duplicate contents of StringBuffer into newly-allocated char array and destroy that StringBuffer */
static char *stringbuffer_to_memory(StringBuffer *ss)
{
  String *result = toString(ss);
  char *tmpptr = tracer_cloneString( toCharArray_String(r(result)) );
  destroy(result);
  return tmpptr;
}


/********************************************************************/
/**                        Win TS Environment                      **/
/********************************************************************/
#ifdef WIN32

#include <windows.h>
#include <tchar.h>

static String* trim( TCHAR* str )
{
int start, end;
StringBuffer *ss = create_StringBuffer();

    if( str == NULL || *str == 0 || ss == NULL )	return create_String("");

    for( start = 0; str[start] != 0; start++ ) {
        if( str[start] != (_TCHAR)(' ') ) break;
    }
    for( end = strlen(str)-1; end >= 0; end-- ) {
        if( str[end] != (_TCHAR)(' ') ) break;
    }
    if( start > end  ) return create_String("");

	appendCharArray_StringBuffer( r(ss), str + start, end - start + 1 );

    return toString(ss);
}

/** @return newly-allocated char array */
static char* WinTSEnvironment_getOSInfo()
{
OSVERSIONINFO osInfo;
StringBuffer *ss;

  osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osInfo)) return NULL;

  ss = create_StringBuffer();
  if (ss == NULL)	return NULL;

  // OS Name

  switch (osInfo.dwPlatformId) {
    case VER_PLATFORM_WIN32s:
//    ss << "Windows " << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion;
	  appendFormat_StringBuffer( r(ss), "Windows %li.%li", (long)osInfo.dwMajorVersion, (long)osInfo.dwMinorVersion);
      break;
    case VER_PLATFORM_WIN32_WINDOWS:
      if (osInfo.dwMajorVersion == 4) {
		if (osInfo.dwMinorVersion == 0)
//        ss << "Windows 95";
	      append_StringBuffer( r(ss), "Windows 95" );
	    else if (osInfo.dwMinorVersion < 90)
//	      ss << "Windows 98";
	      append_StringBuffer( r(ss), "Windows 98" );
		else
//	      ss << "Windows ME";
	      append_StringBuffer( r(ss), "Windows ME" );
	    break;
      }
//    ss << "Windows " << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion;
	  appendFormat_StringBuffer( r(ss), "Windows %li.%li", (long)osInfo.dwMajorVersion, (long)osInfo.dwMinorVersion);
      break;
    case VER_PLATFORM_WIN32_NT:
      if (osInfo.dwMajorVersion == 5) {
        if (osInfo.dwMinorVersion == 0)
//        ss << "Windows 2000";
          append_StringBuffer( r(ss), "Windows 2000" );
	    else if (osInfo.dwMinorVersion == 1)
//	  ss << "Windows XP";
          append_StringBuffer( r(ss), "Windows XP" );
	    else
//	  ss << "Windows NT";
          append_StringBuffer( r(ss), "Windows NT" );
	    break;
      }
//    ss << "Windows NT " << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion;
	  appendFormat_StringBuffer( r(ss), "Windows NT %li.%li", (long)osInfo.dwMajorVersion, (long)osInfo.dwMinorVersion);
      break;
    default:
//    ss << "Unknown Windows " << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion;
	  appendFormat_StringBuffer( r(ss), "Unknown Windows %li.%li", (long)osInfo.dwMajorVersion, (long)osInfo.dwMinorVersion);
      break;
  }

  // OS Build

  // if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
  //   ss << "(build " << LOWORD(osInfo.dwPlatformId) << ")";
  // else
  //   ss << "(build " << osInfo.dwPlatformId << ")";
  // OS Service Pack
  if (strlen(osInfo.szCSDVersion) > 0) {
//  ss << " " << trim( osInfo.szCSDVersion );
	append_StringBuffer( r(ss), " " );
	appendString_StringBuffer( r(ss), trim(osInfo.szCSDVersion) );
  }

  return stringbuffer_to_memory(ss);
}

/** @return newly-allocated char array */
static char * WinTSEnvironment_getHostInfo()
{
DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
char *buff = (char *) malloc(MAX_COMPUTERNAME_LENGTH + 1);
  if(NULL != buff)
    if (!GetComputerName(buff,&size)) { free(buff); return NULL; }

  return buff;
}

#endif /* WIN32 */


/********************************************************************/
/**                       Linux TS Environment                     **/
/********************************************************************/
#ifdef linux

#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>

/** @return newly-allocated char array */
static char * read_file(const char* filename)
{
int fd;
char buffer[256];
size_t len;
StringBuffer *res = create_StringBuffer();

  if (res == NULL)	return NULL;

  if (filename != NULL) {
    fd = open( filename, O_RDONLY );
    if (fd != -1) {
      for(;;) {
        len = read( fd, buffer, sizeof(buffer) );
        if (len <= 0)	break;
		appendCharArray_StringBuffer( r(res), buffer, len );
      }
      close(fd);
    }
  }
  return stringbuffer_to_memory(res);
}

static String* trim( char* str )
{
size_t start, end, len;
StringBuffer *ss = create_StringBuffer();
	
    if( str == NULL || *str == 0 || ss == NULL )	return create_String("");

	for( start = 0; str[start] != 0; start++ ) {
	    if( !isspace(str[start]) && !iscntrl(str[start]) ) break;
	}
	for( end = strlen(str)-1; end > 0; end-- ) {
	    if( !isspace(str[end]) && !iscntrl(str[end]) ) break;
	}
	if( start > end ) return create_String("");
	
	appendCharArray_StringBuffer( r(ss), str + start, end - start + 1 );

    return toString(ss);
}



#define KERNEL_OSTYPE      "/proc/sys/kernel/ostype"
#define KERNEL_OSRELEASE   "/proc/sys/kernel/osrelease"

/** @return newly-allocated char array */
char * LinuxTSEnvironment_getOSInfo()
{
StringBuffer *ss = create_StringBuffer();
char *ptr1;

  if (ss == NULL)	return NULL;

//  ss << trim( read_file( KERNEL_OSTYPE ) ) << " " << trim( read_file( KERNEL_OSRELEASE ) );
  ptr1 = read_file( KERNEL_OSTYPE );
  if(NULL != ptr1) {
	appendString_StringBuffer( r(ss), trim(ptr1) );
    free(ptr1);
  }

  append_StringBuffer( r(ss), " " );

  ptr1 = read_file( KERNEL_OSRELEASE );
  if(NULL != ptr1) {
	appendString_StringBuffer( r(ss), trim(ptr1) );
    free(ptr1);
  }

  return stringbuffer_to_memory(ss);
}

// Linux has problems with dynamic determination of MAXHOSTNAMELEN (HOST_NAME_MAX).
// So we use our own maximum value.
#define LINUX_HOST_NAME_MAX 1024


/** @return newly-allocated char array */
char * LinuxTSEnvironment_getHostInfo()
{
char *hostname = NULL;

  hostname = (char *) malloc(LINUX_HOST_NAME_MAX+1);

  if(NULL != hostname)
    if (gethostname(hostname,LINUX_HOST_NAME_MAX) != 0) {
      free(hostname); return NULL;
    }

  return hostname;
}

#endif /* linux */
/* CTesK Source End */


/********************************************************************/
/**                     Default TS Environment                     **/
/********************************************************************/

static char* DefaultTSEnvironment_getOSInfo()
{
  char* res;

  res = (char*)getenv("OSTYPE");
  if( res == NULL ) res = (char*)getenv("OS"); // cygwin
  if( res == NULL ) res = "";
  return tracer_cloneString(res);
}

static char* DefaultTSEnvironment_getHostInfo()
{
  char* res;

  res = (char *)getenv("HOSTNAME");
  if( res == NULL ) res = (char*)getenv("COMPUTERNAME"); // cygwin
  if( res == NULL ) res = "";
  return tracer_cloneString(res);
}


/********************************************************************/
/**                    TS Environment Factory                      **/
/********************************************************************/
TSEnvironment* TSEnvironment_createTSEnvironment()
{
  static TSEnvironment tsEnv;

#if defined (linux)
  tsEnv.getOSInfo   = LinuxTSEnvironment_getOSInfo;
  tsEnv.getHostInfo = LinuxTSEnvironment_getHostInfo;
  return &tsEnv;
/* CTesK Source Start */
#elif defined (WIN32)
  tsEnv.getOSInfo   = WinTSEnvironment_getOSInfo;
  tsEnv.getHostInfo = WinTSEnvironment_getHostInfo;
  return &tsEnv;
/* CTesK Source End */
#else
  tsEnv.getOSInfo   = DefaultTSEnvironment_getOSInfo;
  tsEnv.getHostInfo = DefaultTSEnvironment_getHostInfo;
  return &tsEnv;
#endif
}




