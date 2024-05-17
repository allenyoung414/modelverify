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
#include <atl/bigint.h>
//#include <common/bigint.h>
#include <utils/assertion.h>
#include <stdlib.h>
#include <string.h>


#define DIGIT_BASE    256
#define ln_DIGIT_BASE 8

#define NEGATIVE(c) ((c) & 0x80)


/********************************************************************/
/**                     BigInteger structure                       **/
/********************************************************************/
struct BigInteger
{
  int len;
  unsigned char* number;
};


/********************************************************************/
/**                       Standard Functions                       **/
/********************************************************************/
static void fillUnsigned(BigInteger *bi, unsigned int n)
{
unsigned int m = n;
int i;
  for(bi->len=1; m>=DIGIT_BASE; bi->len++)
    m = m/DIGIT_BASE;
  bi->number = (unsigned char*)malloc(bi->len+1);

  m=n;
  for(i=bi->len-1; i>=0; i--)
  {
	bi->number[i] = m%DIGIT_BASE;
    m = m/DIGIT_BASE;
  }
}

static void init_BigInteger( BigInteger *bi, va_list *arg_list )
{
int i;
int n = va_arg( *arg_list, int );

  if (n >= 0) {
    fillUnsigned(bi, (unsigned int)n);
  } else {
    fillUnsigned(bi, (unsigned int)(~n));
    for(i=bi->len; i>=0; i--) { // inverse again. This surely works with DIGIT_BASE=256
      bi->number[i] = ~bi->number[i];
	}
  }

  if ( ((n<0) && NEGATIVE(bi->number[0])) || ((n>=0) && !NEGATIVE(bi->number[0])) )
    return;

  for(i=bi->len; i>0; i--)
    bi->number[i] = bi->number[i-1];
  bi->number[0] = (n<0) ? 0xFF : 0;
  bi->len++;
}

static void copy_BigInteger( BigInteger* src, BigInteger* dst )
{
int j;

  dst->len = src->len;
  dst->number = (unsigned char*)malloc(dst->len);
  for(j=0;j<dst->len;j++)
    dst->number[j] = src->number[j];
}

static int compare_BigInteger( BigInteger* left, BigInteger* right )
{
int i;

  if (NEGATIVE(left->number[0]))
   {if (!NEGATIVE(right->number[0]))
      return -1;
    if (left->len > right->len)
      return -1;
    if (left->len < right->len)
      return 1;
    for(i=0;i<left->len;i++)
     {if (left->number[i] > right->number[i])
        return 1;
      if (left->number[i] < right->number[i])
        return -1;
     }
    return 0;
   }
  else
   {if (NEGATIVE(right->number[0]))
      return 1;
    if (left->len > right->len)
      return 1;
    if (left->len < right->len)
      return -1;
    for(i=0;i<left->len;i++)
     {if (left->number[i] > right->number[i])
        return 1;
      if (left->number[i] < right->number[i])
        return -1;
     }
    return 0;
   }
}

static String* to_string_BigInteger( BigInteger* bi )
{
  if (NEGATIVE(bi->number[0]))
  {
    return concat_String( create_String( "-" ), toString(negate_BigInteger(r(bi))) );
  }
  else
  {
    char* buf;
    int i;
    BigInteger* a = clone_Object(r(bi));
    BigInteger* dec = create_BigInteger(10);
    String* res;

    i = (ln_DIGIT_BASE*bi->len)/3+1;
    buf = (char*)malloc(i+1);
    buf[i] = 0;
    i--;
    for(;;i--)
     {BigInteger* tmp = remainder_BigInteger( r(a), r(dec) );
      buf[i] = '0' + (tmp->number[0]);
      destroy(tmp);
      a = divide_BigInteger( a, r(dec) );
      if ((a->len == 1) && (a->number[0] == 0))
        break;
     }
    destroy(a);
    destroy(dec);
    res = create_String( &(buf[i]) );
    free(buf);
    return res;
   }
}

static String* to_XML_BigInteger( BigInteger* bi )
{
    return to_XML_spec("BigInteger", to_string_BigInteger(bi));
}

static void destroy_BigInteger( BigInteger* bi )
{
    free( bi->number );
}

const Type type_BigInteger
    = { "BigInteger"
      , sizeof(BigInteger)
      , &type_Object
      , (Init)init_BigInteger
      , (Copy)copy_BigInteger
      , (Compare)compare_BigInteger
      , (ToString)to_string_BigInteger
      , (ToXML)to_XML_BigInteger
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_BigInteger
      };


/********************************************************************/
/**                     Auxiliary functions                        **/
/********************************************************************/
static int maximum(int n1,int n2)
{
  return (n1 >= n2)?n1:n2;
}

static BigInteger* shift_byte(BigInteger* n1,int n2)
{
unsigned char* buf;
int i,len;
BigInteger* res;

  len = n1->len + n2;
  buf = (unsigned char*)malloc(len);
  for(i=0;i<n1->len;i++)
    buf[i] = n1->number[i];
  for(;i<len;i++)
    buf[i] = 0;

  destroy(n1);

  res = create_BigInteger(0);
  res->len = len;
  free(res->number);
  res->number = buf;
  return res;
}


static BigInteger* shift_bit(BigInteger* n1,int n2)
{
int i,j,len;
unsigned char lmask,rmask;
unsigned char* buf;
int sh = n2%ln_DIGIT_BASE;
int bsh = ln_DIGIT_BASE-sh;
BigInteger* res = shift_byte( r(n1), n2/ln_DIGIT_BASE );

  if (sh == 0)
    return res;
  rmask = 0;
  for(i=0;i<sh;i++)
    rmask = (rmask << 1) | 1;
  lmask = ~rmask;
  len = res->len;
  if (NEGATIVE(n1->number[0]))
   {if (!NEGATIVE(n1->number[0] << sh) || (((n1->number[0] >> bsh) & rmask) != (
0xFF & rmask)))
      len++;
   }
  else
   {if (NEGATIVE(n1->number[0] << sh) || (((n1->number[0] >> bsh) & rmask) != 0)
)
      len++;
   }
  buf = (unsigned char*)malloc(len);
  i = res->len-1;
  j = len - 1;
  buf[j] = ((res->number[i] << sh) & lmask) | ((NEGATIVE(n1->number[0]) ? 0xFF :
 0) & rmask);
  i--; j--;
  for(;i>=0;i--,j--)
    buf[j] = ((res->number[i] << sh) & lmask) | ((res->number[i+1] >> bsh) & rmask);
  if (j == 0)
    buf[0] = ((NEGATIVE(n1->number[0]) ? 0xFF : 0) & lmask) | ((res->number[0] >> bsh) & rmask);

  destroy(n1);

  res->len = len;
  free(res->number);
  res->number = buf;
  return res;
}


static int not_zero_bit(BigInteger* n)
{
int i,j;
unsigned char c;

  for(i=0;i<n->len;i++)
    if (n->number[i] != 0)
      break;
  if (i == n->len)
    return -1;
  c = n->number[i];
  for(j=ln_DIGIT_BASE-1;j>=0;j--)
   {if (c & 0x80)
      break;
    c = c << 1;
   }
  return (n->len-i-1)*ln_DIGIT_BASE + j;
}


/********************************************************************/
/**                       Interface Functions                      **/
/********************************************************************/

/*
 * Constructs a newly allocated BigInteger object
 * that represents the primitive int argument.
 */
BigInteger* create_BigInteger( int i )
{
  return create( &type_BigInteger, i );
}

/*
 * Returns the value of this BigInteger as an int.
 */
int intValue_BigInteger( BigInteger* bi )
{
int i,res;

  assertion( (bi->len <= sizeof(int))
           , FORMAT( "intValue_BigInteger: BigInteger is too big to be converted to int" )
           );

  res = ((signed char)(bi->number[0]));
  for(i=1;i<bi->len;i++)
    res = res*DIGIT_BASE + bi->number[i];

  destroy(bi);
  return res;
}

/*
 * Returns the value of this BigInteger as an long.
 */
long longValue_BigInteger( BigInteger* bi )
{
int i;
long res;

  assertion( (bi->len <= sizeof(long))
           , FORMAT( "longValue_BigInteger: BigInteger is too big to be converted to long" )
           );

  res = ((signed char)(bi->number[0]));
  for(i=1;i<bi->len;i++)
    res = res*DIGIT_BASE + bi->number[i];

  destroy(bi);
  return res;
}

/*
 * Returns a result of addition of the arguments.
 */
BigInteger* add_BigInteger( BigInteger* n1, BigInteger* n2 )
{
unsigned char *buf;
int i1,i2,j,m;
unsigned int carry,tmp;
unsigned char fill;
BigInteger* res = create_BigInteger(0);

  j = maximum(n1->len,n2->len)-1;
  buf = (unsigned char*)malloc(j+2);

  carry = 0;
  i1 = n1->len - 1;
  i2 = n2->len - 1;
  for(;(i1>=0)&&(i2>=0);i1--,i2--,j--)
   {tmp = n1->number[i1]+n2->number[i2]+carry;
    buf[j] = tmp%DIGIT_BASE;
    carry = tmp/DIGIT_BASE;
   }
  if (i1 < 0)
   {if(NEGATIVE(n1->number[0]))
      fill = 0xFF;
    else
      fill = 0;
    for(;i2>=0;i2--,j--)
     {tmp = fill+n2->number[i2]+carry;
      buf[j] = tmp%DIGIT_BASE;
      carry = tmp/DIGIT_BASE;
     }
   }
  else
  if (i2 < 0)
   {if(NEGATIVE(n2->number[0]))
      fill = 0xFF;
    else
      fill = 0;
    for(;i1>=0;i1--,j--)
     {tmp = n1->number[i1]+fill+carry;
      buf[j] = tmp%DIGIT_BASE;
      carry = tmp/DIGIT_BASE;
     }
   }
  if (NEGATIVE(n1->number[0]))
   {if (NEGATIVE(n2->number[0]))
     {if (!NEGATIVE(buf[0]))
       {fill = 0xFF;
        j = 0;
       }
     }
   }
  else
   {if (!NEGATIVE(n2->number[0]))
     {if (NEGATIVE(buf[0]))
       {fill = 0;
        j = 0;
       }
     }
   }


  m = maximum(n1->len,n2->len);
  destroy(n1);
  destroy(n2);
  free(res->number);
  res->number = buf;

  if (j != 0)
   {
    if (m == 1)
     {res->len = m;
      return res;
     }
    if ((buf[0] == 0) && (!NEGATIVE(buf[1])))
     {for(j=1;j<m-1;j++)
       {if ((buf[j]!=0) || (NEGATIVE(buf[j+1])))
          break;
       }
     }
    else if ((buf[0] == 0xFF) && (NEGATIVE(buf[1])))
     {for(j=1;j<m-1;j++)
       {if ((buf[j]!=0xFF) || (!NEGATIVE(buf[j+1])))
          break;
       }
     }
    else
     {res->len = m;
      return res;
     }
    for(i1=0,i2=j;i2<m;i1++,i2++)
      buf[i1] = buf[i2];
    res->len = m-j;
    return res;
   }
  for(j=m;j>0;j--)
    buf[j] = buf[j-1];
  buf[0] = fill;
  res->len = m+1;
  return res;
}

/*
 * Returns a result of subtraction of the arguments.
 */
BigInteger* subtract_BigInteger( BigInteger* n1, BigInteger* n2 )
{
  return add_BigInteger(n1,negate_BigInteger(n2));
}

/*
 * Returns a result of multiplication of the arguments.
 */
BigInteger* multiply_BigInteger( BigInteger* n1, BigInteger* n2 )
{
int i1,i2;

  if (NEGATIVE(n1->number[0]))
   {if (NEGATIVE(n2->number[0]))
      return multiply_BigInteger(negate_BigInteger(n1),negate_BigInteger(n2));
    else
      return negate_BigInteger(multiply_BigInteger(negate_BigInteger(n1),n2));
   }
  if (NEGATIVE(n2->number[0]))
   {
    return negate_BigInteger(multiply_BigInteger(n1,negate_BigInteger(n2)));
   }
  else
   {
     BigInteger* res = create_BigInteger(0);
     BigInteger* sum = NULL;


    for(i2=n2->len-1;i2>=0;i2--)
     {sum = create_BigInteger(0);
      for(i1=n1->len-1;i1>=0;i1--)
       {
        sum = add_BigInteger( sum,
                              shift_byte( create_BigInteger(n1->number[i1]*n2->number[i2]),
                                          (n1->len-i1-1)
                                        )
                            );
       }
      res = add_BigInteger( res, shift_byte( sum, (n2->len-i2-1) ) );
     }
    destroy(n1);
    destroy(n2);
    return res;
   }
}


/*
 * Returns a result of integer division of the arguments.
 */
BigInteger* divide_BigInteger( BigInteger* n1, BigInteger* n2 )
{
unsigned char* buf;
int i,len,d,d1,d2;
BigInteger* a;
BigInteger* b;

  assertion( (n2->len != 1) || (n2->number[0] != 0)
           , FORMAT( "divide_BigInteger: Division by zero" )
           );

  if (NEGATIVE(n1->number[0]))
   {if (NEGATIVE(n2->number[0]))
      return divide_BigInteger(negate_BigInteger(n1),negate_BigInteger(n2));
    else
      return negate_BigInteger(divide_BigInteger(negate_BigInteger(n1),n2));
   }
  if (NEGATIVE(n2->number[0]))
    return negate_BigInteger(divide_BigInteger(n1,negate_BigInteger(n2)));

  d2 = not_zero_bit(n2);


  d1 = not_zero_bit(n1);
  if (d1 < d2)
   {destroy(n1);
    destroy(n2);
    return create_BigInteger(0);
   }
  d = d1 - d2;
  b = shift_bit( r(n2), d );
  if (compare(r(n1),r(b)) < 0)
   {if (d == 0)
     {destroy(n1);
      destroy(n2);
      destroy(b);
      return create_BigInteger(0);
     }
    d--;
    destroy(b);
    b = shift_bit( r(n2), d );
   }
  len = ((d+1)/ln_DIGIT_BASE)+1;
  buf = (unsigned char*)malloc(len);
  for(i=len-1;i>=0;i--)
    buf[i] = 0;
  buf[len-(d/ln_DIGIT_BASE)-1] = (1 << (d%ln_DIGIT_BASE));
  a = subtract_BigInteger( r(n1), r(b) );


  for(;;)
   {d1 = not_zero_bit(a);
    if (d1 < d2)
      break;
    d = d1 - d2;
    destroy(b);
    b = shift_bit( r(n2), d );
    if (compare(r(a),r(b)) < 0)
     {if (d == 0)
        break;
      d--;
      destroy(b);
      b = shift_bit(r(n2),d);
     }
    i = len-(d/ln_DIGIT_BASE)-1;
    buf[i] = buf[i] | (1 << (d%ln_DIGIT_BASE));
    a = subtract_BigInteger( a, r(b) );
   }

  destroy(n1);
  destroy(n2);
  destroy(b);
  free(a->number);
  a->len = len;
  a->number = buf;
  return a;
}

/*
 * Returns a remainder of integer division of the arguments.
 * If n1 is non-negative, and n2 is positive, then positive number is returned.
 * In any case, the following expression holds:
 * integer[n1/n2] * n2 + (n1%n2) == n1
 */
BigInteger* remainder_BigInteger( BigInteger* n1, BigInteger* n2 )
{
int d,d1,d2;
BigInteger* a = NULL;
BigInteger* b = NULL;

  assertion( (n2->len != 1) || (n2->number[0] != 0)
           , FORMAT( "remainder_BigInteger: Division by zero" )
           );

  if (NEGATIVE(n1->number[0]) || NEGATIVE(n2->number[0]))
  {		// return n1 - n2 * [n1 / n2]
	  BigInteger *res = subtract_BigInteger(r(n1),
		  multiply_BigInteger(r(n2),
			divide_BigInteger(r(n1), r(n2))
		  )
	  );
	  destroy(n1);
	  destroy(n2);
	  return res;
  }


  d2 = not_zero_bit(n2);
  a = clone_Object(r(n1));

  for(;;)
   {d1 = not_zero_bit(a);
    if (d1 < d2)
      break;
    d = d1 - d2;
    destroy(b);
    b = shift_bit( r(n2), d );
    if (compare(r(a),r(b)) < 0)
     {if (d == 0)
        break;
      d--;
      destroy(b);
      b = shift_bit( r(n2), d );
     }
    a = subtract_BigInteger( a, r(b) );
   }
   destroy(n1);
   destroy(n2);
   destroy(b);
   return a;
}

/*
 * Returns a result of raising to a power of the arguments.
 */
BigInteger* power_BigInteger( BigInteger* n1, int n2 )
{
int i;
BigInteger* res = create_BigInteger(1);

  assertion( n2 >= 0, FORMAT( "power_BigInteger: Raising to negative power" ) );
  for( i = 0; i < n2; i++ )
    res = multiply_BigInteger( res, r(n1) );
  destroy(n1);
  return res;
}

/*
 * Returns a BigInteger whose value is -i.
 */
BigInteger* negate_BigInteger( BigInteger* n )
{
int i;
BigInteger* res2;

  res2 = clone_Object(n);
  for(i=0;i<res2->len;i++)
    res2->number[i] = ~res2->number[i];
  return add_BigInteger( res2, create_BigInteger(1) );
}

/*
 * Constructs a newly allocated BigInteger object
 * that represents the string argument.
 */
BigInteger* valueOf_BigInteger( String* str )
{
int i;
const char* buff;
BigInteger* res = create_BigInteger(0);
BigInteger* base = create_BigInteger(1);
BigInteger* dec = create_BigInteger(10);

  buff = toCharArray_String(r(str));
  for(i=strlen(buff)-1;i>0;i--)
   {assertion( ('0' <= buff[i]) && (buff[i] <= '9'),
               FORMAT( "valueOf_BigInteger: Incorrect integer format: %s" ),
               buff
             );
    res = add_BigInteger( res, multiply_BigInteger( create_BigInteger(buff[i]-'0'), r(base) ) );
    base = multiply_BigInteger( base, r(dec) );
   }
  if (buff[0] == '-')
    res = negate_BigInteger(res);
  else
    res = add_BigInteger( res, multiply_BigInteger( create_BigInteger(buff[0]-'0'), r(base) ) );

  destroy(str);
  destroy(base);
  destroy(dec);
  return res;
}
