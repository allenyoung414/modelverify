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
#ifndef __ATL_MULTI_SET_H__
#define __ATL_MULTI_SET_H__


#include <atl/object.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __SEC__
  #pragma SEC file
  specification typedef struct MultiSet MultiSet;
#else
  typedef struct MultiSet MultiSet;
  extern const Type type_MultiSet;
#endif


#include <atl/list.h>
#include <atl/set.h>


/*
 * Create empty multiset of specified type.
 */
MultiSet *create_MultiSet( const Type* elem_type );

/*
 * Returns the number of elements in this multiset.
 */
int size_MultiSet( MultiSet *self );

/*
 * Returns true if this multiset contains no elements.
 */
bool isEmpty_MultiSet( MultiSet *self );

/*
 * Returns the number of the specified elements containing in the multiset.
 */
int contains_MultiSet( MultiSet *self, Object *ref );

/*
 * Adds the specified element to this multiset
 */
int add_MultiSet( MultiSet *self, Object *ref );

/*
 * Removes single copy of the specified element from this multiset if it is present
 */
bool remove_MultiSet( MultiSet *self, Object *ref );

/*
 * Removes specified copies of specified element from this multiset if it is present
 */
int removeCount_MultiSet( MultiSet *self, Object *ref, int count );

/*
 * Removes the specified element completely from this multiset if it is present
 */
int removeFull_MultiSet( MultiSet *self, Object *ref );

/*
 * Removes from this multiset all of its elements that are contained in the specified multiset
 */
int removeAll_MultiSet( MultiSet *self, MultiSet *set );

/*
 * Returns true if the multiset contains all of the elements of the specified multiset
 */
bool containsAll_MultiSet( MultiSet *self, MultiSet *set );

/*
 * Adds all of the elements in the specified multiset to this multiset
 */
bool addAll_MultiSet( MultiSet *self, MultiSet *set );

/*
 * Retains only the elements in this multiset that are contained in the specified multiset
 */
bool retainAll_MultiSet( MultiSet *self, MultiSet *set );

/*
 * Removes all of the elements from this multiset
 */
void clear_MultiSet( MultiSet *self );

/*
 * Returns the element at the specified position in this set treated as list
 * NOTE: Order of elements is unpredictable
 */
Object *get_MultiSet( MultiSet *self, int index );

/*
 * Returns list that contains all elements from this multiset
 */
List *toList_MultiSet( MultiSet *self );

/*
 * Returns set that contains all elements from this multiset
 */
Set *toSet_MultiSet( MultiSet *self );

/*
 * Returns the type desc of elements
 */
Type *elemType_MultiSet(MultiSet *self);


//MultiSet* unionOne_MultiSet( MultiSet* left, Object* obj );

//MultiSet* union_MultiSet( MultiSet* left, MultiSet* right );

//MultiSet* intersect_MultiSet( MultiSet* left, MultiSet* right );

//MultiSet* diff_MultiSet( MultiSet* left, MultiSet* right );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
