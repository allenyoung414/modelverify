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
 * File: float.c
 * Description:
 *       This file defines the specification type representing float values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/float.h>

#include <atl/string.h>
#include <utils/assertion.h>

#include <stdio.h>

static void init_Float( Float *f, va_list *arg_list )
{
    *f = (float)va_arg( *arg_list, double );
}

static void copy_Float( Float *src, Float *dst )
{
    *dst = *src;
}

static int compare_Float( Float *left, Float *right )
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

static String *to_string_Float( Float *f )
{
    char buf[256];

    sprintf( buf, "%f", *f );

    return create_String( buf );
}

static String *to_XML_Float( Float *f )
{
	return to_XML_spec("Float", to_string_Float(f));
}

const Type type_Float
    = { "Float"
      , sizeof(float)
      , &type_Object
      , (Init)init_Float
      , (Copy)copy_Float
      , (Compare)compare_Float
      , (ToString)to_string_Float
      , (ToXML)to_XML_Float
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Float *create_Float( float f )
{
    return create( &type_Float, f );
}

float value_Float( Float *f )
{
    float res;

    CHECK_TYPE_COMPATIBLE( value_Float, &type_Float, f );

    res = *f;
    destroy(f);
    return res;
}
