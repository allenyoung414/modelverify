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
 * File: char.c
 * Description:
 *       This file defines the specification type representing characters.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/char.h>
#include <atl/string.h>
#include <utils/assertion.h>
#include <ts/convex.h>

#include <stdio.h>

static void init_Char( Char *ch, va_list *arg_list )
{
    *ch = (char)va_arg( *arg_list, int );
}

static void copy_Char( Char *src, Char *dst )
{
    *dst = *src;
}

static int compare_Char( Char *left, Char *right )
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

static String *to_string_Char( Char *ch )
{
    char buf[256];

    sprintf( buf, "%c", *ch );

    return create_String( buf );
}

static String *to_XML_Char( Char *ch )
{
    return to_XML_spec("Char", to_string_Char(ch));
}

const Type type_Char
    = { "Char"
      , sizeof(char)
      , &type_Object
      , (Init)init_Char
      , (Copy)copy_Char
      , (Compare)compare_Char
      , (ToString)to_string_Char
      , (ToXML)to_XML_Char
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Char *create_Char( char ch )
{
    return create( &type_Char, ch );
}

char value_Char( Char *ch )
{
    char res;

    CHECK_TYPE_COMPATIBLE(value_Char, &type_Char, ch );

    res = *ch;
    destroy(ch);
    return res;
}

