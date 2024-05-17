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
 * File: list.h
 * Description:
 *       This file declares the specification type representing
 *       an abstract list container.
 *       It is a part of CTesK Abstract Type Library.
 */

#ifndef __ATL_LIST_H__
#define __ATL_LIST_H__

#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __SEC__
  #pragma SEC file
  specification typedef struct List List;
#else
  typedef struct List List;
  extern const Type type_List;
#endif


#include <atl/set.h>

#define DEFAULT_INITIAL_LIST_SIZE	16


/*
 * Create empty list of specified type.
 * If elem_type == NULL element types can differ.
 */
List *create_List( const Type *elem_type );

/*
 * Create empty list of specified type with specified initial capacity.
 * If elem_type == NULL element types can differ.
 */
List *createCustomized_List( const Type *elem_type, int initialCapacity );

/*
 * Inserts the specified element at the specified position in this list
 */
void add_List( List *self, int index, Object *ref );

/*
 * Appends the specified element to the end of this list
 */
void append_List( List *self, Object *ref );

/*
 * Removes all of the elements from this list
 */
void clear_List( List *self );

/*
 * Returns true if this list contains the specified element
 * PRECONDITION: type(ref) == elem_type(self)
 */
bool contains_List( List *self, Object *ref );

/*
 * Returns the element at the specified position in this list.
 */
Object *get_List( List *self, int index );

/*
 * Returns the index in this list of the first occurrence
 * of the specified element, or -1 if this list does not contain this element
 * PRECONDITION: type(ref) == elem_type(self)
 */
int indexOf_List( List *self, Object *ref );

/*
 * Returns true if this list contains no elements
 */
bool isEmpty_List( List *self );

/*
 * Returns the index in this list of the last occurrence
 * of the specified element, or -1 if this list does not contain this element
 * PRECONDITION: type(ref) == elem_type(self)
 */
int lastIndexOf_List( List *self, Object *ref );

/*
 * Removes the element at the specified position in this list
 */
void remove_List( List *self, int index );

/*
 * Replaces the element at the specified position in this list
 * with the specified element
 */
void set_List( List *self, int index, Object *ref );

/*
 * Returns the number of elements in this list.
 */
int size_List( List *self );

/*
 * Returns a view of the portion of this list between 
 * the specified fromIndex, inclusive, and toIndex, exclusive.
 */
List *subList_List( List *self, int fromIndex, int toIndex );

/*
 * Inserts the specified list at the specified position in this list
 */
void addAll_List(List *self, int index, List *other);

/*
 * Appends the specified list to the end of this list
 */
void appendAll_List(List *self, List *other);

/*
 * Returns set that contains all elements from this list
 */
Set *toSet_List(List *self);


/*
 * Returns the type desc of elements
 */
Type *elemType_List (List *self);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
