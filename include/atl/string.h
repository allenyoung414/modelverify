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
 * File: string.h
 * Description:
 *       This file declares the specification type representing
 *       null-terminated strings.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/object.h>

#ifndef __ATL_STRING_H__
#define __ATL_STRING_H__

#include <utils/boolean.h>

/*
 * =============================================================================
 *
 * 'String' type
 *
 * =============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SEC__
  #pragma SEC file
  specification typedef struct String String;
#else
  typedef struct String String;
  extern const Type type_String;
#endif

/*
 * Create String from C-string
 */
String *create_String( const char* cstr );

/*
 * Returns the character at the specified index.
 */
char charAt_String( String *self, int index );

/*
 * Concatenates the specified string to the end of this string.
 */
String *concat_String( String *self, String *str );

/*
 * Tests if this string ends with the specified suffix.
 */
bool endsWith_String( String *self, String *suffix );

/*
 * Returns the index within this string of the first occurrence
 * of the specified character.
 */
int indexOfChar_String( String *self, int ch );

/*
 * Returns the index within this string of the first occurrence
 * of the specified character, starting the search at the specified index.
 */
int indexOfCharFrom_String( String *self, int ch, int fromIndex );

/*
 * Returns the index within this string of the first occurrence
 * of the specified substring.
 */
int indexOfString_String( String *self, String *str );

/*
 * Returns the index within this string of the first occurrence
 * of the specified substring, starting at the specified index.
 */
int indexOfStringFrom_String( String *self, String *str, int fromIndex );

/*
 * Returns the index within this string of the last occurrence
 * of the specified character.
 */
int lastIndexOfChar_String( String *self, int ch );

/*
 * Returns the index within this string of the last occurrence
 * of the specified character, searching backward starting
 * at the specified index.
 */
int lastIndexOfCharFrom_String( String *self, int ch, int fromIndex );

/*
 * Returns the index within this string of the rightmost occurrence
 * of the specified substring.
 */
int lastIndexOfString_String( String *self, String *str );

/*
 * Returns the index within this string of the last occurrence
 * of the specified substring.
 */
int lastIndexOfStringFrom_String( String *self, String *str, int fromIndex );

/*
 * Returns the length of this string.
 */
int length_String( String *self );

/*
 * Tests if two string regions are equal.
 */
bool regionMatches_String( String *self
                         , bool ignoreCase
                         , int toffset
                         , String *other
                         , int ooffset
                         , int len
                         );

/*
 * Tests if two string regions are equal.
 */
bool regionMatchesCase_String( String *self
                             , int toffset
                             , String *other
                             , int ooffset
                             , int len
                             );

/*
 * Returns a new string resulting from replacing all occurrences
 * of oldChar in this string with newChar.
 */
String *replace_String( String *self, char oldChar, char newChar );

/*
 * Tests if this string starts with the specified prefix.
 */
bool startsWith_String( String *self, String *prefix );

/*
 * Tests if this string starts with the specified prefix beginning
 * a specified index.
 */
bool startsWithOffset_String( String *self, String *prefix, int toffset );

/*
 * Returns a new string that is a substring of this string.
 */
String *substringFrom_String( String *self, int beginIndex );

/*
 * Returns a new string that is a substring of this string.
 */
String *substring_String( String *self, int beginIndex, int endIndex );

/*
 * Returns C-string representation of this string.
 */
const char* toCharArray_String( String *self );

/*
 * Converts all of the characters in this String to lower case.
 */
String *toLowerCase_String( String *self );

/*
 * Converts all of the characters in this String to upper case.
 */
String *toUpperCase_String( String *self );

/*
 * Removes white space from both ends of this string.
 */
String *trim_String( String *self );

/*
 * Returns the string with formatted data like sprintf.
 */
String *format_String(const char * format, ...);

String *vformat_String(const char * format, va_list args);

/*
 * Returns the string representation of the bool argument.
 */
String *valueOfBool_String( bool b );

/*
 * Returns the string representation of the char argument.
 */
String *valueOfChar_String( char c );

/*
 * Returns the string representation of the short argument.
 */
String *valueOfShort_String( short i );

/*
 * Returns the string representation of the unsigned short argument.
 */
String *valueOfUShort_String( unsigned short i );

/*
 * Returns the string representation of the int argument.
 */
String *valueOfInt_String( int i );

/*
 * Returns the string representation of the unsigned int argument.
 */
String *valueOfUInt_String( unsigned int i );

/*
 * Returns the string representation of the long argument.
 */
String *valueOfLong_String( long l );

/*
 * Returns the string representation of the unsigned long argument.
 */
String *valueOfULong_String( unsigned long l );

/*
 * Returns the string representation of the float argument.
 */
String *valueOfFloat_String( float f );

/*
 * Returns the string representation of the double argument.
 */
String *valueOfDouble_String( double d );

/*
 * Returns the string representation of the long argument.
 */
String *valueOfPtr_String (void *p);

/*
 * Returns the string representation of the Object argument.
 */
String *valueOfObject_String( Object *ref );

/*
 * Returns the string with hex representation
 */
String *valueOfBytes_String( const char* p, int l );

String *XML_encode_String(String *self);

/*
 * Returns hash-code of the String argument.
 */
int hash_String( String *str );

#ifdef __cplusplus
} // extern "C"
#endif

#endif
