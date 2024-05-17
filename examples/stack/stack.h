/* 
 * File: stack.h
 * Description: 
 *       This is a header file of the stack subsystem.
 *
 * Copyright (c) 2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#ifndef STACK_H
#define STACK_H

/*
 * The stack subsystem is a stack of integers.
 * Capacity of the stack is defined during its construction.
 */


#include <stdlib.h>

/*
 * The internal structure of the stack.
 * The elems field points to array of integers.
 * The size field keeps a current size of the stack.
 * The capacity field keeps the maximal size of the stack.
 */
typedef struct {
  int*   elems;
  size_t size;
  size_t capacity;
} stack;


/*
 * Creates the stack with the capacity given.
 */
stack* create_stack (size_t size);

/*
 * Deletes the stack.
 *
 * The stck parameter should be not NULL.
 */
void delete_stack (stack* stck);

/*
 * Pushes the integer into the stack.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns zero if stack is full and nonzero otherwise.
 */  
int push (stack* stck, int e);

/*
 * Pops the element from the stack and writes it by e pointer 
 * if the stack is not empty.
 *
 * The stck and e parameters should be not NULL.
 *
 * The function returns zero if stack is empty and nonzero otherwise.
 */
int pop (stack* stck, int* const e);

/*
 * Evaluates is the stack empty or not.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns nonzero if stack is empty and nonzero otherwise.
 */
int is_empty (stack* stck);

/*
 * Evaluates is the stack full or not.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns nonzero if stack is full and nonzero otherwise.
 */
int is_full (stack* stck);

/*
 * Evaluates the size of the stack.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns the size of the stack.
 */  
size_t how_much (stack* stck);


#endif
