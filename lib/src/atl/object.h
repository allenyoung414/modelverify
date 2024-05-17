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
 * File: object.h
 * Description:
 *       This file contains basic definitions of CTesK Abstract Type Library.
 */

#ifndef __ATL_OBJECT_H__
#define __ATL_OBJECT_H__

/* #define _DEBUG_REFCNT */

#ifndef __SEC__
  typedef void Object;
#endif

typedef struct Type Type;

#include <stdarg.h>
#include <stddef.h>

#include <atl/string.h>
#include <utils/boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 'Reference' is a pointer to <data> part of object.
 */

#define CHECK_REFERENCE( f, ref )               \
    assertion( ref != ((void*)0)                \
             , FORMAT( #f ": NULL pointer" )    \
             )

/*
 * Create new registered reference to object.
 * This function DOESN'T copy the object, it increments reference counter only.
 * Use it to pass objects to functions if reference will be used
 * after function call.
 *
 * These functions are DEPRECATED for use in SEC code.
 */
Object *r( Object *ref );

Object* r_weak( Object* ref );
Object* r_hard( Object* ref );
Object* r_hard2weak( Object* ref );
Object* r_weak2hard( Object* ref );

/*
 * Taking and releasing of references for optimization.
 *
 * These functions are DEPRECATED for use in SEC code.
 */
void r_take( Object* ref );
void r_release( Object* ref );


/*
 * =============================================================================
 *
 * Type descriptor
 *
 * =============================================================================
 */

/*
 * Reference initializer type (initialize given <data> area)
 */
typedef void (*Init)(void* ref, va_list* arg_list);

/*
 * Reference copier type (copy <data> from 'src' to 'dst')
 */
typedef void (*Copy)(void* src, void* dst);

/*
 * Reference comparer type (compare <data> of 'left' and 'right')
 */
typedef int (*Compare)(void* left, void* right);

/*
 * Reference stringifier type (make string representation of <data>)
 */
typedef String* (*ToString)(void* ref);

/*
 * Reference xmlifier type (make xml representation of <data>)
 */
typedef String* (*ToXML)(void* ref);

/*
 * Reference invariant checker type (check invariant of 'ref')
 */
typedef bool (*CheckInvariant)(Object* ref);

/*
 * Type of enumeration callback function
 */
typedef bool (*EnumerationCallbackFuncType)(void* ref, void* par);

/*
 * Subreference enumerator (call callback function for all internal references of 'ref')
 */
typedef void (*Enumerate)(void* ref, EnumerationCallbackFuncType callback, void* par);

/*
 * Reference destructor type (free resources allocated by reference)
 */
typedef void (*Destroy)(void* ref);

/*
 * 'Type' describes data type.
 */
struct Type
{
    const char*    name;            /* type name                   */
    size_t         size;            /* size of <data> part or 0 if it is subtype */
    const Type*    base;            /* base type of the given one  */
    Init           init;            /* initializer of <data> part  */
    Copy           copy;            /* copier of <data> part       */
    Compare        compare;         /* comparer of <data> parts    */
    ToString       to_string;       /* string representation       */
    ToXML          to_XML;          /* xml representation          */
    CheckInvariant check_invariant; /* type invariant checker      */
    Enumerate      enumerate;       /* enumerator of subreferences */
    Destroy        destroy;         /* destructor of <data> part   */

};

/*
 * Basic specification type descriptor.
 */
extern const Type type_Object;

/*
 * Type descriptor of reference.
 */
const Type* type( Object *ref );

/*
 * Type descriptor of reference.
 * This function doesn't destroy its argument, so don't clone it!
 *
 * This function is DEPRECATED for use in SEC code.
 */
const Type* TYPE( Object *ref );

/*
 * returns size of <data> part of type
 */
size_t size_Type( const Type* type );

/*
 * Checks that object has correct type
 */
#define CHECK_TYPE( f, t, ref )                                 \
    CHECK_REFERENCE( f, ref );                                  \
    assertion(    (t) == 0                                      \
               || isBasicType((t), TYPE( (void*)ref ))          \
             , FORMAT( #f ": '%s' type required, '%s' found" )  \
             , (t) ? (t)->name : "<null>"                       \
             , TYPE( (void*)ref )->name                         \
             )

/*
 * returns true iff 'type1' is a basic type of 'type2'
 */
bool isBasicType(const Type* type1,const Type* type2);

/*
 * returns the maximal type of the given one
 */
const Type* getMaximalType(const Type* type);

/*
 * Checks that object has compatible type with the given one
 */
#define CHECK_TYPE_COMPATIBLE( f, t, ref )                      \
    CHECK_REFERENCE( f, ref );                                  \
    assertion(    (t) == ((void*)0)                             \
               || isBasicType(t,TYPE((void*)ref))               \
             , FORMAT( #f ": '%s' type required, '%s' found" )  \
             , (t) ? (t)->name : "<null>"                       \
             , TYPE( (void*)ref )->name                         \
             )

/*
 * Create new object of type '*type',
 * set its value using provided initializer
 * and return registered reference.
 */
Object* create_Object( const Type*, ... );
Object* va_create_Object( const Type*, va_list* arg_list );

#ifndef ATL_CREATE_DISABLE
  Object* create( const Type*, ... );
#endif

#ifndef ATL_VA_CREATE_DISABLE
  Object* va_create( const Type*, va_list* arg_list );
#endif


/*
 * Create new object of type 'type( ref )'
 * and initialize it by '*ref' contents
 */
Object* clone_Object( Object* ref );

#ifndef ATL_CLONE_DISABLE
  Object* clone( Object* ref );
#endif


/*
 * Set '*ref' value using provided initializer
 */
void set_Object( Object* ref, ... );

#ifndef ATL_SET_DISABLE
  void set( Object* ref, ... );
#endif


/*
 * Copy object '*src' to '*dst'
 *
 * PRECONDITION: type( src ) == type( dst )
 * - call 'dst' destructor
 * - call copier 'src' -> 'dst'
 * - unregister 'src', 'dst'
 */
void copy_Object( Object *src, Object *dst );

#ifndef ATL_COPY_DISABLE
  void copy( Object *src, Object *dst );
#endif


/*
 * Compare objects '*left' and '*right'
 *
 * PRECONDITION: type( left ) == type( right )
 * - call comparer 'left' <> 'right'
 * - unregister 'left', 'right'
 */
int compare_Object( Object *left, Object *right );

#ifndef ATL_COMPARE_DISABLE
  int compare( Object *left, Object *right );
#endif


/*
 * Compare objects '*self' and '*ref'
 *
 * PRECONDITION: type( self ) == type( ref )
 * - call comparer 'self' == 'ref'
 * - unregister 'self', 'ref'
 */
bool equals_Object( Object *self, Object *ref );

#ifndef ATL_EQUALS_DISABLE
  bool equals( Object *self, Object *ref );
#endif


/*
 * - call stringifier of '*ref'
 * - unregister 'ref'
 */
String* toString_Object( Object *ref );

#ifndef ATL_TO_STRING_DISABLE
  String* toString( Object *ref );
  String* toTrace( Object *ref );
#endif

/*
 * - call xmlifier of '*ref'
 * - unregister 'ref'
 */
String* toXML_Object( Object *ref );

#ifndef ATL_TO_XML_DISABLE
  String* toXML( Object *ref );
  String* to_XML_spec(const char *type, String *value);
  String* toTrace( Object *ref );
#endif

/*
 * - call invariant checker of '*ref'
 * - unregister 'ref'
 */
bool checkInvariant_Object(Object* ref);

#ifndef ATL_CHECK_INVARIANT_DISABLE
  bool checkInvariant(Object* ref);
#endif


/*
 * Unregister weak reference:
 * - decrement weak reference counter
 * - if reference counter == 0,
 *   call object destructor and free memory
 */
void detach_Object( Object* ref );


/*
 * Unregister hard reference:
 * - decrement reference counter
 * - if reference counter == 0,
 *   call object destructor and free memory
 */
void destroy_Object( Object *ref );

#ifndef ATL_CHECK_INVARIANT_DISABLE
  void destroy( Object *ref );
#endif


/*
 * =============================================================================
 *
 * Default actions
 *
 * =============================================================================
 */

/*
 * fill by zeros
 */
void init_Default( Object *ref, va_list* arg_list );

/*
 * byte-to-byte copy
 */
void copy_Default( Object *src, Object *dst );

/*
 * byte-to-byte compare
 */
int compare_Default( Object *left, Object *right );

/*
 * <type_name>_<pointer>
 */
String *to_string_Default( Object *ref );

/*
 * <type_name>@<pointer>
 */
String *to_XML_Default( Object *ref );

/*
 * return true
 */
bool check_invariant_Default( Object* ref );

/*
 * do nothing
 */
void enumerate_Default( Object *ref, EnumerationCallbackFuncType callback, void* par );

/*
 * do nothing
 */
void destroy_Default( Object *ref );


/*
 * =============================================================================
 *
 * Subtype actions
 *
 * =============================================================================
 */

/*
 * call init function of base type
 */
void init_Subtype( Object *ref, va_list* arg_list );

/*
 * call copy function of base type
 */
void copy_Subtype( Object *src, Object *dst );

/*
 * call compare function of base type
 */
int compare_Subtype( Object *left, Object *right );

/*
 * call to_string function of base type
 */
String *to_string_Subtype( Object *ref );

/*
 * call to_XML function of base type
 */
String *to_XML_Subtype( Object *ref );

/*
 * call enumerate function of base type
 */
void enumerate_Subtype( Object *ref, EnumerationCallbackFuncType callback, void* par );

/*
 * call destroy function of base type
 */
void destroy_Subtype( Object *ref );

#ifdef __cplusplus
} // extern "C"
#endif

#endif
