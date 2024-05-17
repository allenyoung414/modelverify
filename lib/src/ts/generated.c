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
 * FILE:        generated.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    18 April 2005
 * DESCRIPTION: Implementation of functions needed
 *              by generated code.
 */

#include <ts/generated.h>
#include <ts/engine.h>
#include <ts/timemark.h>
#include <atl/object_int.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>


/********************************************************************/
/**                 Test System Auxiliary Functions                **/
/********************************************************************/
void ts_trace_model_operation_timestamp(TimeInterval timestamp)
{
char buffer[256];

  switch (getTSTimeModel())
   {case NotUseTSTime:
      break;
    case LinearTSTime:
      printLinearTimeInterval(buffer,timestamp);
      traceModelOperationTimestamp(buffer);
      break;
    case DistributedTSTime:
      printDistributedTimeInterval(buffer,timestamp);
      traceModelOperationTimestamp(buffer);
      break;
    default:
      assertion( false, "ts_trace_model_operation_timestamp#default" );
      break;
   }
}

void ts_trace_bad_oracle_verdict(void)
{
  if (isInSeries()) {
    traceInterimException(POSTCONDITION_FAILED_MESSAGE);
  } else {
    traceException(POSTCONDITION_FAILED_MESSAGE);
  }
}

void ts_trace_bad_mediator_verdict(void)
{
  if (isInSeries()) {
    traceInterimException(MEDIATOR_FAILED_MESSAGE);
  } else {
    traceException(MEDIATOR_FAILED_MESSAGE);
  }
}


/********************************************************************/
/**                 Specification Reference Casting                **/
/********************************************************************/
Object** ts_cast_spec_ref_lvalue(const Type* type,Object** ref)
{
  assertion( ref != NULL, "ts_cast_spec_ref_lvalue: ref is NULL" );

  if (*ref == NULL)
    return ref;
  if (isBasicType( type, TYPE(*ref) ))
    return ref;
  if (!isBasicType( TYPE(*ref), type ))
   {assertion( false, "Incorrect casting of specification reference" );
    return ref;
   }
  if (!type->check_invariant( r(*ref) ))
   {assertion( false, "Incorrect casting: invariant of subtype is violated" );
    return ref;
   }

  // clone object and forbid assignment or not?
  header( *ref )->type = type;
  return ref;
}

Object* ts_cast_spec_ref(const Type* type,Object* ref)
{
  if (ref == NULL)
    return ref;
  if (isBasicType( type, TYPE(ref) ))
    return ref;
  if (!isBasicType( TYPE(ref), type ))
   {assertion( false, "Incorrect casting of specification reference" );
    return ref;
   }
  if (!type->check_invariant( r(ref) ))
   {assertion( false, "Incorrect casting: invariant of subtype is violated" );
    return ref;
   }

  if (header( ref )->hard_ref_cnt + header( ref )->weak_ref_cnt == 1)
    header( ref )->type = type;
  else
   {// clone object before changing type
    ref = clone_Object( ref );
    header( ref )->type = type;
   }
  return ref;
}

Object* ts_check_spec_ref(const Type* type,Object* ref,const char* kind,const char* where)
{
  if (ref == NULL)
    return ref;
  if (isBasicType( type, TYPE(ref) ))
    return ref;
  if (!isBasicType( TYPE(ref), type ))
   {
    if (where != NULL)
      assertion( false, "Incorrect %s at %s: type of reference is %s instead of %s", kind, where, TYPE(ref)->name, type->name );
    else
      assertion( false, "Incorrect %s: type of reference is %s instead of %s", kind, TYPE(ref)->name, type->name );
    return ref;
   }
//  if (!type->check_invariant( r(ref) ))
//    assertion( false, "Incorrect %s: invariant of subtype is violated", kind );
  return ref;
}


/********************************************************************/
/**     Specification Reference Management Auxiliary Functions     **/
/********************************************************************/
Object* ts_assign_spec_ref(Object** left,Object* right,const char* where)
{
  assertion( (left != NULL), "ts_assign: try to assign by NULL pointer at %s", where );

  destroy(*left);
  return (*left = right);
}

void* ts_destroy_spec_ref(Object* ref)
{
  destroy(ref);
  return ref;
}


/********************************************************************/
/**                       Auxiliary Functions                      **/
/********************************************************************/
void _dummy(void* p)
{
}

Object* singletonState(void)
{
  return NULL;
}


/********************************************************************/
/**                        Oracle Verdict                         **/
/********************************************************************/
static bool _ts_oracle_verdict = true;

void setOracleVerdict(bool verdict)
{
  _ts_oracle_verdict = _ts_oracle_verdict && verdict;
}

bool getOracleVerdict(void)
{
  return _ts_oracle_verdict;
}

void clearOracleVerdict(void)
{
  _ts_oracle_verdict = true;
}

/********************************************************************/
/**                        Precondition checking                   **/
/********************************************************************/

static bool _ts_check_precondition = true;

void setCheckPrecondition( bool check )
{
  _ts_check_precondition = check;
}

bool getCheckPrecondition(void)
{
  return _ts_check_precondition;
}

/********************************************************************/
/**                        Postcondition checking                   **/
/********************************************************************/

static bool _ts_check_postcondition = true;

void setCheckPostcondition( bool check )
{
  _ts_check_postcondition = check;
}

bool getCheckPostcondition(void)
{
  return _ts_check_postcondition;
}

/********************************************************************/
/**                        Trace Messages                          **/
/********************************************************************/
const char PRECONDITION_FAILED_MESSAGE[]            = "PRECONDITION_FAILED";
const char POSTCONDITION_FAILED_MESSAGE[]           = "POSTCONDITION_FAILED";
const char SCENARIO_FUNCTION_FAILED_MESSAGE[]       = "SCENARIO_FUNCTION_FAILED";
const char INCORRECT_SET_OF_INTERACTIONS_MESSAGE[]  = "INCORRECT_SET_OF_INTERACTIONS";
const char MEDIATOR_FAILED_MESSAGE[]                = "MEDIATOR_FAILED";
const char UNCONNECTED_GRAPH_MESSAGE[]              = "UNCONNECTED_GRAPH";
const char NONDETERMINISTIC_GRAPH_MESSAGE[]         = "NONDETERMINISTIC_GRAPH";
const char SCENARIO_INITIALIZATION_FAILED_MESSAGE[] = "SCENARIO_INITIALIZATION_FAILED";
const char SERIALIZATION_FAILED_MESSAGE[]           = "SERIALIZATION_FAILED";
const char NONSTATIONARY_STATE_MESSAGE[]            = "NONSTATIONARY_STATE";
