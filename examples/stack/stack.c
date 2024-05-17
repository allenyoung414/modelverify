/* 
 * File: stack.c
 * Description: 
 *       This file contains an implementation of the stack subsystem.
 *
 * Copyright (c) 2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#include "stack.h"
#include <memory.h>
#include <assert.h>


/*
 * Creates the stack with the capacity given.
 */
stack* create_stack (size_t cap) {
  stack* stck = 0;

  stck = (stack*)malloc(sizeof(stack));
  assert(stck);

  stck->elems = (int*)calloc(cap, sizeof(int));
  assert(stck->elems);
  stck->size = 0;
  stck->capacity = cap;

  return stck;
}

/*
 * Deletes the stack.
 *
 * The stck parameter should be not NULL.
 */
void delete_stack (stack* stck) {
  assert(stck && stck->elems);
  free(stck->elems);
  free(stck);
}

/*
 * Pushes the integer into the stack.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns zero if stack is full and nonzero otherwise.
 */  
int push (stack* stck, int e) {
  assert(stck && stck->elems);

  if (stck->size == stck->capacity) /* stack is full */
    return 0;

  stck->elems[stck->size] = e;
  stck->size++;

  return 1;
}

/*
 * Pops the element from the stack and writes it by e pointer 
 * if the stack is not empty.
 *
 * The stck and e parameters should be not NULL.
 *
 * The function returns zero if stack is empty and nonzero otherwise.
 */  
int pop (stack* stck, int* const e) {
  assert(stck && stck->elems && e);

  if (stck->size == 0) /* stack is empty */
    return 0;

  stck->size--;
  *e = stck->elems[stck->size];

  return 1;
}

/*
 * Evaluates is the stack empty or not.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns nonzero if stack is empty and nonzero otherwise.
 */  
int is_empty (stack* stck) {
  assert(stck);

  return (stck->size == 0);
}

/*
 * Evaluates is the stack full or not.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns nonzero if stack is full and nonzero otherwise.
 */
int is_full (stack* stck) {
  assert(stck);

  return (stck->size == stck->capacity);
}

/*
 * Evaluates the size of the stack.
 *
 * The stck parameter should be not NULL.
 *
 * The function returns the size of the stack.
 */  
size_t how_much (stack* stck) {
  assert(stck);

  return (stck->size);
}
