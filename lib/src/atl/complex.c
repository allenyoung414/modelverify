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
 * File: complex.c
 * Description:
 *       This file defines the specification type representing
 *       complex integer values.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/complex.h>


static void init_Complex( Complex *cmplx, va_list *arg_list ) {
    cmplx->re = va_arg( *arg_list, double );
    cmplx->im = va_arg( *arg_list, double );
}


static void copy_Complex( Complex *self, Complex *cmplx ) {
    cmplx->re = self->re;
    cmplx->im = self->im;
}

static int compare_Complex( Complex *left, Complex *right ) {
    return ( left->re != right->re ) || ( left->im != right->im );
}

static String *to_string_Complex( Complex *cmplx ) {
    return format_String( "( %f + %f*i )", cmplx->re, cmplx->im );
}

static String *to_XML_Complex( Complex *cmplx ) {
	return to_XML_spec("Complex", to_string_Complex(cmplx));
}

const Type type_Complex
	= { "Complex"
      , sizeof( Complex )
      , &type_Object
      , (Init)init_Complex
      , (Copy)copy_Complex
      , (Compare)compare_Complex
      , (ToString)to_string_Complex
      , (ToXML)to_XML_Complex
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Complex *create_Complex( double re, double im ) {
    return create( &type_Complex, re, im );
}
