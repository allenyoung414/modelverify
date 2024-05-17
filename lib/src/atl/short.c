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
 * File: short.c
 * Description:
 *       This file defines the specification types representing
 *       signed and unsigned short integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/short.h>

#include <atl/string.h>
#include <utils/assertion.h>

#include <stdio.h>

static void init_Short( Short *s, va_list *arg_list )
{
    *s = (short)va_arg( *arg_list, int );
}

static void init_UShort( UShort *s, va_list *arg_list )
{
    *s = (unsigned short)va_arg( *arg_list, unsigned int );
}

static void copy_Short( Short *src, Short *dst )
{
    *dst = *src;
}

static void copy_UShort( UShort *src, UShort *dst )
{
    *dst = *src;
}

static int compare_Short( Short *left, Short *right )
{
    if( *left < *right )
    {
        return -1;
    } else
    if( *left > *right )
    {
        return 1;
    } else {
        return 0;
    }
}

static int compare_UShort( UShort *left, UShort *right )
{
    if( *left < *right )
    {
        return -1;
    } else
    if( *left > *right )
    {
        return 1;
    } else {
        return 0;
    }
}

static String *to_string_Short( Short *s )
{
    char buf[256];

    sprintf( buf, "%d", *s );

    return create_String( buf );
}

static String *to_XML_Short( Short *s )
{
	return to_XML_spec("Short", to_string_Short(s));
}

static String *to_string_UShort( UShort *s )
{
    char buf[256];

    sprintf( buf, "%u", *s );

    return create_String( buf );
}

static String *to_XML_UShort( UShort *us )
{
	return to_XML_spec("UShort", to_string_UShort(us));
}

const Type type_Short
    = { "Short"
      , sizeof(short)
      , &type_Object
      , (Init)init_Short
      , (Copy)copy_Short
      , (Compare)compare_Short
      , (ToString)to_string_Short
      , (ToXML)to_XML_Short
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

const Type type_UShort
    = { "UShort"
      , sizeof(unsigned short)
      , &type_Object
      , (Init)init_UShort
      , (Copy)copy_UShort
      , (Compare)compare_UShort
      , (ToString)to_string_UShort
      , (ToXML)to_XML_UShort
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Short *create_Short( short s )
{
    return create( &type_Short, s );
}

UShort *create_UShort( unsigned short s )
{
    return create( &type_UShort, s );
}

short value_Short( Short *s )
{
    short res;

    CHECK_TYPE_COMPATIBLE( value_Short, &type_Short, s );

    res = *s;
    destroy(s);
    return res;
}

unsigned short value_UShort( UShort *s )
{
    unsigned short res;

    CHECK_TYPE_COMPATIBLE( value_UShort, &type_UShort, s );

    res = *s;
    destroy(s);
    return res;
}
