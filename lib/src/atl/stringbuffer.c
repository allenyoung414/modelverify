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
 * File: stringbuffer.c
 * Description:
 *       This file defines the specification type representing
 *       string buffers.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/array.h>
#include <atl/object.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <atl/stringbuffer.h>
#include <ctype.h>
#include <string.h>

#include <atl/object_int.h>


#define DEFAULT_ALLOCATE_SIZE   128

#define CONTENTS(buf)	( (char*) (buf->header + 1) )
#define LENGTH(buf)		(buf->header->length)


/* Copied from string.c */
typedef struct StringHeader
{	// Fields from ArrayHeader
    int ref_cnt;
    int capacity;
	// Own fields
	int length;
	// The following is a char array. For String, it will be null-terminated.
}
StringHeader;

struct String
{
    StringHeader *header;
};

#define MAGIC_LENGTH	~((~0U) >> 1)	// == (bin)1000...000
/* End copy */


struct StringBuffer
{
  StringHeader *header;
  int   allocateBy;
};

static void init_StringBuffer( StringBuffer* strbuf, va_list *arg_list )
{
int allocateBy = va_arg( *arg_list, int );

  assertion( allocateBy > 0,
             FORMAT( "init_StringBuffer: allocateBy should be greater zero" )
           );

  strbuf->header = create_Array( sizeof(StringHeader), 1, allocateBy );
  strbuf->header->length = 0;
  strbuf->allocateBy = allocateBy;
}

static void copy_StringBuffer( StringBuffer* src, StringBuffer* dst )
{
  dst->allocateBy = src->allocateBy;

  // clone_Array() is not suitable here
  dst->header = create_Array( sizeof(StringHeader), 1, src->header->capacity );
  dst->header->length = src->header->length;
  memcpy( CONTENTS(dst), CONTENTS(src), LENGTH(src) );
}

static int compare_StringBuffer( StringBuffer* left, StringBuffer* right )
{
  if (LENGTH(left) != LENGTH(right))
    return LENGTH(left) - LENGTH(right);
  return memcmp( CONTENTS(left), CONTENTS(right), LENGTH(left) );
}

static String* to_string_StringBuffer( StringBuffer* self )
{
char* cstr;
String* res;
int ref_cnt = header( self )->hard_ref_cnt + header( self )->weak_ref_cnt;

  if (ref_cnt == 1 && self->header->capacity > LENGTH(self))
  {	// Simplified copy
	res = create(&type_String, NULL, MAGIC_LENGTH);
    assertion( res != 0, FORMAT( "to_string_StringBuffer: No memory" ) );

	res->header = self->header;
	CONTENTS(self) [ LENGTH(self) ] = 0;

	// Free unused memory. Hope, this will just shrink, without moving.
	if (res->header->capacity > res->header->length + 1) {
		res->header->capacity = res->header->length + 1;
		res->header = (StringHeader*)realloc(res->header, sizeof(StringHeader) + res->header->capacity);
		assertion( res->header != NULL, FORMAT( "to_string_StringBuffer: No memory" ) );	// Should never fail since this is always shrinking
	}

	self->header = NULL;	// Preserve contents during further destruction

	return res;
  }

  // Copy data
  res = create( &type_String, NULL, LENGTH(self) );	// Will allocate length+1 bytes
  cstr = (char*)toCharArray_String( r(res) );
  memcpy( cstr, CONTENTS(self), LENGTH(self) );
  cstr[ LENGTH(self) ] = '\0';
  return res;
}

static String* to_XML_StringBuffer( StringBuffer* self )
{
	return to_XML_spec("StringBuffer", to_string_StringBuffer(self));
}

static void destroy_StringBuffer( StringBuffer* self )
{
  if (self->header != NULL)	// Was set to NULL by toString()
	destroy_Array( self->header );
}

const Type type_StringBuffer
    = { "StringBuffer"
      , sizeof(StringBuffer)
      , &type_Object
      , (Init)init_StringBuffer
      , (Copy)copy_StringBuffer
      , (Compare)compare_StringBuffer
      , (ToString)to_string_StringBuffer
      , (ToXML)to_XML_StringBuffer
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_StringBuffer
      };

/*
 * Expand the string buffer for at least min bytes.
 */
static void expand_StringBuffer( StringBuffer* self, int min )
{
int extra;

  if (min < self->allocateBy)
    extra = self->allocateBy;
  else
    extra = ((min / self->allocateBy) + 1) * self->allocateBy;

  self->header->capacity += extra;

  self->header = realloc( self->header, sizeof(StringHeader) + self->header->capacity);
  assertion( self->header != NULL, FORMAT( "expand_StringBuffer: No memory" ) );

  destroy( self );
}


/*
 * Create empty StringBuffer.
 */
StringBuffer* create_StringBuffer(void)
{
  return create( &type_StringBuffer, DEFAULT_ALLOCATE_SIZE );
}

/*
 * Create empty StringBuffer with the given allocateBy.
 */
StringBuffer* createCustomized_StringBuffer( int allocateBy )
{
  return create( &type_StringBuffer, allocateBy );
}

/*
 * Empty StringBuffer
 */
void clear_StringBuffer( StringBuffer* self )
{
  LENGTH(self) = 0;
  destroy(self);
}


/*
 * Returns the current size of the string buffer.
 */
int size_StringBuffer( StringBuffer* self )
{
int res;

  CHECK_TYPE_COMPATIBLE( size_StringBuffer, &type_StringBuffer, self );

  res = LENGTH(self);

  destroy( self );

  return res;
}

/*
 * Returns the character at the specified index.
 */
char charAt_StringBuffer( StringBuffer* self, int index )
{
char res;

  CHECK_TYPE_COMPATIBLE( charAt_StringBuffer, &type_StringBuffer, self );

  assertion( 0 <= index && index < LENGTH(self)
           , FORMAT( "charAt_StringBuffer: char index %d is out of bounds [0, %d[" )
           , index
           , LENGTH(self)
           );

  res = CONTENTS(self) [index];

  destroy( self );

  return res;
}

/*
 * Appends the specified char to the end of the string buffer.
 */
void appendChar_StringBuffer( StringBuffer* self, char ch )
{
  CHECK_TYPE_COMPATIBLE( appendChar_StringBuffer, &type_StringBuffer, self );

  if (LENGTH(self) == self->header->capacity) {
    expand_StringBuffer( r(self), 1 );
  }

  CONTENTS(self) [LENGTH(self)] = ch;
  LENGTH(self)++;

  destroy( self );
}

/*
 * Appends the specified null-terminating string
 * to the end of the string buffer.
 */
void append_StringBuffer( StringBuffer* self, const char* str )
{
int length;

  CHECK_TYPE_COMPATIBLE( append_StringBuffer, &type_StringBuffer, self );

  length = strlen( str );

  if (LENGTH(self) + length > self->header->capacity)
   {
    expand_StringBuffer( r(self), LENGTH(self) + length - self->header->capacity );
   }

  memcpy( CONTENTS(self) + LENGTH(self), str, length );
  LENGTH(self) += length;

  destroy( self );
}

/*
 * Appends the specified char array to the end of the string buffer.
 */
void appendCharArray_StringBuffer( StringBuffer* self, const char* arr, int size )
{
  CHECK_TYPE_COMPATIBLE( appendCharArray_StringBuffer, &type_StringBuffer, self );

  if (LENGTH(self) + size > self->header->capacity) {
    expand_StringBuffer( r(self), LENGTH(self) + size - self->header->capacity );
  }

  memcpy( CONTENTS(self) + LENGTH(self), arr, size );
  LENGTH(self) += size;

  destroy( self );
}

/*
 * Appends the specified string to the end of the string buffer.
 */
void appendString_StringBuffer( StringBuffer* self, String* str )
{
int length;

  CHECK_TYPE_COMPATIBLE( appendString_StringBuffer, &type_StringBuffer, self );
  CHECK_TYPE_COMPATIBLE( appendString_StringBuffer, &type_String, str );

  length = length_String( r(str) );

  if (LENGTH(self) + length > self->header->capacity) {
    expand_StringBuffer( r(self), LENGTH(self) + length - self->header->capacity );
  }

  memcpy( CONTENTS(self) + LENGTH(self), toCharArray_String( r(str) ), length );
  LENGTH(self) += length;

  destroy( self );
  destroy( str );
}

/*
 * Appends the specified string buffer to the end of the string buffer.
 */
void appendStringBuffer_StringBuffer( StringBuffer* self, StringBuffer* strbuf )
{
  CHECK_TYPE_COMPATIBLE( appendStringBuffer_StringBuffer, &type_StringBuffer, self );
  CHECK_TYPE_COMPATIBLE( appendStringBuffer_StringBuffer, &type_StringBuffer, strbuf );

  if (LENGTH(self) + LENGTH(strbuf) > self->header->capacity) {
    expand_StringBuffer( r(self), LENGTH(self) + LENGTH(strbuf) - self->header->capacity );
  }

  memcpy( CONTENTS(self) + LENGTH(self), CONTENTS(strbuf), LENGTH(strbuf) );
  LENGTH(self) += LENGTH(strbuf);

  destroy( self );
  destroy( strbuf );
}


/*
 * Appends the string with formatted data like sprintf
 * to the end of the string buffer.
 */
void appendFormat_StringBuffer( StringBuffer* self, const char* format, ... )
{
va_list args;

    va_start( args, format );
    vappendFormat_StringBuffer( r(self), format, args );
    va_end( args );
    destroy( self );
}

#ifdef WIN32

/*
 * Appends the string with formatted data like sprintf
 * to the end of the string buffer.
 */
void vappendFormat_StringBuffer( StringBuffer* self, const char* format, va_list args )
{
int i;
int res_vsnprintf;

  CHECK_TYPE_COMPATIBLE( appendFormat_StringBuffer, &type_StringBuffer, self );

  for( i = 1; ; i++ )
  {
    res_vsnprintf = _vsnprintf( CONTENTS(self) + LENGTH(self)
							  , self->header->capacity - LENGTH(self)
							  , format
							  , args );
    if ( res_vsnprintf >= 0 )
		break;

    expand_StringBuffer( r(self), i * self->allocateBy );
  }
  LENGTH(self) += res_vsnprintf;

  destroy( self );
}

#else

/*
 * Appends the string with formatted data like sprintf
 * to the end of the string buffer.
 */
void vappendFormat_StringBuffer( StringBuffer* self, const char* format, va_list args )
{
int added_len;
va_list args_copy;

  CHECK_TYPE_COMPATIBLE( appendFormat_StringBuffer, &type_StringBuffer, self );

  va_copy( args_copy, args );

  added_len = vsnprintf( CONTENTS(self) + LENGTH(self)
					   , self->header->capacity - LENGTH(self)
					   , format
					   , args );
  va_end(args);

  if ( LENGTH(self) + added_len + 1 > self->header->capacity )
  {
    expand_StringBuffer( r(self), LENGTH(self) + added_len + 1 - self->header->capacity );
    added_len = vsnprintf( CONTENTS(self) + LENGTH(self)
						 , self->header->capacity - LENGTH(self)
						 , format
						 , args_copy );
  }
  LENGTH(self) += added_len;

  va_end( args_copy );
  destroy( self );
}
#endif


/*
 * Tests if this string buffer ends with the specified suffix.
 */
bool endsWith_StringBuffer( StringBuffer* self, String* suffix )
{
const char *suffix_cstr;
int suffix_len;
bool res;

  CHECK_TYPE_COMPATIBLE( endsWith_StringBuffer, &type_StringBuffer, self   );
  CHECK_TYPE_COMPATIBLE( endsWith_StringBuffer, &type_String, suffix );

  suffix_cstr = toCharArray_String( r( suffix ) );
  suffix_len  = length_String( r( suffix ) );

  if( LENGTH(self) >= suffix_len )
  {
    res = memcmp( CONTENTS(self) + LENGTH(self) - suffix_len, suffix_cstr, suffix_len )
			? false : true;
  } else {
    res = false;
  }

  destroy( self );
  destroy( suffix );

  return res;
}

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified character.
 */
int indexOfChar_StringBuffer( StringBuffer* self, int ch )
{
  return indexOfCharFrom_StringBuffer( self, ch, 0 );
}

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified character, starting the search at the specified index.
 */
int indexOfCharFrom_StringBuffer( StringBuffer* self, int ch, int fromIndex )
{
int res = - 1;

  CHECK_TYPE_COMPATIBLE( indexOfCharFrom_StringBuffer, &type_StringBuffer, self );

  if ( fromIndex < 0 ) fromIndex = 0;

  if ( ch != 0 && fromIndex < LENGTH(self) )
  {
    const char* pos = memchr( CONTENTS(self) + fromIndex, ch, LENGTH(self) - fromIndex );
    if (pos != NULL) {
      res = pos - CONTENTS(self);
    }
  }

  destroy( self );

  return res;
}

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified substring.
 */
int indexOfString_StringBuffer( StringBuffer* self, String* str )
{
  return indexOfStringFrom_StringBuffer( self, str, 0 );
}

/*
 * Returns the index within this string buffer of the first occurrence
 * of the specified substring, starting at the specified index.
 */
int indexOfStringFrom_StringBuffer( StringBuffer* self, String* str, int fromIndex )
{
int i,str_length;
int res = -1;

  CHECK_TYPE_COMPATIBLE( indexOfStringFrom_StringBuffer, &type_StringBuffer, self );
  CHECK_TYPE_COMPATIBLE( indexOfStringFrom_StringBuffer, &type_String, str  );

  if ( fromIndex < 0 ) fromIndex = 0;

  str_length = length_String( r(str) );

  if ( fromIndex <= LENGTH(self) - str_length )
  {
    const char* str_cstr = toCharArray_String( r(str) );

    for( i = fromIndex; i <= LENGTH(self) - str_length; i++ )
    {
      if (memcmp( CONTENTS(self) + i, str_cstr, str_length ) == 0)
      {
        res = i;
        break;
      }
    }
  }

  destroy( self );
  destroy( str );

  return res;
}

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified character.
 */
int lastIndexOfChar_StringBuffer( StringBuffer* self, int ch )
{
  CHECK_TYPE_COMPATIBLE( lastIndexOfChar_StringBuffer, &type_StringBuffer, self );

  return lastIndexOfCharFrom_StringBuffer( self, ch, LENGTH(self) );
}

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified character, searching backward starting
 * at the specified index.
 */
int lastIndexOfCharFrom_StringBuffer( StringBuffer* self, int ch, int fromIndex )
{
int res = - 1;

  CHECK_TYPE_COMPATIBLE( lastIndexOfCharFrom_StringBuffer, &type_StringBuffer, self );

  if ( ch != 0 && 0 <= fromIndex )
  {
    int i;

    if (fromIndex >= LENGTH(self))
		fromIndex = LENGTH(self) - 1;

    for( i = fromIndex; 0 <= i; i-- )
    {
      if (CONTENTS(self) [i] == ch)
      {
        res = i;
        break;
      }
    }
  }

  destroy( self );

  return res;
}

/*
 * Returns the index within this string buffer of the rightmost occurrence
 * of the specified substring.
 */
int lastIndexOfString_StringBuffer( StringBuffer* self, String* str )
{
  CHECK_TYPE_COMPATIBLE( lastIndexOfString_StringBuffer, &type_StringBuffer, self );

  return lastIndexOfStringFrom_StringBuffer( self, str, LENGTH(self) );
}

/*
 * Returns the index within this string buffer of the last occurrence
 * of the specified substring.
 */
int lastIndexOfStringFrom_StringBuffer( StringBuffer* self, String* str, int fromIndex )
{
int res = -1;
int i;
int str_len;

  CHECK_TYPE_COMPATIBLE( lastIndexOfStringFrom_StringBuffer, &type_StringBuffer, self );
  CHECK_TYPE_COMPATIBLE( lastIndexOfStringFrom_StringBuffer, &type_String, str  );

  str_len = length_String( r(str) );

  if (str_len <= LENGTH(self)) {
    if ( fromIndex > LENGTH(self) - str_len )
	    fromIndex = LENGTH(self) - str_len;

    for( i = fromIndex; 0 <= i; i-- ) {
      if ( startsWithOffset_StringBuffer( r(self), r(str), i ) ) {
        res = i;
        break;
	  }
    }
  }

  destroy( self );
  destroy( str );

  return res;
}

/*
 * Replaces all occurrences of oldChar in the string buffer with newChar.
 */
void replace_StringBuffer( StringBuffer* self, char oldChar, char newChar )
{
int i;
char *p;

  CHECK_TYPE_COMPATIBLE( replace_StringBuffer, &type_StringBuffer, self );

  p = CONTENTS(self);
  for( i = 0; i < LENGTH(self); i++ )
  {
    if (p[i] == oldChar)
      p[i] = newChar;
  }

  destroy( self );
}

/*
 * Tests if this string buffer starts with the specified prefix.
 */
bool startsWith_StringBuffer( StringBuffer* self, String* prefix )
{
    return startsWithOffset_StringBuffer( self, prefix, 0 );
}

/*
 * Tests if this string buffer starts with the specified prefix beginning
 * a specified index.
 */
bool startsWithOffset_StringBuffer( StringBuffer* self, String* prefix, int toffset )
{
int length;
bool res = false;

  CHECK_TYPE_COMPATIBLE( startsWithOffset_StringBuffer, &type_StringBuffer, self   );
  CHECK_TYPE_COMPATIBLE( startsWithOffset_StringBuffer, &type_String, prefix );

  length = length_String( r( prefix ) );

  if ( 0 <= toffset && toffset <= LENGTH(self) - length )
  {
    res = memcmp(  CONTENTS(self) + toffset
                 , toCharArray_String( r( prefix ) )
                 , length
                )
          ? false : true;
  }

  destroy( self );
  destroy( prefix );

  return res;
}

/*
 * Returns a new string that is a substring of the string buffer.
 */
String* substringFrom_StringBuffer( StringBuffer* self, int beginIndex )
{
String* res;
char* cstr;

  CHECK_TYPE_COMPATIBLE( substringFrom_StringBuffer, &type_StringBuffer, self );

  assertion( 0 <= beginIndex && beginIndex <= LENGTH(self)
             , FORMAT( "substringFrom_StringBuffer: beginIndex %d is out of bounds [0,%d]" )
             , beginIndex
             , LENGTH(self)
             );

  res = create( &type_String, NULL, LENGTH(self) - beginIndex );
  cstr = (char*)toCharArray_String( r(res) );
  memcpy( cstr, CONTENTS(self) + beginIndex, LENGTH(self) - beginIndex );
  cstr[LENGTH(self) - beginIndex] = '\0';

  destroy( self );

  return res;
}

/*
 * Returns a new string that is a substring of the string buffer.
 */
String* substring_StringBuffer( StringBuffer* self, int beginIndex, int endIndex )
{
String* res;
char* cstr;

  CHECK_TYPE_COMPATIBLE( substring_StringBuffer, &type_StringBuffer, self );

  assertion(    0 <= beginIndex
             && beginIndex <= endIndex
             && endIndex <= LENGTH(self)
           , FORMAT( "substring_String: beginIndex %d or endIndex %d are out of bounds [0,%d]"
                     " or endIndex < beginIndex"
                   )
           , beginIndex
           , endIndex
           , LENGTH(self)
           );

  res = create( &type_String, NULL, endIndex - beginIndex );
  cstr = (char*)toCharArray_String( r(res) );
  memcpy( cstr, CONTENTS(self) + beginIndex, endIndex - beginIndex );
  cstr[endIndex - beginIndex] = '\0';

  destroy( self );

  return res;
}


