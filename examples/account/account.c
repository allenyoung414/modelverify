/* 
 * File: account.c
 * Description: 
 *       This file contains an implementation of the simple account system.
 *
 * Copyright (c) 2003-2004 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#include "account.h"


/*
 * Deposits the sum to the account.
 *
 * The parameter acct is a pointer to an Account structure. Should be not NULL.
 * The parameter sum is a sum to be deposited. Should be more than 0.
 */  
void deposit (Account *acct, int sum) {
  acct->balance += sum;
}


/*
 * Withdraws the sum from the account.
 *
 * The parameter acct is a pointer to an Account structure. Should be not NULL.
 * The parameter sum is a sum to be withdrawn. Should be more than 0.
 *
 * The function returns 
 *   0   - if the sum exceeds the balance on more than MAXIMUM_CREDIT,
 *   sum - if the sum does not exceed the balance
 *            or exceeds it on less than MAXIMUM_CREDIT.
 */  
int withdraw (Account *acct, int sum) {
  /* if (acct->balance < sum - MAXIMUM_CREDIT) return 0; */
  if (acct->balance - sum < -MAXIMUM_CREDIT)
    return 0;
  acct->balance -= sum;
  return sum;
}
