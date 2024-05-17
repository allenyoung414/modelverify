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
 * File: void_ast.c
 * Description:
 *       This file defines the specification type representing pointer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/void_ast.h>

#include <atl/string.h>
#include <utils/assertion.h>

#include <stdio.h>

static void init_VoidAst( VoidAst *ptr, va_list *arg_list )
{
    *ptr = va_arg( *arg_list, void *);
}

static void copy_VoidAst( VoidAst *src, VoidAst *dst )
{
    *dst = *src;
}

static int compare_VoidAst( VoidAst *left, VoidAst *right )
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

static String *to_string_VoidAst( VoidAst *ptr )
{
    char buf[256];

    sprintf( buf, "%p", *ptr );

    return create_String( buf );
}

static String *to_XML_VoidAst( VoidAst *ptr )
{
	return to_XML_spec("VoidAst", to_string_VoidAst(ptr));
}

const Type type_VoidAst
    = { "VoidAst"
      , sizeof(void*)
      , &type_Object
      , (Init)init_VoidAst
      , (Copy)copy_VoidAst
      , (Compare)compare_VoidAst
      , (ToString)to_string_VoidAst
      , (ToXML)to_XML_VoidAst
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

VoidAst *create_VoidAst( void *ptr )
{
    return create( &type_VoidAst, ptr );
}

void *value_VoidAst( VoidAst *ptr )
{
void* res;

  CHECK_TYPE_COMPATIBLE( value_VoidAst, &type_VoidAst, ptr );

  res = *ptr;
  destroy(ptr);
  return res;
}
