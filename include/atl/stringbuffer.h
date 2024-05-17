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
 * File: stringbuffer.h
 * Description:
 *       This file declares the specification type representing
 *       string buffers.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/object.h>

#ifndef __ATL_STRING_BUFFER_H__
#define __ATL_STRING_BUFFER_H__

#include <utils/boolean.h>

/*
 * =============================================================================
 *
 * 'StringBuffer' type
 *
 * =============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SEC__
  #pragma SEC file
  specification typedef struct StringBuffer StringBuffer;
#else
  typedef struct StringBuffer StringBuffer;
  extern const Type type_StringBuffer;
#endif


/*
 * Create empty StringBuffer.
 */
StringBuffer* create_StringBuffer(void);

/*
 * Create empty StringBuffer with the given allocateBy.
 */
StringBuffer* createCustomized_StringBuffer( int allocateBy );

/*
 * Empty StringBuffer
 */
void clear_StringBuffer( StringBuffer* self );

/*
 * Returns the current size of the string buffer.
 */
int size_StringBuffer( StringBuffer* self );

/*
 * Returns the character at the specified index.
 */
char charAt_StringBuffer( StringBuffer* self, int index );

/*
 * Appends the specified char to the end of the string buffer.
 */
void appendChar_StringBuffer( StringBuffer* self, char ch );

/*
 * Appends the specified null-terminating string
 * to the end of the string buffer.
 */
void append_StringBuffer( StringBuffer* self, const char* str );

/*
 * Appends the specified char array to the end of the string buffer.
 */
void appendCharArray_StringBuffer( StringBuffer* self, const char* arr, int size );

/*
 * Appends the specified string to the end of the string buffer.
 */
void appendString_StringBuffer( StringBuffer* self, String* str );

/*
 * Appends the specified string buffer to the end of the string buffer.
 */
void appendStringBuffer_StringBuffer( StringBuffer* self, StringBuffer* strbuf );

/*
 * Appends the string with formatted data like sprintf
 * to the end of the string buffer.
 */
void appendFormat_StringBuffer( StringBuffer* self, const char* format, ... );

void vappendFormat_StringBuffer( StringBuffer* self, const char* format, va_list args );

/*
 * Tests if this string buffer ends with the specified suffix.
 */
bool endsWith_StringBuffer( StringBuffer* self, String* suffix );

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified character.
 */
int indexOfChar_StringBuffer( StringBuffer* self, int ch );

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified character, starting the search at the specified index.
 */
int indexOfCharFrom_StringBuffer( StringBuffer* self, int ch, int fromIndex );

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified substring.
 */
int indexOfString_StringBuffer( StringBuffer* self, String* str );

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified substring, starting at the specified index.
 */
int indexOfStringFrom_StringBuffer( StringBuffer* self, String* str, int fromIndex );

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified character.
 */
int lastIndexOfChar_StringBuffer( StringBuffer* self, int ch );

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified character, searching backward starting
 * at the specified index.
 */
int lastIndexOfCharFrom_StringBuffer( StringBuffer* self, int ch, int fromIndex );

/*
 * Returns the index within this string buffer of the rightmost occurrence
 * of the specified substring.
 */
int lastIndexOfString_StringBuffer( StringBuffer* self, String* str );

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified substring.
 */
int lastIndexOfStringFrom_StringBuffer( StringBuffer* self, String* str, int fromIndex );

/*
 * Replaces all occurrences of oldChar in the string buffer with newChar.
 */
void replace_StringBuffer( StringBuffer* self, char oldChar, char newChar );

/*
 * Tests if this string buffer starts with the specified prefix.
 */
bool startsWith_StringBuffer( StringBuffer* self, String* prefix );

/*
 * Tests if this string buffer starts with the specified prefix beginning
 * a specified index.
 */
bool startsWithOffset_StringBuffer( StringBuffer* self, String* prefix, int toffset );

/*
 * Returns a new string that is a substring of the string buffer.
 */
String* substringFrom_StringBuffer( StringBuffer* self, int beginIndex );

/*
 * Returns a new string that is a substring of the string buffer.
 */
String* substring_StringBuffer( StringBuffer* self, int beginIndex, int endIndex );

#ifdef __cplusplus
} // extern "C"
#endif

#endif
