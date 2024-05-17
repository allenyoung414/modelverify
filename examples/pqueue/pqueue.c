/* 
 * File: pqueue.c
 * Description: 
 *       This file contains an implementation of the queue with priorities.
 *
 * Copyright (c) 2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#include "pqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESERVE 16

/*
 * The queue state
 *
 * The items variable contains a pointer to array of items.
 * The head variable contains an offset of the first element of the queue. 
 * The join variable contains an offset of the first low prioritized element of the queue. 
 * The tail variable contains an offset of the first free element of the queue.
 * The end variable contains a size of allocated array pointed by items.
 */  
static void **items = NULL;
static int head = 0;
static int join = 0;
static int tail = 0;
static int end = 0;

/*
 * The auxiliary function.
 *
 * Expands array pointed by items.
 */
static void grow() {

  if (head < RESERVE) {
    items = realloc(items, (end + RESERVE) * sizeof(void*));
    memmove(items + head + RESERVE, items + head, (tail - head) * sizeof(void*));
    head += RESERVE;
    join += RESERVE;
    tail += RESERVE;
    end += RESERVE;
  }

  if (end - tail < RESERVE) {
    items = realloc(items, (end + RESERVE) * sizeof(void*));
    end += RESERVE;
  }
}

static void shrink() {

  if (head > RESERVE << 1) {
    memmove(items + head - RESERVE, items + head, (tail - head) * sizeof(void*));
    items = realloc(items, (end - RESERVE) * sizeof(void*));
    head -= RESERVE;
    join -= RESERVE;
    tail -= RESERVE;
    end -= RESERVE;
  }

  if ( (end-tail) > RESERVE << 1) {
    items = realloc(items, (end - RESERVE) * sizeof(void*));
    end -= RESERVE;
  }
  
}


/*
 * Enqueues the item to the queue with low priority.
 *
 * The parameter item is a nonnull pointer.
 */  
void enq(void *item) {
  grow();
  items[tail++] = item;
}

/*
 * Enqueues the item to the queue with high priority.
 *
 * The parameter item is a nonnull pointer.
 */
void enqp(void *item) {
  grow();
  memmove(items+head-1,items+head,(join-head) * sizeof(void*));
  head--;
  items[join-1] = item;
}

/*
 * Dequeues the item from the queue.
 *
 * The function returns 
 *     the first item enqueued into the queue with high priority 
 *         if such item is in the queue,
 *     the first item enqueued into the queue with low priority
 *         if such item is in the queue,
 *     NULL
 *         otherwise.
 */  
void* deq() {
  void* res;
  if (head == tail)
    return NULL;
  res = items[head++];
  if (join < head)
    join = head;
  shrink();
  return res;
}
