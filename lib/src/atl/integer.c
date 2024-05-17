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
 * File: integer.c
 * Description:
 *       This file defines the specification types representing
 *       signed and unsigned integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/integer.h>
#include <atl/string.h>
#include <ts/convex.h>
#include <utils/assertion.h>

#include <stdio.h>


static void init_Integer( Integer *i, va_list *arg_list )
{
    *i = va_arg( *arg_list, int );
}

static void init_UInteger( UInteger *i, va_list *arg_list )
{
    *i = va_arg( *arg_list, unsigned int );
}

static void copy_Integer( Integer *src, Integer *dst )
{
    *dst = *src;
}

static void copy_UInteger( UInteger *src, UInteger *dst )
{
    *dst = *src;
}

static int compare_Integer( Integer *left, Integer *right )
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

static int compare_UInteger( UInteger *left, UInteger *right )
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

static String *to_string_Integer( Integer *i )
{
    char buf[256];

    sprintf( buf, "%d", *i );

    return create_String( buf );
}

static String *to_XML_Integer( Integer *i )
{
	return to_XML_spec("Integer", to_string_Integer(i));
}

static String *to_string_UInteger( UInteger *i )
{
    char buf[256];

    sprintf( buf, "%u", *i );

    return create_String( buf );
}

static String *to_XML_UInteger( UInteger *ui )
{
	return to_XML_spec("UInteger", to_string_UInteger(ui));
}

const Type type_Integer
    = { "Integer"
      , sizeof(int)
      , &type_Object
      , (Init)init_Integer
      , (Copy)copy_Integer
      , (Compare)compare_Integer
      , (ToString)to_string_Integer
      , (ToXML)to_XML_Integer
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

const Type type_UInteger
    = { "UInteger"
      , sizeof(unsigned int)
      , &type_Object
      , (Init)init_UInteger
      , (Copy)copy_UInteger
      , (Compare)compare_UInteger
      , (ToString)to_string_UInteger
      , (ToXML)to_XML_UInteger
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Integer *create_Integer( int i )
{
    return create( &type_Integer, i );
}

UInteger *create_UInteger( unsigned int i )
{
    return create( &type_UInteger, i );
}

int value_Integer( Integer *i )
{
    int res;

    CHECK_TYPE_COMPATIBLE( value_Integer, &type_Integer, i );

    res = *i;
    destroy(i);
    return res;
}

unsigned int value_UInteger( UInteger *i )
{
    unsigned int res;

    CHECK_TYPE_COMPATIBLE( value_UInteger, &type_UInteger, i );

    res = *i;
    destroy(i);
    return res;
}
