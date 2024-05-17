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
 * File: object.c
 * Description:
 *       This file contains an implementation of
 *       basic functions of CTesK Abstract Type Library.
 */

#include <atl/object_int.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <atl/hash.h>
#include <ts/convex.h>
#include <utils/boolean.h>
#include <utils/assertion.h>

#include <stdlib.h>
#include <string.h>

/*
 * Object header contains reference to type descriptor
 * and reference counter for an object
 */
#define HEADER(ref)  ((Header*)((char*)ref - sizeof(Header)))

Header *header( Object *ref )
{
  return (Header*)((char*)ref - sizeof(Header));
}

static int inc_weak( Object *ref )
{
#ifdef _DEBUG_REFCNT
    printf( "inc %s %p %d\n", TYPE( ref )->name, ref, HEADER( ref )->hard_ref_cnt + HEADER( ref )->weak_ref_cnt + 1 );
#endif
  return ++(HEADER( ref )->weak_ref_cnt);
}

static int dec_weak( Object *ref )
{
#ifdef _DEBUG_REFCNT
  printf( "dec %s %p %d\n", TYPE( ref )->name, ref, HEADER( ref )->hard_ref_cnt + HEADER( ref )->weak_ref_cnt - 1 );
#endif
  return --(HEADER( ref )->weak_ref_cnt);
}

static int inc_hard( Object *ref )
{
#ifdef _DEBUG_REFCNT
  printf( "inc %s %p %d\n", TYPE( ref )->name, ref, HEADER( ref )->hard_ref_cnt + HEADER( ref )->weak_ref_cnt + 1 );
#endif
  return ++(HEADER( ref )->hard_ref_cnt);
}

static int dec_hard( Object *ref )
{
#ifdef _DEBUG_REFCNT
  printf( "dec %s %p %d\n", TYPE( ref )->name, ref, HEADER( ref )->hard_ref_cnt + HEADER( ref )->weak_ref_cnt - 1 );
#endif
  return --(HEADER( ref )->hard_ref_cnt);
}

static void hard2weak( Object *ref )
{
  if (HEADER( ref )->hard_ref_cnt == 0)
    return;
  --(HEADER( ref )->hard_ref_cnt);
  ++(HEADER( ref )->weak_ref_cnt);
}

static void weak2hard( Object *ref )
{
  if (HEADER( ref )->weak_ref_cnt == 0)
    return;
  --(HEADER( ref )->weak_ref_cnt);
  ++(HEADER( ref )->hard_ref_cnt);
}


Object* r( Object* ref )
{
  if (ref != 0)
    inc_hard( ref );
  return ref;
}

Object* r_weak( Object* ref )
{
  if (ref != 0)
    inc_weak( ref );
  return ref;
}

Object* r_hard( Object* ref )
{
  if (ref != 0)
    inc_hard( ref );
  return ref;
}

Object* r_hard2weak( Object* ref )
{
  if (ref != 0)
    hard2weak( ref );
  return ref;
}

Object* r_weak2hard( Object* ref )
{
  if (ref != 0)
    weak2hard( ref );
  return ref;
}

void r_take( Object* ref )
{
  if (ref != 0)
    inc_hard( ref );
}

void r_release( Object* ref )
{
  if (ref != 0)
    dec_hard( ref );
}

/*
 * Basic specification type descriptor.
 */
const Type type_Object
    = { "Object"
      , 0
      , NULL
      , (Init)NULL
      , (Copy)NULL
      , (Compare)NULL
      , (ToString)NULL
      , (ToXML)NULL
      , (CheckInvariant)NULL
      , (Enumerate)NULL
      , (Destroy)NULL
      };


const Type *TYPE( Object *ref )
{
  CHECK_REFERENCE( TYPE, ref );

  return HEADER( ref )->type;
}

const Type *type( Object *ref )
{
const Type *res;

  CHECK_REFERENCE( type, ref );

  res = HEADER( ref )->type;

  destroy( ref );

  return res;
}

/*
 * returns size of <data> part of type
 *   the field size contains size of <data> part of type or 0 if it is subtype
 *
 */
size_t size_Type( const Type* type )
{
  if (type->size != 0)
    return type->size;
  return getMaximalType(type)->size;
}

/*
 * returns true iff 'type1' is basic type of 'type2'
 */
bool isBasicType(const Type* type1,const Type* type2)
{
const Type* tmp;

  for(tmp=type2;tmp!=NULL;tmp=tmp->base)
    if (tmp == type1)
      return true;
  return false;
}

/*
 * returns the maximal type of the given one
 */
const Type* getMaximalType(const Type* type)
{
const Type* tmp;

  for(tmp=type;tmp!=NULL;tmp=tmp->base)
    if (tmp->base == &type_Object)
      return tmp;
  assertion( false, "Incorrect type descriptor of '%s'", (type->name != NULL) ? type->name : "<NULL>" );
  return NULL;
}

/*
 * =============================================================================
 *
 * Cycle by reference processing
 *
 * =============================================================================
 */
static void atl_start_copy(void);
static Object* atl_was_copy(Object* ref);
static void atl_register_copy(Object* ref,Object* replica);
static void atl_finish_copy(void);

static bool atl_start_comparison(Object* left,Object* right);
static void atl_finish_comparison(Object* left,Object* right);

static bool atl_start_to_string_operation(Object* ref);
static void atl_finish_to_string_operation(Object* ref);

static bool atl_start_to_XML_operation(Object* ref);
static void atl_finish_to_XML_operation(Object* ref);

static bool atl_start_check_invariant_operation(Object* ref);
static void atl_finish_check_invariant_operation(Object* ref);

static bool atl_destroy_needed(Object* ref);
static bool atl_start_destroy_operation(Object* ref);
static void atl_finish_destroy_operation(void);


/*
 * =============================================================================
 *
 * Reference methods
 *
 * =============================================================================
 */

/*
 * Create new object of type '*type',
 * set its value using provided initializer
 * and return registered reference.
 */
Object* create_Object( const Type *type, ... )
{
va_list arg_list;
Object* res;

  va_start( arg_list, type );
  res = va_create( type, &arg_list );
  va_end( arg_list );
  return res;
}

Object *create( const Type *type, ... )
{
va_list arg_list;
/* alloc memory for header and <data> */
void *res = calloc( sizeof(Header) + size_Type( type ), 1 );

#ifdef _DEBUG_REFCNT
  printf( "create %s %p\n", type->name, (char*)res + sizeof(Header) );
#endif

  assertion( res != 0, FORMAT( "create: No memory" ) );

  /* init header */
  ((Header*)res)->type = type;
  ((Header*)res)->weak_ref_cnt = 0;
  ((Header*)res)->hard_ref_cnt = 1;

  /* init <data> */
  va_start( arg_list, type );
  type->init( (char*)res + sizeof(Header), &arg_list );
  va_end( arg_list );

  /* return pointer to <data> */
  return (char*)res + sizeof(Header);
}

Object* va_create_Object( const Type* type, va_list* arg_list )
{
  return va_create(type,arg_list);
}

Object* va_create( const Type* type, va_list* arg_list )
{
/* alloc memory for header and <data> */
void *res = calloc( sizeof(Header) + size_Type( type ), 1 );

#ifdef _DEBUG_REFCNT
  printf( "create %s %p\n", type->name, (char*)res + sizeof(Header) );
#endif

  assertion( res != 0, FORMAT( "create: No memory" ) );

  /* init header */
  ((Header*)res)->type = type;
  ((Header*)res)->weak_ref_cnt = 0;
  ((Header*)res)->hard_ref_cnt = 1;

  /* init <data> */
  type->init( (char*)res + sizeof(Header), arg_list );

  /* return pointer to <data> */
  return (char*)res + sizeof(Header);
}

Object* clone_Object(Object* ref)
{
const Type* t;
void* res;

  if (ref == NULL)
    return NULL;

  CHECK_REFERENCE( clone_Object, ref );
  t = TYPE( ref );

  /* check for presence of copy */
  res = atl_was_copy( ref );
  if (res != NULL)
  {
	inc_hard(res);
	destroy(ref);
    return res;
  }

  /* start copy operation */
  atl_start_copy();

  /* alloc memory for header and <data> initialized by zero */
  res = calloc( sizeof(Header) + size_Type( t ), 1 );

#ifdef _DEBUG_REFCNT
  printf( "clone_Object %s %p\n", t->name, (char*)res + sizeof(Header) );
#endif

  assertion( res != NULL, "clone_Object: No memory" );

  /* init header */
  ((Header*)res)->type = t;
  ((Header*)res)->weak_ref_cnt = 0;
  ((Header*)res)->hard_ref_cnt = 1;

  /* register created copy */
  atl_register_copy( ref, (char*)res + sizeof(Header) );

  /* init <data> */
  t->copy( ref, (char*)res + sizeof(Header) );

  /* destroy argument */
  destroy( ref );

  /* finish copy operation */
  atl_finish_copy();

  /* return pointer to <data> */
  return (char*)res + sizeof(Header);
}

/*
 * Set '*ref' value using provided initializer
 */
void set_Object( Object* ref, ... )
{
    va_list arg_list;

    CHECK_REFERENCE( set, ref );

    /* destroy reference contents before set */
    TYPE( ref )->destroy( ref );

    /* set memory to zero */
    memset( ref, 0, size_Type( TYPE( ref ) ) );

    /* init <data> */
    va_start( arg_list, ref );
    TYPE( ref )->init( ref, &arg_list );
    va_end( arg_list );

    destroy( ref );
}

void set( Object *ref, ... )
{
    va_list arg_list;

    CHECK_REFERENCE( set, ref );

    /* destroy reference contents before set */
    TYPE( ref )->destroy( ref );

    /* set memory to zero */
    memset( ref, 0, size_Type( TYPE( ref ) ) );

    /* init <data> */
    va_start( arg_list, ref );
    TYPE( ref )->init( ref, &arg_list );
    va_end( arg_list );

    destroy( ref );
}

/*
 * Copy object '*src' to '*dst'
 *
 * PRECONDITION: type( src ) == type( dst )
 * - call 'dst' destructor
 * - call copier 'src' -> 'dst'
 * - unregister 'src', 'dst'
 */
void copy_Object( Object *src, Object *dst )
{
  copy(src,dst);
}

void copy(Object* src, Object* dst)
{
  CHECK_REFERENCE( copy, src );
  CHECK_REFERENCE( copy, dst );

  if (src != dst) {

    assertion( getMaximalType(TYPE(src)) == getMaximalType(TYPE(dst))
             , FORMAT( "copy: Different types of arguments: %s %s" )
             , TYPE( src )->name
             , TYPE( dst )->name
             );

    /* destroy destination before copy */
    TYPE( dst )->destroy( dst );

    /* initialize destination with zeroes before copy */
    memset( dst, 0, TYPE( dst )->size );

    /* start copy operation */
    atl_start_copy();
    atl_register_copy( src, dst );

    /* copy */
    TYPE( src )->copy( src, dst );

    /* finish copy operation */
    atl_finish_copy();
  }

  /* destroy registered references */
  destroy( src );
  destroy( dst );
}

/*
 * Compare objects '*left' and '*right'
 *
 * PRECONDITION: type( left ) == type( right )
 * - call comparator 'left' <> 'right'
 * - unregister 'left', 'right'
 */
int compare_Object( Object *left, Object *right )
{
  return compare(left,right);
}

int compare(Object *left, Object *right)
{
int res = 1;

  /* The same references always equal, including NULLs */
  if (left == right)
  {
    destroy( left );
    destroy( right );
    return 0;
  }

  /* NULL references always less than non-NULL */
  if ( left == 0 || right == 0 )
  {
    destroy( left );
    destroy( right );
    return (left != 0) - (right != 0);
  }

  /* compare */
  if (getMaximalType(TYPE(left)) == getMaximalType(TYPE(right)))
   {/* check for cycle */
    if (atl_start_comparison(left,right))
     {res = TYPE( left )->compare( left, right );
      atl_finish_comparison(left,right);
     }
    else
     {/* If there is cycle assume that they are equal */
      res = 0;
     }
   }

  /* destroy registered references */
  destroy( left );
  destroy( right );

  return res;
}

/*
 * Compare objects '*self' and '*ref'
 *
 * PRECONDITION: type( self ) == type( ref )
 * - call comparer 'self' == 'ref'
 * - unregister 'self', 'ref'
 */
bool equals_Object( Object *self, Object *ref )
{
  return equals(self,ref);
}

bool equals(Object* self, Object* ref)
{
  return compare( self, ref ) ? false : true;
}

/*
 * - call stringifier of '*ref'
 * - unregister 'ref'
 */
String* toString_Object( Object *ref )
{
  return toString(ref);
}

String* toString(Object* ref)
{
String* res;

  if( ref != 0 )
   {if (atl_start_to_string_operation( ref ))
     {/* call user defined to_string operation */
      res = TYPE(ref)->to_string( ref );
      atl_finish_to_string_operation( ref );
     }
    else
     {
      res = create_String( "<CYCLIC_PTR>" );
     }
   }
  else
   {
    res = create_String( "<NULL>" );
   }

  destroy( ref );

  return res;
}


/*
 * - call xmlifier of '*ref'
 * - unregister 'ref'
 */

String* to_XML_spec(const char *type, String *value) {
  return create_single_XML_entry("spec", type, value);
}

String* toXML_Object( Object *ref )
{
  return toXML(ref);
}



String* toXML(Object *ref)
{
String *res, *xml_value;
char *string_value = NULL;
bool xml_started_here = ts_start_to_XML();

  if( ref != 0 ) {
	if (atl_start_to_XML_operation( ref )) { /* call user defined to_XML operation */
      xml_value = TYPE(ref)->to_XML(ref);
      atl_finish_to_XML_operation( ref );
    } else {
	  string_value = "<CYCLIC_PTR>";
      xml_value = to_XML_spec(TYPE(ref)->name, create_String(string_value));
	}
  } else {
	string_value = "<NULL>";
    xml_value = to_XML_spec("Object", create_String(string_value));
  }

  if (xml_started_here) {
	StringBuffer *res2 = create_StringBuffer();

	append_StringBuffer( r(res2), "<value><string_value>" );
	if (string_value==NULL) {
	  appendString_StringBuffer( r(res2), XML_encode_String( TYPE(ref)->to_string(ref) ) );
	} else {
	  appendString_StringBuffer( r(res2), XML_encode_String(create_String(string_value)) );
	}
	append_StringBuffer( r(res2), "</string_value>\n<xml_value>" );
	appendString_StringBuffer( r(res2), xml_value); // xml_value destroyed here
	append_StringBuffer( r(res2), "</xml_value></value>" );

	res = toString(res2);
/*    res = replace_String(res, '\n', ' '); */

	ts_finish_to_XML();
  } else {
	res = xml_value;
  }

  destroy( ref );
/*  destroy( xml_value ); -- already destroyed or reference copied to res */
  return res;
}


/*
 * - call invariant checker of '*ref'
 * - unregister 'ref'
 */
bool checkInvariant_Object(Object* ref)
{
  return checkInvariant(ref);
}

bool checkInvariant(Object* ref)
{
bool res;

  if (ref == NULL)
    return true;

  if (atl_start_check_invariant_operation( ref ))
   {/* user defined check invariant function should destroy its parameter */
    res = TYPE(ref)->check_invariant( r(ref) );
    atl_finish_check_invariant_operation( ref );
   }
  else
   {/* this object is under check of invariant */
    res = true;
   }

  destroy( ref );

  return res;
}

/*
 * Unregister weak reference:
 * - decrement weak reference counter
 * - if reference counter == 0,
 *   call object destructor and free memory
 */
void detach_Object(Object* ref)
{
int wrc,hrc,rc;

  if (ref == NULL) return;

  wrc = HEADER( ref )->weak_ref_cnt;
  hrc = HEADER( ref )->hard_ref_cnt;
  rc  = wrc + hrc;

  if (rc <= 0) {
	assertion( false, FORMAT( "detach_Object: Reference counter for reference %p = %d" ), ref, rc );
  }

  if (wrc == 0)
   {/* hard destroy */
    destroy( ref );
    return;
   }
  if ( (hrc == 0)
    && ((wrc == 1) || atl_destroy_needed(ref))
     )
   {
    if (atl_start_destroy_operation(ref))
     {
#ifdef _DEBUG_REFCNT
      printf( "destroy %s %p\n", TYPE( ref )->name, ref );
#endif
      HEADER( ref )->type->destroy( ref );
      free( HEADER(ref) );
      atl_finish_destroy_operation();
     }
   }
  else
   {
    dec_weak( ref );
   }
}

/*
 * Unregister hard reference:
 * - decrement reference counter
 * - if reference counter == 0,
 *   call object destructor and free memory
 */
void destroy_Object(Object *ref)
{
  destroy(ref);
}

void destroy(Object* ref)
{
int wrc,hrc,rc;

  if (ref == NULL) return;

  wrc = HEADER( ref )->weak_ref_cnt;
  hrc = HEADER( ref )->hard_ref_cnt;
  rc  = wrc + hrc;

  assertion( rc > 0, FORMAT( "destroy: Reference counter for reference %p = %d" ), ref, rc );

  if (hrc == 0)
  {/* weak destroy */
    if (wrc != 0) { /* avoid infinite recursion */
	  detach_Object( ref );
	}
    return;
  }
  if (hrc == 1)
   {
    bool destroy_needed = (wrc == 0);
    if (!destroy_needed)
     {
      /* atl_destroy_needed assumes (hrc == 0) and wrc contains the destroying reference  */
      hard2weak( ref );
      /* check for cyclic dependencies */
      destroy_needed = atl_destroy_needed(ref);
     }
    if (destroy_needed)
     {
      if (atl_start_destroy_operation(ref))
       {
#ifdef _DEBUG_REFCNT
        printf( "destroy %s %p\n", TYPE( ref )->name, ref );
#endif
/* !!! SGG: ???
/ 		dec_weak( ref ); */
        HEADER( ref )->type->destroy( ref );
        free( HEADER(ref) );
        atl_finish_destroy_operation();
       }
     }
    else
     {
      /* decrement weak counter */
      dec_weak( ref );
     }
   }
  else
   {
    /* (hrc > 1) */
    dec_hard( ref );
   }
}


/*
 * =============================================================================
 *
 * Default actions
 *
 * =============================================================================
 */

void init_Default( Object *ref, va_list *arg_list )
{
    memset( ref, 0, size_Type( TYPE( ref ) ) );
}

void copy_Default( Object *src, Object *dst )
{
    memcpy( dst, src, size_Type( TYPE( src ) ) );
}

int compare_Default( Object *left, Object *right )
{
    return memcmp( left, right, size_Type( TYPE( left ) ) );
}

String *to_string_Default( Object *ref )
{
    char buf[256];
    sprintf( buf, "%s_%p", TYPE( ref )->name, ref );
    return create_String( buf );
}

String *to_XML_Default( Object *ref )
{
	return to_XML_spec(TYPE(ref)->name, format_String("%s@%p", TYPE( ref )->name, ref));
}

bool check_invariant_Default( Object* ref )
{
  destroy( ref );
  return true;
}

void enumerate_Default( Object *ref, EnumerationCallbackFuncType callback, void* par )
{
  /* no action required */
}

void destroy_Default( Object *ref )
{
  /* no action required */
}

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
void init_Subtype( Object *ref, va_list* arg_list )
{
  TYPE( ref )->base->init( ref, arg_list );

  assertion( checkInvariant( r(ref) ), "init_Subtype: invariant violation" );
}

/*
 * call copy function of base type
 */
void copy_Subtype( Object *src, Object *dst )
{
  TYPE( src )->base->copy( src, dst );
}

/*
 * call compare function of base type
 */
int compare_Subtype( Object *left, Object *right )
{
  return TYPE( left )->base->compare( left, right );
}

/*
 * call to_string function of base type
 */
String *to_string_Subtype( Object *ref )
{
  return TYPE( ref )->base->to_string( ref );
}

/*
 * call to_string function of base type
 */
String *to_XML_Subtype( Object *ref )
{
  return TYPE( ref )->base->to_XML( ref );
}

/*
 * call enumerate function of base type
 */
void enumerate_Subtype( Object *ref, EnumerationCallbackFuncType callback, void* par )
{
  TYPE( ref )->base->enumerate( ref, callback, par );
}

/*
 * call destroy function of base type
 */
void destroy_Subtype( Object *ref )
{
  TYPE( ref )->base->destroy( ref );
}


/*
 * =============================================================================
 *
 * Thread local variables for processing cycles by references
 *
 * =============================================================================
 */
typedef
struct AtlThreadLocalData
{
  long              copy_operation;
  void_HashMap      copy_storage;
  TwoPointerHashSet comparison_storage;
  HashSet           to_string_storage;
  HashSet           to_XML_storage;
  HashSet           check_invariant_storage;
  bool              is_destroy_needed_in_progress;
  IntegerHashMap    destroy_primary_storage;
  HashSet           destroy_secondary_storage;
  long              destroy_operation;
  HashSet           destroying_objects_storage;
} AtlThreadLocalData;


#if defined(PTHREAD)
/* ===================================== *
 * PTHREAD implementation of the thread local data access
 * ===================================== */
#include <pthread.h>
#include <semaphore.h>

static bool atl_tls_key_was_inited = false;
static pthread_key_t atl_tls_key;

/* The worst case storage */
static AtlThreadLocalData atl_thread_local_data;

static void destroyAtlThreadLocalData(AtlThreadLocalData* data);

static void initAtlTlsKey(void)
{
typedef void (*pthread_tls_destructor)(void*);

  pthread_key_create( &atl_tls_key, (pthread_tls_destructor)destroyAtlThreadLocalData );
  atl_tls_key_was_inited = true;
}

static AtlThreadLocalData* createAtlThreadLocalData(void)
{
AtlThreadLocalData* result;

  result = (AtlThreadLocalData*)malloc( sizeof(AtlThreadLocalData) );
  memset( result, 0, sizeof(AtlThreadLocalData) );
  if (pthread_setspecific(atl_tls_key,result) != 0)
   {free(result);
    return NULL;
   }
  return result;
}

static void destroyAtlThreadLocalData(AtlThreadLocalData* data)
{
  free(data);
}

static AtlThreadLocalData* getAtlThreadLocalData(void)
{
static pthread_once_t once_atl_init = PTHREAD_ONCE_INIT;
AtlThreadLocalData* result;

  if (!atl_tls_key_was_inited)
   {pthread_once( &once_atl_init, initAtlTlsKey );
    result = createAtlThreadLocalData();
    return (result != NULL) ? result : &atl_thread_local_data ;
   }
  result = (AtlThreadLocalData*)pthread_getspecific( atl_tls_key );
  if (result != NULL)
    return result;
  result = createAtlThreadLocalData();
  return (result != NULL) ? result : &atl_thread_local_data ;
}

#define ATL_TLS_DATA(name)    (getAtlThreadLocalData()->name)


/* CTesK Source Start */
#elif defined(WIN32) && defined(_MT)
/* ===================================== *
 * WIN32 implementation of the thread local data access
 * ===================================== */


/*
 * The DLL which has __declspec(thread) constructs cannot be loaded by LoadLibrary -
 * only by LdrInitializeProcess during EXE's or other DLL's import resolution.
 */
#ifdef _DLL
  #define WIN32_THREAD_LOCAL_STORAGE
#endif

/** Implementation over __declspec(thread) **/
#if !defined(WIN32_THREAD_LOCAL_STORAGE)


#define THREAD_LOCAL __declspec(thread)

static THREAD_LOCAL AtlThreadLocalData atl_thread_local_data;

#define ATL_TLS_DATA(name)    (atl_thread_local_data.name)


/** Implementation over Windows TLS API **/
#else /* defined(WIN32_THREAD_LOCAL_STORAGE) */

#include "windows.h"


static volatile bool atl_tls_key_was_inited = false;
static DWORD atl_tls_key;

/* The worst case storage */
static AtlThreadLocalData atl_thread_local_data;

static void destroyAtlThreadLocalData(AtlThreadLocalData* data);

static void initAtlTlsKey(void)
{
  atl_tls_key = TlsAlloc();
  atl_tls_key_was_inited = true;
}

static AtlThreadLocalData* createAtlThreadLocalData(void)
{
AtlThreadLocalData* result;

  result = (AtlThreadLocalData*)malloc( sizeof(AtlThreadLocalData) );
  memset( result, 0, sizeof(AtlThreadLocalData) );
  if (TlsSetValue(atl_tls_key,result) == 0)
   {free(result);
    return NULL;
   }
  return result;
}

static void destroyAtlThreadLocalData(AtlThreadLocalData* data)
{
  free(data);
}

static AtlThreadLocalData* getAtlThreadLocalData(void)
{
AtlThreadLocalData* result;

  if (!atl_tls_key_was_inited)
   {
    initAtlTlsKey();
    result = createAtlThreadLocalData();
    return (result != NULL) ? result : &atl_thread_local_data ;
   }
  result = (AtlThreadLocalData*)TlsGetValue( atl_tls_key );
  if (result != NULL)
    return result;
  result = createAtlThreadLocalData();
  return (result != NULL) ? result : &atl_thread_local_data ;
}

#define ATL_TLS_DATA(name)    (getAtlThreadLocalData()->name)


#endif /* defined(WIN32_THREAD_LOCAL_STORAGE) */
/* CTesK Source End */


#else
/*
 * Default: single thread implementation
 */

static AtlThreadLocalData atl_thread_local_data;

#define ATL_TLS_DATA(name)    (atl_thread_local_data.name)


#endif


/*
 * =============================================================================
 *
 * Cycle by reference processing for copy operation
 *
 * =============================================================================
 */
static void atl_start_copy(void)
{
  ATL_TLS_DATA(copy_operation)++;
}

static Object* atl_was_copy(Object* ref)
{
  return atl_get_hash_map( &ATL_TLS_DATA(copy_storage), ref );
}

static void atl_register_copy(Object* ref,Object* replica)
{
  atl_put_hash_map( &ATL_TLS_DATA(copy_storage), ref, replica );
}

static void atl_finish_copy(void)
{
  ATL_TLS_DATA(copy_operation)--;
  if (ATL_TLS_DATA(copy_operation) == 0)
    atl_reinit_hash_map( &ATL_TLS_DATA(copy_storage) );
}


/*
 * =============================================================================
 *
 * Cycle by reference processing for compare operation
 *
 * =============================================================================
 */
static bool atl_start_comparison(Object* left,Object* right)
{
  return atl_add_two_pointer_hash_set( &ATL_TLS_DATA(comparison_storage), left, right );
}

static void atl_finish_comparison(Object* left,Object* right)
{
  atl_remove_two_pointer_hash_set( &ATL_TLS_DATA(comparison_storage), left, right );
}


/*
 * =============================================================================
 *
 * Cycle by reference processing for to_string operation
 *
 * =============================================================================
 */
static bool atl_start_to_string_operation(Object* ref)
{
  return atl_add_hash_set( &ATL_TLS_DATA(to_string_storage), ref );
}

static void atl_finish_to_string_operation(Object* ref)
{
  atl_remove_hash_set( &ATL_TLS_DATA(to_string_storage), ref );
}


/*
 * =============================================================================
 *
 * Cycle by reference processing for to_XML operation
 *
 * =============================================================================
 */
static bool atl_start_to_XML_operation(Object* ref)
{
  return atl_add_hash_set( &ATL_TLS_DATA(to_XML_storage), ref );
}

static void atl_finish_to_XML_operation(Object* ref)
{
  atl_remove_hash_set( &ATL_TLS_DATA(to_XML_storage), ref );
}


/*
 * =============================================================================
 *
 * Cycle by reference processing for check invariant operation
 *
 * =============================================================================
 */
static bool atl_start_check_invariant_operation(Object* ref)
{
  return atl_add_hash_set( &ATL_TLS_DATA(check_invariant_storage), ref );
}

static void atl_finish_check_invariant_operation(Object* ref)
{
  atl_remove_hash_set( &ATL_TLS_DATA(check_invariant_storage), ref );
}


/*
 * =============================================================================
 *
 * Cycle by reference processing for destroy operation
 *
 * =============================================================================
 */
typedef struct SecondaryEnumeratePar SecondaryEnumeratePar;
struct SecondaryEnumeratePar
{
  Object* pretender_to_destroy;
  bool    path_founded;
};

static bool atl_destroy_primary_enumerate( void* ref, void* par )
{
int* value;
int wrc;

  if (ref == NULL)
    return true;
  if (HEADER(ref)->hard_ref_cnt > 0)
    return true;
  wrc = HEADER(ref)->weak_ref_cnt;
  if (wrc > 1)
   {IntegerHashMap* ptr_destroy_primary_storage = &ATL_TLS_DATA(destroy_primary_storage);
    value = atl_get_integer_hash_map( ptr_destroy_primary_storage, ref );
    if (value != NULL)
     {(*value)--;
      return true;
     }
    atl_put_integer_hash_map( ptr_destroy_primary_storage, ref, wrc-1 );
   }
  TYPE( ref )->enumerate( ref, atl_destroy_primary_enumerate, par );
  return true;
}

static bool atl_destroy_secondary_enumerate( void* ref, SecondaryEnumeratePar* par )
{
  assertion( par != NULL, "atl_destroy_secondary_enumerate: par is NULL" );

  if (ref == NULL)
    return !par->path_founded;
  if (ref == par->pretender_to_destroy)
   {
    par->path_founded = true;
    return false;
   }
  else if (atl_add_hash_set( &ATL_TLS_DATA(destroy_secondary_storage), ref ))
   {TYPE( ref )->enumerate( ref, (EnumerationCallbackFuncType)atl_destroy_secondary_enumerate, par );
    atl_remove_hash_set( &ATL_TLS_DATA(destroy_secondary_storage), ref );
   }
  return !par->path_founded;
}

static bool atl_is_destroy_needed(Object* ref)
{
IntegerHashMap* ptr_destroy_primary_storage;
SecondaryEnumeratePar enumerate_par;
IntegerHashMapElement* tmp;
int* value;
int i;

  ptr_destroy_primary_storage = &ATL_TLS_DATA(destroy_primary_storage);
  atl_reinit_integer_hash_map( ptr_destroy_primary_storage );
  atl_put_integer_hash_map( ptr_destroy_primary_storage, ref, HEADER(ref)->weak_ref_cnt - 1 );
  /* Enumerate referenced objectes */
  TYPE( ref )->enumerate( ref, atl_destroy_primary_enumerate, NULL );
  value = atl_get_integer_hash_map( ptr_destroy_primary_storage, ref );
  if (*value > 0)
    return false;
  /* Search path from nonzero referenced object to ref */
  enumerate_par.pretender_to_destroy = ref;
  enumerate_par.path_founded = false;
  for(i=0;i<ATL_HASH_SIZE;i++)
   {if (ptr_destroy_primary_storage->data[i].key == NULL)
      continue;
    for(tmp=&ptr_destroy_primary_storage->data[i];tmp!=NULL;tmp=tmp->next)
     {if ((tmp->key != ref) && (tmp->value > 0))
       {TYPE( ref )->enumerate( ref, (EnumerationCallbackFuncType)atl_destroy_secondary_enumerate, &enumerate_par );
        if (enumerate_par.path_founded)
          return false;
       }
     }
   }
  return true;
}

static bool atl_destroy_needed(Object* ref)
{
bool res;

  if (ATL_TLS_DATA(is_destroy_needed_in_progress))
   {assertion( false, "destroy has been called inside enumerate function" );
    /* return false; */
   }
  /* check if the ref is destroying */
  if (atl_contains_hash_set( &ATL_TLS_DATA(destroying_objects_storage), ref ))
    return false;

  ATL_TLS_DATA(is_destroy_needed_in_progress) = true;
  res = atl_is_destroy_needed( ref );
  ATL_TLS_DATA(is_destroy_needed_in_progress) = false;
  return res;
}

static bool atl_start_destroy_operation(Object* ref)
{
  if (!atl_add_hash_set( &ATL_TLS_DATA(destroying_objects_storage), ref ))
    return false;
  ATL_TLS_DATA(destroy_operation)++;
  return true;
}

static void atl_finish_destroy_operation(void)
{
  ATL_TLS_DATA(destroy_operation)--;
  if (ATL_TLS_DATA(destroy_operation) == 0)
    atl_reinit_hash_set( &ATL_TLS_DATA(destroying_objects_storage) );
}


