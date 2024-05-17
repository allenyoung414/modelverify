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
 * File: long.c
 * Description:
 *       This file defines the specification types representing
 *       signed and unsigned long integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/long.h>

#include <atl/string.h>
#include <utils/assertion.h>

#include <stdio.h>

static void init_Long( Long *l, va_list *arg_list )
{
    *l = va_arg( *arg_list, long );
}

static void init_ULong( ULong *l, va_list *arg_list )
{
    *l = va_arg( *arg_list, unsigned long );
}

static void copy_Long( Long *src, Long *dst )
{
    *dst = *src;
}

static void copy_ULong( ULong *src, ULong *dst )
{
    *dst = *src;
}

static int compare_Long( Long *left, Long *right )
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

static int compare_ULong( ULong *left, ULong *right )
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

static String *to_string_Long( Long *l )
{
    char buf[256];

    sprintf( buf, "%ld", *l );

    return create_String( buf );
}

static String *to_XML_Long( Long *l )
{
	return to_XML_spec("Long", to_string_Long(l));
}

static String *to_string_ULong( ULong *l )
{
    char buf[256];

    sprintf( buf, "%lu", *l );

    return create_String( buf );
}

static String *to_XML_ULong( ULong *ul )
{
	return to_XML_spec("ULong", to_string_ULong(ul));
}

const Type type_Long
    = { "Long"
      , sizeof(long)
      , &type_Object
      , (Init)init_Long
      , (Copy)copy_Long
      , (Compare)compare_Long
      , (ToString)to_string_Long
      , (ToXML)to_XML_Long
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

const Type type_ULong
    = { "ULong"
      , sizeof(unsigned long)
      , &type_Object
      , (Init)init_ULong
      , (Copy)copy_ULong
      , (Compare)compare_ULong
      , (ToString)to_string_ULong
      , (ToXML)to_XML_ULong
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Long *create_Long( long l )
{
    return create( &type_Long, l );
}

ULong *create_ULong( unsigned long l )
{
    return create( &type_ULong, l );
}

long value_Long( Long *l )
{
    long res = *l;

    CHECK_TYPE_COMPATIBLE( value_Long, &type_Long, l );

    res = *l;
    destroy(l);
    return res;
}

unsigned long value_ULong( ULong *l )
{
    unsigned long res = *l;

    CHECK_TYPE_COMPATIBLE( value_ULong, &type_ULong, l );

    res = *l;
    destroy(l);
    return res;
}
