/* 
 * File: pqueue.h
 * Description: 
 *       This is a header file of the queue with priorities.
 *
 * Copyright (c) 2003 ISP RAS.
 * B.Communisticheskaya,25, Moscow, Russia.
 * All rights reserved.
 *
 */

#ifndef __PQUEUE_H__
#define __PQUEUE_H__

/*
 * The pqueue system is a queue with two priorities. 
 * The dequeue operation returns a value with high priority in the first place
 * and does not change order of values with equal priorities.
 * Values stored are nonnull pointers.
 */


/*
 * Enqueues the item to the queue with low priority.
 *
 * The parameter item is a nonnull pointer.
 */  
void enq(void* item);


/*
 * Enqueues the item to the queue with high priority.
 *
 * The parameter item is a nonnull pointer.
 */
void enqp(void* item);


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
void* deq(void);


#endif
