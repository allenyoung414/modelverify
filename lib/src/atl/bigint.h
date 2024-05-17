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
#ifndef __ATL_BIG_INTEGER_H__
#define __ATL_BIG_INTEGER_H__

#include <atl/object.h>


#ifdef __SEC__
  #pragma SEC file
  specification typedef struct BigInteger BigInteger;
#else
  typedef struct BigInteger BigInteger;
  extern const Type type_BigInteger;
#endif


/*
 * Constructs a newly allocated BigInteger object
 * that represents the primitive int argument.
 */
BigInteger* create_BigInteger( int i );

/*
 * Returns the value of this BigInteger as an int.
 */
int intValue_BigInteger( BigInteger* bi );

/*
 * Returns the value of this BigInteger as an long.
 */
long longValue_BigInteger( BigInteger* bi );

/*
 * Returns a result of addition of the arguments.
 */
BigInteger* add_BigInteger( BigInteger* n1, BigInteger* n2 );

/*
 * Returns a result of subtraction of the arguments.
 */
BigInteger* subtract_BigInteger( BigInteger* n1, BigInteger* n2 );

/*
 * Returns a result of multiplication of the arguments.
 */
BigInteger* multiply_BigInteger( BigInteger* n1, BigInteger* n2 );

/*
 * Returns a result of integer division of the arguments.
 */
BigInteger* divide_BigInteger( BigInteger* n1, BigInteger* n2 );

/*
 * Returns a remainder of integer division of the arguments.
 */
BigInteger* remainder_BigInteger( BigInteger* n1, BigInteger* n2 );

/*
 * Returns a result of raising to a power of the arguments.
 */
BigInteger* power_BigInteger( BigInteger* n1, int n2 );

/*
 * Returns a BigInteger whose value is -i.
 */
BigInteger* negate_BigInteger( BigInteger* n );

/*
 * Constructs a newly allocated BigInteger object
 * that represents the string argument.
 */
BigInteger* valueOf_BigInteger( String* str );

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
