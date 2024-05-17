/* 
 * File: account.h 
 * Description: 
 *       This is a header file of the simple account system.
 *
 * Copyright (c) 2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

/*
 * A simple account can store an integer value of balance in thousands dollars.
 * The account given supports credits not greater than some fixed value.
 */
 

/*
 * The value opposite to the minimum possible balance of an account.
 */
#define MAXIMUM_CREDIT 3


/*
 * The Account structure contains the only balance field.
 */  
typedef struct Account {
  int balance;
} Account;


/*
 * Deposits the sum to the account.
 *
 * The parameter acct is a pointer to an Account structure. Should be not NULL.
 * The parameter sum is a sum to be deposited. Should be more than 0.
 */  
void deposit(Account* acct, int sum);


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
int  withdraw(Account *acct, int sum);


#endif /* __ACCOUNT_H__ */
