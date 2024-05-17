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
 * File: string.c
 * Description:
 *       This file defines the specification type representing
 *       null-terminated strings.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/object.h>
#include <atl/array.h>
#include <atl/stringbuffer.h>
#include <utils/assertion.h>
#include <utils/boolean.h>

#include <string.h>
#include <ctype.h>

#include <atl/object_int.h>

typedef struct StringHeader
{	// Fields from ArrayHeader
    int ref_cnt;
    int capacity;
	// Own fields
	int length;
	// The following is a null-terminated char array
}
StringHeader;

struct String
{
    StringHeader *header;
};

#define MAGIC_LENGTH	~((~0U) >> 1)	// == (bin)1000...000

#define header_String( str ) (str->header)


static void init_array_String( String *str, int len )
{
    header_String(str) = create_Array( sizeof(StringHeader), 1, len + 1 );
	header_String(str)->length = len;
}

static void init_String( String *str, va_list *arg_list )
{
const char* cstr = va_arg( *arg_list, const char *);

    if ( cstr == NULL )
     {
        int len = va_arg( *arg_list, int );

		// This length value is flag used by to_string_StringBuffer()
		if (len == MAGIC_LENGTH)
			return;

        assertion( len >= 0, FORMAT( "init_String: C-string is NULL and length is less than zero" ) );

        init_array_String( str, len );

        ((char*)(header_String( str ) + 1))[0] = 0;
     }
    else
     {
        init_array_String( str, strlen( cstr ) );

        strcpy( (char*)(header_String( str ) + 1), cstr );
     }
}

static void copy_String( String *src, String *dst )
{
    clone_Array( dst->header = src->header );
}

static int compare_String( String *left, String *right )
{
    return strcmp( (char*)( header_String( left ) + 1 )
                 , (char*)( header_String( right ) + 1 )
                 );
}

static String *to_string_String( String *str )
{
    return r( str );
}

static String *to_XML_String( String *str )
{
    return to_XML_spec("String", r(str) );
}

static void destroy_String( String *str )
{
    destroy_Array( header_String( str ) );
}

const Type type_String
    = { "String"
      , sizeof(void*)
      , &type_Object
      , (Init)init_String
      , (Copy)copy_String
      , (Compare)compare_String
      , (ToString)to_string_String
      , (ToXML)to_XML_String
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_String
      };

String *create_String( const char *cstr )
{
    assertion( cstr != 0, FORMAT( "create_String: C-string can't be NULL" ) );

    return create( &type_String, cstr );
}

static String *createLen_String( int len )
{
    return create( &type_String, NULL, len );
}

char charAt_String( String *self, int index )
{
    char res;

    CHECK_TYPE_COMPATIBLE( charAt_String, &type_String, self );

    assertion( 0 <= index && index < length_String( r( self ) )
             , FORMAT( "charAt_String: char index %d is out of bounds [0, %d[" )
             , index
             , length_String( r( self ) )
             );

    res = ((char*)(header_String( self )+1))[index];

    destroy( self );

    return res;
}

String *concat_String( String *self, String *str )
{
    int self_len;
    int len;
    String *res;
    char *cstr;

    CHECK_TYPE_COMPATIBLE( concat_String, &type_String, self );
    CHECK_TYPE_COMPATIBLE( concat_String, &type_String, str  );

    self_len = length_String( r( self ) );
    len = self_len + length_String( r( str ) );

    res = createLen_String( len );
    cstr = (char*)toCharArray_String( r( res ) );

    strcpy( cstr
          , toCharArray_String( r( self ) )
          );

    strcpy( cstr + self_len
          , toCharArray_String( r( str ) )
          );

    destroy( self );
    destroy( str );

    return res;
}

bool endsWith_String( String *self, String *suffix )
{
    const char *self_cstr;
    const char *suffix_cstr;
    int self_len;
    int suffix_len;
    bool res;

    CHECK_TYPE_COMPATIBLE( endsWith_String, &type_String, self   );
    CHECK_TYPE_COMPATIBLE( endsWith_String, &type_String, suffix );

    self_cstr   = toCharArray_String( r( self   ) );
    suffix_cstr = toCharArray_String( r( suffix ) );

    self_len   = length_String( r( self   ) );
    suffix_len = length_String( r( suffix ) );

    if( self_len >= suffix_len )
    {
        res =   strncmp( self_cstr + self_len - suffix_len
                       , suffix_cstr
                       , suffix_len
                       )
              ?   false
                : true;
    } else {
        res = false;
    }

    destroy( self );
    destroy( suffix );

    return res;
}

int indexOfChar_String( String *self, int ch )
{
    return indexOfCharFrom_String( self, ch, 0 );
}

int indexOfCharFrom_String( String *self, int ch, int fromIndex )
{
    int res = - 1;

    CHECK_TYPE_COMPATIBLE( indexOfCharFrom_String, &type_String, self );

    if( fromIndex < 0 ) fromIndex = 0;

    if( ch != 0 && fromIndex < length_String( r( self ) ) )
    {
        const char *self_cstr = toCharArray_String( r( self ) );
        const char *pos = strchr( self_cstr + fromIndex, ch );

        if( pos )
        {
            res = pos - self_cstr;
        }
    }

    destroy( self );

    return res;
}

int indexOfString_String( String *self, String *str )
{
    return indexOfStringFrom_String( self, str, 0 );
}

int indexOfStringFrom_String( String *self, String *str, int fromIndex )
{
    int res = -1;

    CHECK_TYPE_COMPATIBLE( indexOfStringFrom_String, &type_String, self );
    CHECK_TYPE_COMPATIBLE( indexOfStringFrom_String, &type_String, str  );

    if( fromIndex < 0 ) fromIndex = 0;

    if( fromIndex <= length_String( r( self ) ) )
    {
        const char *self_cstr = toCharArray_String( r( self ) );
        const char *str_cstr  = toCharArray_String( r( str  ) );
        const char *pos = strstr( self_cstr + fromIndex, str_cstr );

        if( pos )
        {
            res = pos - self_cstr;
        }
    }

    destroy( self );
    destroy( str );

    return res;
}

int lastIndexOfChar_String( String *self, int ch )
{
    return lastIndexOfCharFrom_String( self
                                     , ch
                                     , length_String( r( self ) )
                                     );
}

int lastIndexOfCharFrom_String( String *self, int ch, int fromIndex )
{
    int res = - 1;

    CHECK_TYPE_COMPATIBLE( lastIndexOfCharFrom_String, &type_String, self );

    if( ch != 0 && 0 <= fromIndex )
    {
        int i;
        int len = length_String( r( self ) ) - 1;

        if( fromIndex > len ) fromIndex = len;

        for( i = fromIndex; 0 <= i; i-- )
        {
            if( charAt_String( r( self ), i ) == ch )
            {
                res = i;
                break;
            }
        }
    }

    destroy( self );

    return res;
}

int lastIndexOfString_String( String *self, String *str )
{
    return lastIndexOfStringFrom_String( self
                                       , str
                                       , length_String( r( self ) )
                                       );
}

int lastIndexOfStringFrom_String( String *self, String *str, int fromIndex )
{
    int res = -1;
    int i;
    int len;

    CHECK_TYPE_COMPATIBLE( lastIndexOfStringFrom_String, &type_String, self );
    CHECK_TYPE_COMPATIBLE( lastIndexOfStringFrom_String, &type_String, str  );

    len = length_String( r( self ) );
    if( fromIndex >= len ) fromIndex = len;

    for( i = fromIndex; 0 <= i; i-- )
    {
        if( startsWithOffset_String( r( self ), r( str ), i ) )
        {
            res = i;
            break;
        }
    }

    destroy( self );
    destroy( str );

    return res;
}

int length_String( String *self )
{
    int res;

    CHECK_TYPE_COMPATIBLE( length_String, &type_String, self );

    res = header_String( self )->length;

    destroy( self );

    return res;
}

bool regionMatches_String( String *self
                         , bool ignoreCase
                         , int toffset
                         , String *other
                         , int ooffset
                         , int len
                         )
{
bool res = false;
const char *self_cstr;
const char *other_cstr;

  CHECK_TYPE_COMPATIBLE( regionMatches_String, &type_String, self  );
  CHECK_TYPE_COMPATIBLE( regionMatches_String, &type_String, other );
  assertion( len >= 0
           , FORMAT( "regionMatches_String: argument length %d is less than zero" )
           , len
           );

  self_cstr  = toCharArray_String( r( self  ) );
  other_cstr = toCharArray_String( r( other ) );

  if(    0 <= toffset
      && toffset <= length_String( r( self ) )
      && 0 <= ooffset
      && ooffset <= length_String( r( other ) )
    )
   {
    if( ignoreCase )
     {
/* CTesK Source Start */
#ifdef WIN32
      res =   strnicmp( self_cstr + toffset, other_cstr + ooffset, len )
#else
/* CTesK Source End */
      res =   strncasecmp( self_cstr + toffset, other_cstr + ooffset, len )
/* CTesK Source Start */
#endif
/* CTesK Source End */
            ?   false
              : true;
     }
    else
     {
      res =   strncmp( self_cstr + toffset, other_cstr + ooffset, len )
            ?   false
              : true;
     }
   }

  destroy( self );
  destroy( other );

  return res;
}

bool regionMatchesCase_String( String *self
                             , int toffset
                             , String *other
                             , int ooffset
                             , int len
                             )
{
  return regionMatches_String( self, false, toffset, other, ooffset, len );
}

String *replace_String( String *self, char oldChar, char newChar )
{
    String *res;
    char *res_cstr;
    const char *self_cstr;
    int i;
    int len;

    CHECK_TYPE_COMPATIBLE( replace_String, &type_String, self );

    res       = createLen_String( length_String( r( self ) ) );
    res_cstr  = (char*)toCharArray_String( r( res  ) );
    self_cstr = toCharArray_String( r( self ) );
    len       = length_String( r( self ) );

    for( i = 0; i < len; i++ )
    {
        char ch = self_cstr[i];
        res_cstr[i] = (ch == oldChar) ? newChar : ch;
    }
    res_cstr[len] = 0;

    destroy( self );

    return res;
}

bool startsWith_String( String *self, String *prefix )
{
    return startsWithOffset_String( self, prefix, 0 );
}

bool startsWithOffset_String( String *self, String *prefix, int toffset )
{
    bool res = false;

    CHECK_TYPE_COMPATIBLE( startsWithOffset_String, &type_String, self   );
    CHECK_TYPE_COMPATIBLE( startsWithOffset_String, &type_String, prefix );

    if( 0 <= toffset && toffset <= length_String( r( self ) ) )
    {
        const char *self_cstr   = toCharArray_String( r( self   ) );
        const char *prefix_cstr = toCharArray_String( r( prefix ) );

        res =   strncmp( self_cstr + toffset
                       , prefix_cstr
                       , length_String( r( prefix ) )
                       )
              ?   false
                : true;
    }

    destroy( self );
    destroy( prefix );

    return res;
}

String *substringFrom_String( String *self, int beginIndex )
{
    String *res;

    CHECK_TYPE_COMPATIBLE( substringFrom_String, &type_String, self );

    assertion( 0 <= beginIndex && beginIndex <= length_String( r( self ) )
             , FORMAT( "substringFrom_String: beginIndex %d is out of bounds [0,%d]" )
             , beginIndex
             , length_String( r( self ) )
             );

    res = create_String( toCharArray_String( r( self ) ) + beginIndex );

    destroy( self );

    return res;
}

String *substring_String( String *self, int beginIndex, int endIndex )
{
    String *res;
    char *res_cstr;
    const char *self_cstr;

    CHECK_TYPE_COMPATIBLE( substring_String, &type_String, self );

    assertion(    0 <= beginIndex
               && beginIndex <= endIndex
               && endIndex <= length_String( r( self ) )
             , FORMAT( "substring_String: beginIndex %d or endIndex %d are out of bounds [0,%d]"
                       " or endIndex < beginIndex"
                     )
             , beginIndex
             , endIndex
             , length_String( r( self ) )
             );

    res = createLen_String( endIndex - beginIndex );

    res_cstr  = (char*)toCharArray_String( r( res  ) );
    self_cstr = toCharArray_String( r( self ) );

    strncpy( res_cstr, self_cstr + beginIndex, endIndex - beginIndex );
    res_cstr[endIndex - beginIndex] = '\0';

    destroy( self );

    return res;
}

const char *toCharArray_String( String *self )
{
    const char *res;

    CHECK_TYPE_COMPATIBLE( toCharArray_String, &type_String, self );

    assertion( header( self )->hard_ref_cnt + header( self )->weak_ref_cnt > 1,
               "toCharArray_String: self has no other references, pointer to internal buffer will be invalid"
             );

    res = (const char*)(header_String( self ) + 1);

    destroy( self );

    return res;
}

String *toLowerCase_String( String *self )
{
    String *res;
    int i;
    int l;
    char *res_cstr;

    CHECK_TYPE_COMPATIBLE( toLowerCase_String, &type_String, self );

    res = create_String( toCharArray_String( r( self ) ) );
    l = length_String( r( res ) );
    res_cstr = (char*)toCharArray_String( r( res ) );

    for( i = 0; i < l; i++ )
    {
        res_cstr[i] = tolower( res_cstr[i] );
    }

    destroy( self );

    return res;
}

String *toUpperCase_String( String *self )
{
    String *res;
    int i;
    int l;
    char *res_cstr;

    CHECK_TYPE_COMPATIBLE( toUpperCase_String, &type_String, self );

    res = create_String( toCharArray_String( r( self ) ) );
    l = length_String( r( res ) );
    res_cstr = (char*)toCharArray_String( r( res ) );

    for( i = 0; i < l; i++ )
    {
        res_cstr[i] = toupper( res_cstr[i] );
    }

    destroy( self );

    return res;
}

String *trim_String( String *self )
{
    int len;
    int beginIndex;
    int endIndex;

    CHECK_TYPE_COMPATIBLE( trim_String, &type_String, self );

    len = length_String( r( self ) );

    for( beginIndex = 0; beginIndex < len; beginIndex++ )
    {
        if( !isspace( charAt_String( r( self ), beginIndex ) ) ) break;
    }

    if( beginIndex == len )
    {
        destroy( self );
        return create_String( "" );
    }

    for( endIndex = len - 1; beginIndex < endIndex; endIndex-- )
    {
        if( !isspace( charAt_String( r( self ), endIndex ) ) ) break;
    }

    return substring_String( self, beginIndex, endIndex + 1 );
}

#define FORMAT_CSTRING_LEN (1024)
/* When format_cstring overflows increase it at
 * (1 + 1/FORMAT_CSTRING_DIV) times
 * */
#define FORMAT_CSTRING_DIV 5

String *format_String(const char * format, ...) {
  String * s;
  va_list args;
  va_start( args, format );
  s = vformat_String( format, args );
  va_end( args );
  return s;
}

#ifdef WIN32

String *vformat_String( const char * format, va_list args )
{
  void * p;
  char * format_cstring = NULL;
  size_t format_cstring_len = FORMAT_CSTRING_LEN;
  int num_written = 0;
  String * res = NULL;

  format_cstring=malloc( format_cstring_len );
  assertion(format_cstring!=NULL,
            FORMAT("format_String: Failed to allocate %d bytes"),
            format_cstring_len);

  while( 1 )
  {
      num_written = _vsnprintf(format_cstring, format_cstring_len, format, args);

      if( num_written >= 0 )
          break;

      format_cstring_len += (format_cstring_len/FORMAT_CSTRING_DIV);
      p = realloc(format_cstring, format_cstring_len);
      assertion(p!=NULL,
             FORMAT("format_String: Failed to allocate %d bytes"),
             format_cstring_len);
      format_cstring = (char *)p;
  }

  res = create_String( format_cstring );
  free( format_cstring );
  return res;
}

#else

String *vformat_String(const char * format, va_list args) {
  void * p;
  char * format_cstring = NULL;
  int size = FORMAT_CSTRING_LEN;
  int total_size = 0;
  String * res = NULL;

  va_list args_copy;
  va_copy( args_copy, args );

  format_cstring = malloc( size );

  assertion( format_cstring != NULL
           , FORMAT( "format_String: Failed to allocate %d bytes" )
           , size
           );

  total_size = vsnprintf( format_cstring, size, format, args );

  if(total_size < 0)
    return NULL;

  if( total_size > size )
  {
    size = total_size + 1;

    p = realloc( format_cstring, size );
    assertion( p != NULL
             , FORMAT( "format_String: Failed to allocate %d bytes" )
             , size
             );

    format_cstring = (char *)p;

    total_size = vsnprintf( format_cstring, size, format, args_copy );
    if( total_size < 0 )
        return NULL;
  }
  va_end( args_copy );

  res = create_String( format_cstring );
  free( format_cstring );
  return res;
}

#endif

String *valueOfBool_String( bool b )
{
    if( b ) {
        return create_String( "true" );
    } else {
        return create_String( "false" );
    }
}

String *valueOfChar_String( char c )
{
    char s[2];

    s[0] = c;
    s[1] = 0;

    return create_String( s );
}

String *valueOfDouble_String( double d )
{
    char buf[384];
    sprintf( buf, "%f", d );
    return create_String( buf );
}

String *valueOfFloat_String( float f )
{
    char buf[255];
    sprintf( buf, "%f", f );
    return create_String( buf );
}

String *valueOfInt_String( int i )
{
    char buf[255];
    sprintf( buf, "%d", i );
    return create_String( buf );
}

String *valueOfUInt_String( unsigned int i )
{
    char buf[255];
    sprintf( buf, "%u", i );
    return create_String( buf );
}

String *valueOfShort_String( short i )
{
    char buf[255];
    sprintf( buf, "%hd", i );
    return create_String( buf );
}

String *valueOfUShort_String( unsigned short i )
{
    char buf[255];
    sprintf( buf, "%hu", i );
    return create_String( buf );
}

String *valueOfLong_String( long l )
{
    char buf[255];
    sprintf( buf, "%ld", l );
    return create_String( buf );
}

String *valueOfULong_String( unsigned long l )
{
    char buf[255];
    sprintf( buf, "%lu", l );
    return create_String( buf );
}

String *valueOfPtr_String( void *p )
{
    char buf[255];
    sprintf( buf, "%p", p );
    return create_String( buf );
}

String *valueOfObject_String( Object *ref )
{
    return toString( ref );
}

String *valueOfBytes_String( const char *bytes, int l )
{
    String *res;
    char *cstr;
    int i;
    int l3 = l*3;

    if( l <= 0 ) l3 = 1;

    res = createLen_String( l3 + 1 );
    cstr = (char*)toCharArray_String( r( res ) );

    cstr[0] = '[';

    for( i = 0; i < l; i++ )
    {
        sprintf( cstr + i*3 + 1, "%02X ", (unsigned char)bytes[i] );
    }

    cstr[l3] = ']';
    cstr[l3+1] = '\0'; /* needed when l<=0 */

    return res;
}

String *XML_encode_String(String *self)
{
StringBuffer *buff = create_StringBuffer();
int i;
const char *array = toCharArray_String( r(self) );
int length = length_String( r(self) );
char buf2[16];
	for(i = 0; i < length; i++)
	{
		unsigned char c = array[i];
		switch (c)
		{
		case '<':
			append_StringBuffer(r(buff), "&lt;");
			break;
		case '>':
			append_StringBuffer(r(buff), "&gt;");
			break;
		case '&':
			append_StringBuffer(r(buff), "&amp;");
			break;
		case '"':
			append_StringBuffer(r(buff), "&quot;");
			break;
		case '\n':
			append_StringBuffer(r(buff), "\\n");
			break;
		default:
			if (c < 32 || c > 127) {
				sprintf(buf2, "&#x%02X;", (int)c);
				append_StringBuffer(r(buff), buf2);
			} else
				appendChar_StringBuffer(r(buff), c);
			break;
		}
	}

	destroy(self);
	return toString(buff);
}

int hash_String( String *str )
{
    const unsigned char *s = toCharArray_String( r(str) );
    int hash = 0, l = length_String( r(str) );
    int i;
    for( i = 0; i < l; i++ )
    {
        hash = 31*hash + s[i];
    }
    destroy(str);
    return hash;
}
