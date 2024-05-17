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
 * PROJECT:     CTesK
 * FILE:        convex.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    26 May 2003
 * DESCRIPTION: Implementation of functions needed
 *              for work with convex types.
 */

#include <string.h>
#include <stdlib.h>

#include <ts/convex.h>
#include <ts/ts.h>
#include <atl/map.h>
#include <atl/set.h>
#include <atl/void_ast.h>
#include <atl/hash.h>
#include <atl/stringbuffer.h>


static String* to_string_sectype(SECTypeDesc* desc,void* ptr);


#define ALLOW_SIMPLIFIED_XML
#define STRING_THRESHOLD	15
#define STRING_ABBR		10


/********************************************************************/
/**                         Sub Type Desc                          **/
/********************************************************************/
SECTypeDesc* ts_base_type_desc(SECTypeDesc* type_desc)
{
  assertion( type_desc!=NULL, "ts_base_type_desc: NULL type desc" );

  if (type_desc->kind != subType)
    return type_desc;
  return ts_base_type_desc( ((SubTypeDesc*)type_desc)->base );
}


/********************************************************************/
/**                     Auxiliary Functions                        **/
/********************************************************************/
#define FIELD(PTR,OFFSET)            ((void*)(((char*)PTR)+OFFSET))
#define POINTER(PTR)                 (*(void**)PTR)
#define ELEMENT(PTR,I,SIZE)          (((char*)PTR)+(I*SIZE))

static size_t sizeof_sectype(SECTypeDesc* desc)
{
  assertion( desc!=NULL, "sizeof_sectype: desc is NULL" );
  switch (desc->kind)
   {case simpleType:
      return ((SimpleTypeDesc*)desc)->sizeOf;
    case structType:
      return ((StructTypeDesc*)desc)->sizeOf;
    case pointerType:
      return sizeof (void*);
    case arrayType:
      return ((ArrayTypeDesc*)desc)->numElements*((ArrayTypeDesc*)desc)->sizeOfElement;
    case specType:
      return sizeof (void*);
    case subType:
      return sizeof_sectype( ((SubTypeDesc*)desc)->base );
    default:
      assertion(false,"sizeof_sectype#default");
      return 0;
   }
}

static bool is_specification_reference(SECTypeDesc* desc)
{
  switch (desc->kind)
   {case specType:
      return true;
    case subType:
      return is_specification_reference(((SubTypeDesc*)desc)->base);
    default:
      return false;
   }
}

static bool is_convex_type(SECTypeDesc* desc);

static bool is_convex_struct_type(StructTypeDesc* desc)
{
int i;

  for(i=0;i<desc->numFields;i++)
    if (is_convex_type(desc->fields[i].type_desc))
      return true;
  return false;
}

static bool is_convex_type(SECTypeDesc* desc)
{
  assertion( desc!=NULL, "is_convex_type: desc is NULL" );
  switch (desc->kind)
   {case simpleType:
      return false;
    case structType:
      return is_convex_struct_type((StructTypeDesc*)desc);
    case pointerType:
      return true;
    case arrayType:
      return is_convex_type(((ArrayTypeDesc*)desc)->base);
    case specType:
      return true;
    case subType:
      return is_convex_type( ((SubTypeDesc*)desc)->base );
    default:
      assertion(false,"is_convex_type#default");
      return false;
   }
}

static
bool ts_was_sectype_correctly_copied(SECTypeDesc* copied,SECTypeDesc* current);

static
bool ts_was_pointers_correctly_copied
         (PointerTypeDesc* copied,int copied_level,PointerTypeDesc* current,int current_level)
{
SECTypeDesc* base;

  if (copied_level == current_level)
    return ts_was_sectype_correctly_copied( copied->base, current->base );
  if (copied_level > current_level)
   {base = ts_base_type_desc(current->base);
    if (base->kind == pointerType)
      return ts_was_pointers_correctly_copied( copied,                 copied_level - current_level,
                                               (PointerTypeDesc*)base, ((PointerTypeDesc*)base)->level
                                             );
    return false;
   }
  base = ts_base_type_desc(copied->base);
  if (base->kind == pointerType)
    return ts_was_pointers_correctly_copied( (PointerTypeDesc*)base, ((PointerTypeDesc*)base)->level,
                                             current,                current_level - copied_level
                                           );
  return false;
}

static
bool ts_was_sectype_correctly_copied(SECTypeDesc* copied,SECTypeDesc* current)
{
  copied  = ts_base_type_desc(copied);
  current = ts_base_type_desc(current);
  if (copied == current)
    return true;
  switch (copied->kind)
   {case simpleType:
      if (is_convex_type(current))
        return false;
      return (sizeof_sectype(current) <= ((SimpleTypeDesc*)copied)->sizeOf);
    case structType:
      return false;
    case pointerType:
      if (current->kind != pointerType)
        return false;
      return ts_was_pointers_correctly_copied( (PointerTypeDesc*)copied,  ((PointerTypeDesc*)copied)->level,
                                               (PointerTypeDesc*)current, ((PointerTypeDesc*)current)->level
                                             );
    case arrayType:
      if (is_convex_type(((ArrayTypeDesc*)copied)->base) || is_convex_type(current))
        return false;
      return (sizeof_sectype(current) < sizeof_sectype(copied));
    case specType:
      return (current->kind == specType);
    default:
      assertion(false,"isCompatible#default");
   }
  return false;
}


/********************************************************************/
/**                    Copy Operation Context                      **/
/********************************************************************/
#define CopyOperationHashSize  128

static unsigned convex_pointer_hash_function(void* ptr)
{
unsigned i,res;

  res = 0;
  for(i=0;i<sizeof(ptr);i++)
    res ^= ((char*)&ptr)[i];
  return res % CopyOperationHashSize;
}

typedef struct CopyOperationContextElement CopyOperationContextElement;
struct CopyOperationContextElement
{
  void* source;
  void* replica;
  PointerTypeDesc* desc;
  int   level;
  CopyOperationContextElement* next;
};

typedef struct CopyOperationContext CopyOperationContext;
struct CopyOperationContext
{
  CopyOperationContextElement data[CopyOperationHashSize];
  long nestedCounter;
};

static
void ts_reinit_CopyOperationContext(CopyOperationContext* context)
{
CopyOperationContextElement *tmp,*coffin;
int i;

  for(i=0;i<CopyOperationHashSize;i++)
   {for(tmp=context->data[i].next;tmp!=NULL;)
     {coffin = tmp;
      tmp = tmp->next;
      free(coffin);
     }
    context->data[i].source  = NULL;
    context->data[i].replica = NULL;
    context->data[i].desc    = NULL;
    context->data[i].level   = 0;
    context->data[i].next    = NULL;
   }
}

static
CopyOperationContextElement* ts_was_copied(CopyOperationContext* context,void* ptr)
{
CopyOperationContextElement* tmp;

  for(tmp=&(context->data[convex_pointer_hash_function(ptr)]);tmp!=NULL;tmp=tmp->next)
    if (tmp->source == ptr)
      return tmp;
  return NULL;
}

static
void* ts_was_correctly_copied(CopyOperationContext* context,void* ptr,PointerTypeDesc* desc,int level)
{
CopyOperationContextElement* tmp;

  for(tmp=&(context->data[convex_pointer_hash_function(ptr)]);tmp!=NULL;tmp=tmp->next)
   {if (tmp->source == ptr)
      if (ts_was_pointers_correctly_copied( tmp->desc, tmp->level, desc, level ))
        return tmp->replica;
   }
  return NULL;
}

static
void ts_register_replica
         (CopyOperationContext* context,void* source,void* replica,PointerTypeDesc* desc,int level)
{
CopyOperationContextElement *base,*tmp;

  base = &(context->data[convex_pointer_hash_function(source)]);
  if (base->source == NULL)
   {base->source  = source;
    base->replica = replica;
    base->desc    = desc;
    base->level   = level;
    return;
   }
  tmp = (CopyOperationContextElement*)malloc(sizeof(CopyOperationContextElement));
  assertion( tmp != NULL, "ts_register_replica: not enough memory" );

  tmp->source  = source;
  tmp->replica = replica;
  tmp->desc    = desc;
  tmp->level   = level;
  tmp->next  = base->next;
  base->next = tmp;
}


/********************************************************************/
/**                        Copy Function                           **/
/********************************************************************/
CopyOperationContext context_CopyOperation;

static CopyOperationContext* getCopyOperationContext(void)
{
  /* Temporary: thread local contexts not yet implemented */
/**
  if (ts_is_main_thread())
    return &context_CopyOperation;
**/
  return &context_CopyOperation;
}

static void copy_sectype(SECTypeDesc* desc,void* dst,void* src,bool weak);

static void copy_simple_sectype(SimpleTypeDesc* desc,void* dst,void* src,bool weak)
{
  memcpy(dst,src,desc->sizeOf);
}

static void copy_struct_sectype(StructTypeDesc* desc,void* dst,void* src,bool weak)
{
int i;

  for(i=0;i<desc->numFields;i++)
    copy_sectype(desc->fields[i].type_desc,FIELD(dst,desc->fields[i].offset),FIELD(src,desc->fields[i].offset),weak);
}

static void copy_pointer_sectype(PointerTypeDesc* desc,void* dst,void* src,bool weak)
{
void *dst_ptr,*src_ptr;
int i;

  dst_ptr = dst;
  src_ptr = src;
  for(i=desc->level;i>0;i--)
   {if (POINTER(src_ptr) == NULL)
     {POINTER(dst_ptr) = NULL;
      return;
     }
    POINTER(dst_ptr) = ts_was_correctly_copied( getCopyOperationContext(), POINTER(src_ptr), desc, i );
    if (POINTER(dst_ptr) != NULL)
      return;
    if (i == 1)
      break;

    POINTER(dst_ptr) = malloc(sizeof(void*));

#ifdef _DEBUG_CONVEX
    printf("Debug: malloc(%p)\n",POINTER(dst_ptr));
#endif

    assertion(POINTER(dst_ptr) != NULL,"copy_pointer: not enough memory");
    ts_register_replica( getCopyOperationContext(), POINTER(src_ptr), POINTER(dst_ptr), desc, i );

    dst_ptr = POINTER(dst_ptr);
    src_ptr = POINTER(src_ptr);
   }

  POINTER(dst_ptr) = malloc(sizeof_sectype(desc->base));
  assertion(POINTER(dst_ptr) != NULL,"copy_pointer: not enough memory");
  ts_register_replica( getCopyOperationContext(), POINTER(src_ptr), POINTER(dst_ptr), desc, 1 );

#ifdef _DEBUG_CONVEX
    printf("Debug: malloc(%p)\n",POINTER(dst_ptr));
#endif

  copy_sectype(desc->base,POINTER(dst_ptr),POINTER(src_ptr),weak);
}

static void copy_array_sectype(ArrayTypeDesc* desc,void* dst,void* src,bool weak)
{
int i;

  for(i=0;i<desc->numElements;i++)
    copy_sectype(desc->base,ELEMENT(dst,i,desc->sizeOfElement),ELEMENT(src,i,desc->sizeOfElement),weak);
}

static void copy_spec_sectype(SpecTypeDesc* desc,void* dst,void* src,bool weak)
{
  if (POINTER(src) == NULL)
   {POINTER(dst) = NULL;
    return;
   }
  r_take(POINTER(src));
  if (weak)
   {
    POINTER(dst) = r_hard2weak(clone_Object(r(POINTER(src))));
   }
  else
   {
    POINTER(dst) = clone_Object(r(POINTER(src)));
   }
  r_release(POINTER(src));
}

static void copy_sectype(SECTypeDesc* desc,void* dst,void* src,bool weak)
{
  switch (desc->kind)
   {case simpleType:
      copy_simple_sectype((SimpleTypeDesc*)desc,dst,src,weak);
      return;
    case structType:
      copy_struct_sectype((StructTypeDesc*)desc,dst,src,weak);
      return;
    case pointerType:
      copy_pointer_sectype((PointerTypeDesc*)desc,dst,src,weak);
      return;
    case arrayType:
      copy_array_sectype((ArrayTypeDesc*)desc,dst,src,weak);
      return;
    case specType:
      copy_spec_sectype((SpecTypeDesc*)desc,dst,src,weak);
      return;
    case subType:
      copy_sectype(ts_base_type_desc(desc),dst,src,weak);
      return;
    default:
      assertion(false,"copy_sectype#default");
   }
}

void ts_start_copy_sectype()
{
  getCopyOperationContext()->nestedCounter++;
}

void ts_finish_copy_sectype()
{
CopyOperationContext* context;

  context = getCopyOperationContext();
  context->nestedCounter--;
  if (context->nestedCounter == 0)
    ts_reinit_CopyOperationContext(context);
}

void ts_copy_sectype(SECTypeDesc* desc,void* dst,void* src,bool weak)
{
  assertion( desc!=NULL,"ts_copy_sectype: desc is NULL");
  assertion( dst!=NULL, "ts_copy_sectype: dst is NULL" );
  assertion( src!=NULL, "ts_copy_sectype: src is NULL" );

  ts_start_copy_sectype();
  copy_sectype(desc,dst,src,weak);
  ts_finish_copy_sectype();
}


/********************************************************************/
/**                       Equals Function                          **/
/********************************************************************/
TwoPointerHashSet context_EqualOperation;

static bool ts_convex_start_comparison(Object* left,Object* right)
{
  return atl_add_two_pointer_hash_set( &context_EqualOperation, left, right );
}

static void ts_convex_finish_comparison(Object* left,Object* right)
{
  atl_remove_two_pointer_hash_set( &context_EqualOperation, left, right );
}


static bool equals_sectype(SECTypeDesc* desc,void* left,void* right);

static bool equals_simple_sectype(SimpleTypeDesc* desc,void* left,void* right)
{
  return (memcmp(left,right,desc->sizeOf) == 0);
}

static bool equals_struct_sectype(StructTypeDesc* desc,void* left,void* right)
{
int i;

  if (left == right)
    return true;
  for(i=0;i<desc->numFields;i++)
   {if (!equals_sectype(desc->fields[i].type_desc,
                        FIELD(left,desc->fields[i].offset),
                        FIELD(right,desc->fields[i].offset)
                       )
       )
      return false;
   }
  return true;
}

static bool equals_pointer_sectype(PointerTypeDesc* desc,void* left,void* right)
{
bool res;
void *left_ptr,*right_ptr;
int i,bound;

  left_ptr  = left;
  right_ptr = right;
  // result evaluation
  for(i=desc->level;i>0;i--)
   {if (POINTER(left_ptr) == POINTER(right_ptr))
     {bound = i;
      res = true;
      break;
     }
    if ((POINTER(left_ptr) == NULL) || (POINTER(right_ptr) == NULL))
     {bound = i;
      res = false;
      break;
     }
    if (!ts_convex_start_comparison( POINTER(left_ptr), POINTER(right_ptr) ))
     {bound = i;
      res = true;
      break;
     }
    if (i == 1)
     {bound = 0;
      res = equals_sectype(desc->base,POINTER(left_ptr),POINTER(right_ptr));
      break;
     }
    left_ptr  = POINTER(left_ptr);
    right_ptr = POINTER(right_ptr);
   }
  // context restore
  for(i=desc->level;i>bound;i--)
   {ts_convex_finish_comparison( POINTER(left_ptr), POINTER(right_ptr) );
    left_ptr  = POINTER(left_ptr);
    right_ptr = POINTER(right_ptr);
   }
  return res;
}

static bool equals_array_sectype(ArrayTypeDesc* desc,void* left,void* right)
{
int i;

  for(i=0;i<desc->numElements;i++)
    if (!equals_sectype(desc->base,ELEMENT(left,i,desc->sizeOfElement),ELEMENT(right,i,desc->sizeOfElement)))
      return false;
  return true;
}

static bool equals_spec_sectype(SpecTypeDesc* desc,void* left,void* right)
{
  if (POINTER(left) == POINTER(right))
    return true;
  if ((POINTER(left) == NULL) || (POINTER(right) == NULL))
    return false;
  return equals(r(POINTER(left)),r(POINTER(right)));
}

static bool equals_sectype(SECTypeDesc* desc,void* left,void* right)
{
  switch (desc->kind)
   {case simpleType:
      return equals_simple_sectype((SimpleTypeDesc*)desc,left,right);
    case structType:
      return equals_struct_sectype((StructTypeDesc*)desc,left,right);
    case pointerType:
      return equals_pointer_sectype((PointerTypeDesc*)desc,left,right);
    case arrayType:
      return equals_array_sectype((ArrayTypeDesc*)desc,left,right);
    case specType:
      return equals_spec_sectype((SpecTypeDesc*)desc,left,right);
    case subType:
      return equals_sectype(ts_base_type_desc(desc),left,right);
    default:
      assertion(false,"equals_sectype#default");
      return false;
   }
}


bool ts_equals_sectype(SECTypeDesc* desc,void* left,void* right)
{
  assertion( desc!=NULL,  "ts_equals_sectype: desc is NULL"  );
  assertion( left!=NULL,  "ts_equals_sectype: left is NULL"  );
  assertion( right!=NULL, "ts_equals_sectype: right is NULL" );

  return equals_sectype(desc,left,right);
}


/********************************************************************/
/**                      ToXML Function                         **/
/********************************************************************/
HashSet context_ToXMLOperation;

// Not thread-safe!!!
/* TODO:
 * Merge toXML and toString from object and from convex
 * Make it thread-safe (look at function getAtlThreadLocalData)
 */
static bool ts_convex_start_to_XML_operation(Object* ref)
{
  return atl_add_hash_set( &context_ToXMLOperation, ref );
}
static void ts_convex_finish_to_XML_operation(Object* ref)
{
  atl_remove_hash_set( &context_ToXMLOperation, ref );
}

static int inside_XML;

bool ts_start_to_XML()
{
  return ts_convex_start_to_XML_operation( &inside_XML );
}
void ts_finish_to_XML()
{
  ts_convex_finish_to_XML_operation( &inside_XML );
}


static String* to_XML_sectype(SECTypeDesc* desc,void* ptr);

/*
  valid conversion:
    'c' -- char
    'd' -- int
    'u' -- unsigned
    'f' -- float/double (decimal)
    'p' -- pointer

  valid length:
    ' ' -- none
    'h' -- short
    'l' -- long
    'L' -- long long
	'M' -- __int64
 */
static String* to_XML_simple_sectype(SimpleTypeDesc* desc,void* ptr)
{
  if (desc->format) {
	if ( strlen(desc->format) != 3 ) {
      fault( false, "invalid format '%s'", desc->format);
    } else {
      char len = desc->format[1];
      char conv = desc->format[2];
      switch (conv)
      {
        case 'c':
          if (len == ' ') {
			if (*(char *)ptr == 0)	// Special case
				return create_String("<object kind=\"simple\" type=\"char\" text=\"&#x00;\"/>");
			return create_single_XML_entry("simple", "char", format_String("%c", *(char *)ptr));
		  }
          break;
        case 'u':
          switch (len)
           {
            case ' ':
              return create_single_XML_entry("simple", "unsigned int", format_String("%u", *(int *)ptr));
            case 'h':
			  if (desc->format[0] == ' ')
                return create_single_XML_entry("simple", "unsigned short", format_String ("%hu", *(unsigned short *)ptr));
			  if (desc->format[0] == 'h')
                return create_single_XML_entry("simple", "unsigned char", format_String ("%hhu", *(unsigned char *)ptr));
			  break;
            case 'l':
              return create_single_XML_entry("simple", "unsigned long", format_String("%lu", *(unsigned long *)ptr));
#ifdef __STDC__
            case 'L':
              return create_single_XML_entry("simple", "unsigned long long", format_String("%llu", *(unsigned long long *)ptr));
#endif
#ifdef _MSC_VER
            case 'M':
              return create_single_XML_entry("simple", "unsigned __int64", format_String("%I64u", *(unsigned __int64 *)ptr));
#endif
            }
          break;
        case 'd':
          switch (len)
            {
             case ' ':
              return create_single_XML_entry("simple", "int", format_String("%d", *(int *)ptr));
             case 'h':
			  if (desc->format[0] == ' ')
                return create_single_XML_entry("simple", "short", format_String ("%hd", *(short *)ptr));
			  if (desc->format[0] == 'h')
                return create_single_XML_entry("simple", "signed char", format_String ("%hhd", *(signed char *)ptr));
			  break;
             case 'l':
              return create_single_XML_entry("simple", "long", format_String("%d", *(long *)ptr));
#ifdef __STDC__
             case 'L':
              return create_single_XML_entry("simple", "long long", format_String("%lld", *(long long *)ptr));
#endif
#ifdef _MSC_VER
            case 'M':
              return create_single_XML_entry("simple", "__int64", format_String("%I64d", *(__int64 *)ptr));
#endif
            }
          break;
        case 'f':
          switch (len)
           {
            case ' ':
              return create_single_XML_entry("simple", "double", format_String("%f", *(double *)ptr));
            case 'h':
              return create_single_XML_entry("simple", "float", format_String("%f", *(float *)ptr));
            case 'L':
              return create_single_XML_entry("simple", "long double", format_String("%Lf", *(long double *)ptr));
           }
          break;
        case 'p':
          if (len == ' ')	return create_single_XML_entry("simple", "pointer", format_String("%p", *(void **)ptr));
          break;
        default:
          fault (0, "invalid conversion '%c'", conv);
      } // switch
      fault (0, "invalid length modifier '%c' for conversion '%c'", len, conv);
    }
	return NULL;
  }
  else // desc->format
  {
    return create_single_XML_entry("simple", "unknown", valueOfBytes_String( (const char*)ptr, desc->sizeOf ));
  }
}

static String* to_XML_struct_sectype(StructTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i;

  res = create_StringBuffer();
  appendString_StringBuffer( r(res),
	  format_String("<object kind=\"struct\" type=\"struct\" text=\"struct %s\">\n", desc->name) );

  for( i = 0; i < desc->numFields; i++ ) {
    appendString_StringBuffer( r(res), format_String("<label text=\"%s\"/>", desc->fields[i].name) );
    appendString_StringBuffer( r(res), to_XML_sectype(desc->fields[i].type_desc,FIELD(ptr,desc->fields[i].offset)) );
    append_StringBuffer( r(res), "\n" );
  }
  append_StringBuffer( r(res), "</object>" );

  return toString(res);
}

static String* to_XML_pointer_sectype(PointerTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i,bound,count;

  count = 0;
  // result evaluation
  res = create_StringBuffer();
  for( i = desc->level; i > 0; i-- )
  {
    if (POINTER(ptr) == NULL) {
	  bound = i;
      append_StringBuffer( r(res), "<object kind=\"pointer\" type=\"pointer\" text=\"&lt;NULL&gt;\"/>\n" );
      break;
    }
    if (!ts_convex_start_to_XML_operation( POINTER(ptr) )) {
      bound = i;
      append_StringBuffer( r(res), "<object kind=\"pointer\" type=\"pointer\" text=\"Pointer\"/>\n" );
      break;
    }
    append_StringBuffer( r(res), "<object kind=\"pointer\" type=\"pointer\" text=\"Pointer to\">\n" );
	count++;
    if (i == 1) {
      bound = 0; /* (*) */
      appendString_StringBuffer( r(res), to_XML_sectype(desc->base,POINTER(ptr)) );
      break;
    }
    ptr = POINTER(ptr);
  }

  for(i=0; i<count; i++) {
    appendString_StringBuffer( r(res), create_String("</object>\n") );
  }

  // context restore
  for(i=desc->level; i>bound; i--) {
    if (ptr == NULL) break;	/* May happen if bound==0 after (*) and POINTER(ptr)==NULL */
    ts_convex_finish_to_XML_operation( POINTER(ptr) );
    ptr = POINTER(ptr);
  }
  return toString(res);
}

static String* to_XML_array_sectype(ArrayTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i;

  res = create_StringBuffer();
  appendString_StringBuffer( r(res), format_String("<object kind=\"array\" type=\"array\" text=\"Array (length=%d)\">\n", desc->numElements) );

  for(i=0; i<desc->numElements; i++) {
	appendString_StringBuffer( r(res), format_String("<object kind=\"array\" type=\"array element\" text=\"element [%d]\">\n", i));
    appendString_StringBuffer( r(res), to_XML_sectype(desc->base,ELEMENT(ptr,i,desc->sizeOfElement)) );
    append_StringBuffer( r(res), "</object>\n" );
  }

  append_StringBuffer( r(res), "</object>\n" );

  return toString(res);
}

static String* to_XML_spec_sectype(SpecTypeDesc* desc,void* ptr)
{
String* res;

  if (POINTER(ptr) == NULL)
	return create_single_XML_entry("spec", desc->type->name, create_String("<NULL>"));

  r_take(POINTER(ptr));
  res = toXML(r(POINTER(ptr)));
  r_release(POINTER(ptr));
  return res;
}

static String* to_XML_sectype(SECTypeDesc* desc,void* ptr)
{
  if (desc->customToXML) {
	return (*desc->customToXML)(ptr);
  }

  switch (desc->kind)
   {case simpleType:
      return to_XML_simple_sectype((SimpleTypeDesc*)desc,ptr);
    case structType:
      return to_XML_struct_sectype((StructTypeDesc*)desc,ptr);
    case pointerType:
      return to_XML_pointer_sectype((PointerTypeDesc*)desc,ptr);
    case arrayType:
      return to_XML_array_sectype((ArrayTypeDesc*)desc,ptr);
    case specType:
      return to_XML_spec_sectype((SpecTypeDesc*)desc,ptr);
    case subType:
      return to_XML_sectype(ts_base_type_desc(desc),ptr);
    default:
      assertion(false,"to_XML_sectype#default");
      return NULL;
   }
}

/** Format XML entry "<object>" with trailing slash (/) */
String* create_single_XML_entry(const char *kind, const char *type, String *value)
{
  String *f_value = XML_encode_String(value);	// value is destroyed here
  String *res = format_String("<object kind=\"%s\" type=\"%s\" text=\"%s\"/>",
		kind, type, toCharArray_String(r(f_value)));

  destroy(f_value);
  return res;
}

#ifdef ALLOW_SIMPLIFIED_XML

static String *format_shortened_string(String *s)
{
	StringBuffer *res = create_StringBuffer();
	append_StringBuffer(      r(res), "<value><string_value>");
	appendString_StringBuffer(r(res), XML_encode_String(r(s)) );
	append_StringBuffer(      r(res), "</string_value><xml_value>");
	append_StringBuffer(      r(res), "<object kind=\"spec\" type=\"String\" text=\"");
	appendString_StringBuffer(r(res), XML_encode_String( substring_String(r(s), 0, STRING_ABBR) ));
//	append_StringBuffer(      r(res), "&#133;\">");	// #133 is '...'
	append_StringBuffer(      r(res), "...\">");
	appendString_StringBuffer(r(res), to_XML_spec("String", r(s)));
	append_StringBuffer(      r(res), "</object></xml_value></value>");

	destroy(s);
	return toString(res);
}

static HashSet primitive_specs;
static bool primitive_specs_initted = false;

extern Type type_Char, type_Double, type_Float, type_Integer, type_UInteger, type_Long, type_ULong, type_Short, type_UShort;

static bool is_primitive_spec(const Type *type) {
  if (!primitive_specs_initted) {
	atl_reinit_hash_set(&primitive_specs);
	atl_add_hash_set(&primitive_specs, (void*)&type_Char);
	atl_add_hash_set(&primitive_specs, (void*)&type_Double);
	atl_add_hash_set(&primitive_specs, (void*)&type_Float);
	atl_add_hash_set(&primitive_specs, (void*)&type_Integer);
	atl_add_hash_set(&primitive_specs, (void*)&type_UInteger);
	atl_add_hash_set(&primitive_specs, (void*)&type_Long);
	atl_add_hash_set(&primitive_specs, (void*)&type_ULong);
	atl_add_hash_set(&primitive_specs, (void*)&type_Short);
	atl_add_hash_set(&primitive_specs, (void*)&type_UShort);
	primitive_specs_initted = true;
  }
  return atl_contains_hash_set(&primitive_specs, (void*)type);
}
#endif

String* ts_to_XML_sectype(SECTypeDesc* desc, void* ptr)
{
  assertion( desc!=NULL, "ts_to_XML_sectype: desc is NULL" );
  assertion( ptr!=NULL,  "ts_to_XML_sectype: ptr is NULL"  );

  if (ts_convex_start_to_XML_operation( ptr ))
  {
    String* res = NULL;
    if (ts_start_to_XML())
	{
#ifdef ALLOW_SIMPLIFIED_XML
	  if(desc->kind == simpleType)
	  { // Special case: skip XML formatting for primitive types
		res = /*XML_encode_String(*/ ts_to_string_sectype(desc, ptr) ;
	  } else if (  desc->kind == specType
				&& ((SpecTypeDesc*)desc)->type == &type_String )
	  { // Special cases: skip formatting for short Strings, special format for long ones
		  void *str = POINTER(ptr);
		  if (str != NULL && length_String( r(str) ) > STRING_THRESHOLD)
		    res = format_shortened_string( r(str) );
		  else res = /*XML_encode_String(*/ ts_to_string_sectype(desc, ptr) ;
	  } else if (  desc->kind == specType
				&& is_primitive_spec( ((SpecTypeDesc*)desc)->type) )
	  { // Special case: specification of primitive types (also supports NULL)
        res = /*XML_encode_String(*/ ts_to_string_sectype(desc, ptr) ;
	  } else
#endif
	  {
		StringBuffer *r2 = create_StringBuffer();
        append_StringBuffer(      r(r2), "<value><string_value>");
        appendString_StringBuffer(r(r2), XML_encode_String(ts_to_string_sectype(desc, ptr)));
        append_StringBuffer(      r(r2), "</string_value>\n<xml_value>");
        appendString_StringBuffer(r(r2), to_XML_sectype(desc, ptr));
        append_StringBuffer(      r(r2), "</xml_value></value>");
		res = toString(r2);
//        res = replace_String(res, '\n', ' ');
	  }
      ts_finish_to_XML();
	}
	else { // ts_start_to_XML()
	  res = to_XML_sectype(desc, ptr);
	}

    ts_convex_finish_to_XML_operation( ptr );
    return res;
  }
  else { // ts_convex_start_to_XML_operation(ptr)
    return to_XML_spec("Object", create_String("<CYCLIC_PTR>"));
  }
}


/********************************************************************/
/**                      ToString Function                         **/
/********************************************************************/
HashSet context_ToStringOperation;

static bool ts_convex_start_to_string_operation(Object* ref)
{
  return atl_add_hash_set( &context_ToStringOperation, ref );
}

static void ts_convex_finish_to_string_operation(Object* ref)
{
  atl_remove_hash_set( &context_ToStringOperation, ref );
}


/*
  valid conversion:
    'c' -- char
    'd' -- int
    'u' -- unsigned
    'f' -- float/double (decimal)
    'p' -- pointer

  valid length:
    ' ' -- none
    'h' -- short
    'l' -- long
    'L' -- long long
	'M' -- __int64
 */
static String* to_string_simple_sectype(SimpleTypeDesc* desc,void* ptr)
{
  if (desc->format)
   {if ( strlen(desc->format) != 3 )
      fault( false, "invalid format '%s'", desc->format);
    else
     {
      char len = desc->format[1];
      char conv = desc->format[2];
      switch (conv)
       {
        case 'c':
          if (len == ' ') return format_String ("%c", *(char *)ptr);
          break;
        case 'u':
          switch (len)
           {
            case ' ':
              return format_String ("%u", *(int *)ptr);
            case 'h':
			  if (desc->format[0] == ' ')
                return format_String ("%hu", *(unsigned short *)ptr);
			  if (desc->format[0] == 'h')
                return format_String ("%hhu", *(unsigned char *)ptr);
			  break;
            case 'l':
              return format_String ("%lu", *(long *)ptr);
#ifdef __STDC__
            case 'L':
              return format_String ("%llu", *(unsigned long long *)ptr);
#endif
/* CTesK Source Start */
#ifdef _MSC_VER
            case 'M':
              return format_String ("%I64u", *(__int64 *)ptr);
#endif
/* CTesK Source End */
            }
          break;
        case 'd':
          switch (len)
            {
             case ' ':
              return format_String ("%d", *(int *)ptr);
             case 'h':
			  if (desc->format[0] == ' ')
                return format_String ("%hd", *(short *)ptr);
			  if (desc->format[0] == 'h')
                return format_String ("%hhd", *(signed char *)ptr);
			  break;
             case 'l':
              return format_String ("%ld", *(long *)ptr);
#ifdef __STDC__
             case 'L':
              return format_String ("%lld", *(long long *)ptr);
#endif
/* CTesK Source Start */
#ifdef _MSC_VER
            case 'M':
              return format_String ("%I64d", *(__int64 *)ptr);
#endif
/* CTesK Source End */
            }
          break;
        case 'f':
          switch (len)
           {
            case ' ':
              return format_String ("%f", *(double *)ptr);
            case 'h':
              return format_String ("%f", *(float *)ptr);
            case 'L':
              return format_String ("%Lf", *(long double *)ptr);
           }
          break;
        case 'p':
          if (len == ' ') return format_String ("%p", *(void **)ptr);
          break;
        default:
          fault (0, "invalid conversion '%c'", conv);
       }
      fault (0, "invalid length modifier '%c' for conversion '%c'", len, conv);
     }
    return NULL;
   }
  else
    return valueOfBytes_String( (const char*)ptr, desc->sizeOf );
}

static String* to_string_struct_sectype(StructTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i;

  res = create_StringBuffer();
  append_StringBuffer( r(res), "struct " );
  append_StringBuffer( r(res), desc->name );
  append_StringBuffer( r(res), " { " );

  for( i = 0; i < desc->numFields; i++ )
   {
    append_StringBuffer( r(res), desc->fields[i].name );
    append_StringBuffer( r(res), "=" );
    appendString_StringBuffer( r(res), to_string_sectype(desc->fields[i].type_desc,FIELD(ptr,desc->fields[i].offset)) );
    if ( i < desc->numFields-1 )
     {
      append_StringBuffer( r(res), ", " );
     }
   }
  append_StringBuffer( r(res), " }" );

  return toString(res);
}

static String* to_string_pointer_sectype(PointerTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i,bound;

  // result evaluation
  res = create_StringBuffer();
  for( i = desc->level; i > 0; i-- )
   {
    if (POINTER(ptr) == NULL)
     {bound = i;
      append_StringBuffer( r(res), "<NULL>" );
      break;
     }
    if (!ts_convex_start_to_string_operation( POINTER(ptr) ))
     {bound = i;
      appendFormat_StringBuffer( r(res), "<%p>", POINTER(ptr) );
      break;
     }
    appendFormat_StringBuffer( r(res), "<%p>ptr to ", POINTER(ptr) );
    if (i == 1)
     {bound = 0; /* (*) */
      appendString_StringBuffer( r(res), to_string_sectype(desc->base,POINTER(ptr)) );
      break;
     }
    ptr = POINTER(ptr);
   }
  // context restore
  for(i=desc->level;i>bound;i--)
  {
    if (ptr == NULL) break;	/* May happen if bound==0 after (*) and POINTER(ptr)==NULL */
	ts_convex_finish_to_string_operation( POINTER(ptr) );
    ptr = POINTER(ptr);
  }
  return toString(res);
}

static String* to_string_array_sectype(ArrayTypeDesc* desc,void* ptr)
{
StringBuffer* res;
int i;

  res = create_StringBuffer();
  append_StringBuffer( r(res), "[ " );

  for(i=0;;)
   {
    appendString_StringBuffer( r(res), to_string_sectype(desc->base,ELEMENT(ptr,i,desc->sizeOfElement)) );
    i++;
    if (i == desc->numElements)
      break;
    append_StringBuffer( r(res), ", " );
   }

  append_StringBuffer( r(res), " ]" );

  return toString(res);
}

static String* to_string_spec_sectype(SpecTypeDesc* desc,void* ptr)
{
String* res;

  if (POINTER(ptr) == NULL)
    return create_String("<NULL>");

  r_take(POINTER(ptr));
  res = toString(r(POINTER(ptr)));
  r_release(POINTER(ptr));
  return res;
}

static String* to_string_sectype(SECTypeDesc* desc,void* ptr)
{
  if (desc->customToString) {
	return (*desc->customToString)(ptr);
  }

  switch (desc->kind)
   {case simpleType:
      return to_string_simple_sectype((SimpleTypeDesc*)desc,ptr);
    case structType:
      return to_string_struct_sectype((StructTypeDesc*)desc,ptr);
    case pointerType:
      return to_string_pointer_sectype((PointerTypeDesc*)desc,ptr);
    case arrayType:
      return to_string_array_sectype((ArrayTypeDesc*)desc,ptr);
    case specType:
      return to_string_spec_sectype((SpecTypeDesc*)desc,ptr);
    case subType:
      return to_string_sectype(ts_base_type_desc(desc),ptr);
    default:
      assertion(false,"to_string_sectype#default");
      return NULL;
   }
}

String* ts_to_string_sectype(SECTypeDesc* desc, void* ptr)
{
  if (ts_convex_start_to_string_operation( ptr )) {
    String *res = to_string_sectype(desc,ptr);
	ts_convex_finish_to_string_operation( ptr );
	return res;
  } else {
	return create_String( "<CYCLIC_PTR>" );
  }
}


/********************************************************************/
/**                   Check Invariant Function                     **/
/********************************************************************/
HashSet context_CheckInvariantOperation;

static bool ts_convex_start_check_invariant_operation(Object* ref)
{
  return atl_add_hash_set( &context_CheckInvariantOperation, ref );
}

static void ts_convex_finish_check_invariant_operation(Object* ref)
{
  atl_remove_hash_set( &context_CheckInvariantOperation, ref );
}


static bool check_invariant_sectype(SECTypeDesc* desc,void* ptr);

static bool check_invariant_simple_sectype(SimpleTypeDesc* desc,void* ptr)
{
  return true;
}

static bool check_invariant_struct_sectype(StructTypeDesc* desc,void* ptr)
{
int i;

  for(i=0;i<desc->numFields;i++)
    if (!check_invariant_sectype(desc->fields[i].type_desc,FIELD(ptr,desc->fields[i].offset)))
      return false;
  return true;
}

static bool check_invariant_pointer_sectype(PointerTypeDesc* desc,void* ptr)
{
bool res;
int i,bound;

  // result evaluation
  for(i=desc->level;i>0;i--)
   {if (POINTER(ptr) == NULL)
     {bound = i;
      res = true;
      break;
     }
    if (!ts_convex_start_check_invariant_operation( POINTER(ptr) ))
     {bound = i;
      res = true;
      break;
     }
    if (i == 1)
     {bound = 0;
      res = check_invariant_sectype( desc->base, POINTER(ptr) );
      break;
     }
    ptr  = POINTER(ptr);
   }
  // context restore
  for(i=desc->level;i>bound;i--)
   {ts_convex_finish_check_invariant_operation( POINTER(ptr) );
    ptr = POINTER(ptr);
   }
  return res;
}

static bool check_invariant_array_sectype(ArrayTypeDesc* desc,void* ptr)
{
int i;

  for(i=0;i<desc->numElements;i++)
    if (!check_invariant_sectype(desc->base,ELEMENT(ptr,i,desc->sizeOfElement)))
      return false;
  return true;
}

static bool check_invariant_spec_sectype(SpecTypeDesc* desc,void* ptr)
{
  if (POINTER(ptr) != NULL)
    if (!isBasicType(desc->type,TYPE(POINTER(ptr))))
      return false;
  return checkInvariant(r(POINTER(ptr)));
}


static bool check_invariant_sub_sectype(SubTypeDesc* desc,void* ptr)
{
  if (is_specification_reference(desc->base))
   {
    r(POINTER(ptr));
   }
  return desc->invariant_checker(ptr);
}

static bool check_invariant_sectype(SECTypeDesc* desc,void* ptr)
{
  switch (desc->kind)
   {case simpleType:
      return check_invariant_simple_sectype((SimpleTypeDesc*)desc,ptr);
    case structType:
      return check_invariant_struct_sectype((StructTypeDesc*)desc,ptr);
    case pointerType:
      return check_invariant_pointer_sectype((PointerTypeDesc*)desc,ptr);
    case arrayType:
      return check_invariant_array_sectype((ArrayTypeDesc*)desc,ptr);
    case specType:
      return check_invariant_spec_sectype((SpecTypeDesc*)desc,ptr);
    case subType:
      return check_invariant_sub_sectype((SubTypeDesc*)desc,ptr);
    default:
      assertion(false,"check_invariant_sectype#default");
      return false;
   }
}


bool ts_check_invariant_sectype(SECTypeDesc* desc,void* ptr)
{
  assertion( desc!=NULL,"ts_check_invariant_sectype: desc is NULL");
  assertion( ptr!=NULL, "ts_check_invariant_sectype: ptr is NULL" );

  return check_invariant_sectype(desc,ptr);
}


/********************************************************************/
/**                      Enumerate Function                        **/
/********************************************************************/
HashSet context_EnumerateOperation;

static bool ts_convex_start_enumerate_operation(Object* ref)
{
  return atl_add_hash_set( &context_EnumerateOperation, ref );
}

static void ts_convex_finish_enumerate_operation(Object* ref)
{
  atl_remove_hash_set( &context_EnumerateOperation, ref );
}


static bool enumerate_sectype(SECTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par);

static
bool enumerate_simple_sectype(SimpleTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
  return true;
}

static
bool enumerate_struct_sectype(StructTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
int i;

  for(i=0;i<desc->numFields;i++)
   {
    if ( !enumerate_sectype( desc->fields[i].type_desc, FIELD(ptr,desc->fields[i].offset), callback, par ) )
      return false;
   }
  return true;
}

static
bool enumerate_pointer_sectype(PointerTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
bool verdict = true;
int i,bound;

  // result evaluation
  for(i=desc->level;i>0;i--)
   {if (POINTER(ptr) == NULL)
     {bound = i;
      break;
     }
    if (!ts_convex_start_enumerate_operation( POINTER(ptr) ))
     {bound = i;
      break;
     }
    if (i == 1)
     {bound = 0;
      verdict = enumerate_sectype( desc->base, POINTER(ptr), callback, par );
      break;
     }
    ptr  = POINTER(ptr);
   }
  // context restore
  for(i=desc->level;i>bound;i--)
   {ts_convex_finish_enumerate_operation( POINTER(ptr) );
    ptr = POINTER(ptr);
   }
  return verdict;
}

static
bool enumerate_array_sectype(ArrayTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
int i;

  for(i=0;i<desc->numElements;i++)
   {
    if ( !enumerate_sectype( desc->base, ELEMENT(ptr,i,desc->sizeOfElement), callback, par ) )
      return false;
   }
  return true;
}

static
bool enumerate_spec_sectype(SpecTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
  if (POINTER(ptr) != NULL)
    return callback( POINTER(ptr), par );
  return true;
}

static
bool enumerate_sectype(SECTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
  switch (desc->kind)
   {case simpleType:
      return enumerate_simple_sectype((SimpleTypeDesc*)desc,ptr,callback,par);
    case structType:
      return enumerate_struct_sectype((StructTypeDesc*)desc,ptr,callback,par);
    case pointerType:
      return enumerate_pointer_sectype((PointerTypeDesc*)desc,ptr,callback,par);
    case arrayType:
      return enumerate_array_sectype((ArrayTypeDesc*)desc,ptr,callback,par);
    case specType:
      return enumerate_spec_sectype((SpecTypeDesc*)desc,ptr,callback,par);
    case subType:
      return enumerate_sectype(ts_base_type_desc(desc),ptr,callback,par);
    default:
      assertion(false,"enumerate_sectype#default");
      return true;
   }
}

void ts_enumerate_sectype(SECTypeDesc* desc,void* ptr,EnumerationCallbackFuncType callback,void* par)
{
  assertion( desc!=NULL,     "ts_enumerate_sectype: desc is NULL");
  assertion( ptr!=NULL,      "ts_enumerate_sectype: ptr is NULL" );
  assertion( callback!=NULL, "ts_enumerate_sectype: callback is NULL" );

  enumerate_sectype(desc,ptr,callback,par);
}


/********************************************************************/
/**                       Destroy Function                         **/
/********************************************************************/
HashSet context_DestroyOperation;
long nestedCnt_DestroyOperation = 0L;


static void destroy_sectype(SECTypeDesc* desc,void* ptr,bool weak);

static void destroy_simple_sectype(SimpleTypeDesc* desc,void* ptr,bool weak)
{
}

static void destroy_struct_sectype(StructTypeDesc* desc,void* ptr,bool weak)
{
int i;

  for(i=0;i<desc->numFields;i++)
    destroy_sectype(desc->fields[i].type_desc,FIELD(ptr,desc->fields[i].offset),weak);
}

static void destroy_pointer_sectype(PointerTypeDesc* desc,void* ptr,bool weak)
{
void* tmp;
int i,bound;

  tmp = ptr;
  for(bound=desc->level;bound>0;bound--)
   {if (POINTER(tmp) == NULL)
      break;
    if (!atl_add_hash_set( &context_DestroyOperation, POINTER(tmp) ))
      break;
    tmp  = POINTER(tmp);
   }
  if (bound == 0)
    destroy_sectype(desc->base,tmp,weak);

  for(;bound<desc->level;bound++)
   {tmp = ptr;
    for(i=desc->level-1;i>bound;i--)
      tmp = POINTER(tmp);

#ifdef _DEBUG_CONVEX
    printf("Debug: free(%p)\n",POINTER(tmp));
#endif

    free(POINTER(tmp));
   }
}

static void destroy_array_sectype(ArrayTypeDesc* desc,void* ptr,bool weak)
{
int i;

  for(i=0;i<desc->numElements;i++)
    destroy_sectype(desc->base,ELEMENT(ptr,i,desc->sizeOfElement),weak);
}

static void destroy_spec_sectype(SpecTypeDesc* desc,void* ptr,bool weak)
{
  if (weak)
    detach_Object(POINTER(ptr));
  else
    destroy(POINTER(ptr));
}

static void destroy_sectype(SECTypeDesc* desc,void* ptr,bool weak)
{
  switch (desc->kind)
   {case simpleType:
      destroy_simple_sectype((SimpleTypeDesc*)desc,ptr,weak);
      return;
    case structType:
      destroy_struct_sectype((StructTypeDesc*)desc,ptr,weak);
      return;
    case pointerType:
      destroy_pointer_sectype((PointerTypeDesc*)desc,ptr,weak);
      return;
    case arrayType:
      destroy_array_sectype((ArrayTypeDesc*)desc,ptr,weak);
      return;
    case specType:
      destroy_spec_sectype((SpecTypeDesc*)desc,ptr,weak);
      return;
    case subType:
      destroy_sectype(ts_base_type_desc(desc),ptr,weak);
      return;
    default:
      assertion(false,"destroy_sectype#default");
   }
}

void ts_start_destroy_sectype()
{
  nestedCnt_DestroyOperation++;
}

void ts_finish_destroy_sectype()
{
  nestedCnt_DestroyOperation--;
  if (nestedCnt_DestroyOperation == 0)
    atl_reinit_hash_set( &context_DestroyOperation );
}

void ts_destroy_sectype(SECTypeDesc* desc,void* ptr,bool weak)
{
  assertion( desc!=NULL,"ts_destroy_sectype: desc is NULL");
  assertion( ptr!=NULL, "ts_destroy_sectype: ptr is NULL" );

  ts_start_destroy_sectype();
  destroy_sectype(desc,ptr,weak);
  ts_finish_destroy_sectype();
}
