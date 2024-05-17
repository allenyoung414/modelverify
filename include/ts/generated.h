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
 * FILE:        generated.h
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    18 April 2005
 * DESCRIPTION: Contains declarations of functions
 *              and data types needed by
 *              generated code.
 * SCOPE:       It is an interface header.
 */

#ifndef TS_GENERATED_H
#define TS_GENERATED_H

#ifdef __SEC__
#pragma SEC force_use_start
#endif

#include "ts/convex.h"
#include "ts/timemark.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************/
/**                        Call Properties                         **/
/********************************************************************/
typedef struct CallProperties CallProperties;

struct CallProperties
{
  bool         verdict;
  TimeInterval time;
};


/********************************************************************/
/**                          InteractionId                         **/
/********************************************************************/
typedef long InteractionId;


/********************************************************************/
/**                     Oracle Call Properties                     **/
/********************************************************************/
typedef struct OracleCallProperties OracleCallProperties;

struct OracleCallProperties
{
  InteractionId refId;
  bool          verdict;
};


/********************************************************************/
/**                        Oracle Verdict                         **/
/********************************************************************/
void setOracleVerdict(bool verdict);
bool getOracleVerdict(void);
void clearOracleVerdict(void);


/********************************************************************/
/**                    Call Pure Oracle Func Type                  **/
/********************************************************************/
typedef enum OracleVerdict
{
  OracleVerdict_Ok,
  OracleVerdict_PreconditionFailed,
  OracleVerdict_PostconditionFailed
}
OracleVerdict;

typedef struct CallContextStruct
{
  Object*       params;
  TimeInterval  time;
  InteractionId refId;
}
CallContextType;

typedef bool (*PreFuncType)(Object* params);

typedef OracleVerdict (*CallPureOracleFuncType)(CallContextType);


/********************************************************************/
/**                 Trace Model Operation Func Type                **/
/********************************************************************/
typedef void (*TraceModelOperationFuncType)(InteractionId,Object*,ChannelID,TimeInterval);


/********************************************************************/
/**                       Specification Desc                       **/
/********************************************************************/
typedef struct SpecificationDesc
{
  const char*  name;
  const char*  subsystem;
  const char*  signature;
  PreFuncType  precondition;
  CallPureOracleFuncType call_pure_oracle;
  TraceModelOperationFuncType trace_model_operation;
  bool         is_reaction;
  const Type*  par_res_type;
}
SpecificationDesc;


/********************************************************************/
/**                        SpecificationID                         **/
/********************************************************************/
typedef SpecificationDesc* SpecificationID;


/********************************************************************/
/**             Test Scenario Action Descriptor Types              **/
/********************************************************************/
/*
  init - returns an initialized context of the action
*/
typedef Object* (*PtrActionInit)(void);


/*
  has_next - returns true iff the next test model parameters of
             the action exist
*/
typedef bool (*PtrActionHasNext)(Object*);


/*
  next - makes next test model parameters
*/
typedef bool (*PtrActionNext)(Object*);

/*
  call - converts test model parameters to calls of
         specification functions
*/
typedef bool (*PtrActionCall)(Object*);


/********************************************************************/
/**                  Test Scenario Action Descriptor               **/
/********************************************************************/
typedef struct
{
  PtrActionInit    init;
  PtrActionHasNext has_next;
  PtrActionNext    next;
  PtrActionCall    call;
  const char*      name;
} ScenarioFunctionDesc;


/********************************************************************/
/**                     Test Scenario Action ID                    **/
/********************************************************************/
typedef ScenarioFunctionDesc* ScenarioFunctionID;


/********************************************************************/
/**                 Test System Auxiliary Functions                **/
/********************************************************************/
void ts_trace_model_operation_timestamp(TimeInterval timestamp);
void ts_trace_bad_oracle_verdict(void);
void ts_trace_bad_mediator_verdict(void);


/********************************************************************/
/**                 Specification Reference Casting                **/
/********************************************************************/
Object** ts_cast_spec_ref_lvalue(const Type* type,Object** ref);
Object* ts_cast_spec_ref(const Type* type,Object* ref);
Object* ts_check_spec_ref(const Type* type,Object* ref,const char* kind,const char* where);


/********************************************************************/
/**     Specification Reference Management Auxiliary Functions     **/
/********************************************************************/
Object* ts_assign_spec_ref(Object** left,Object* right,const char* where);
void*   ts_destroy_spec_ref(Object* ref);


/********************************************************************/
/**                       Auxiliary Functions                      **/
/********************************************************************/
void _dummy(void* p);
Object* singletonState(void);

/********************************************************************/
/**                        Precondition checking                   **/
/********************************************************************/

void setCheckPrecondition( bool check );
bool getCheckPrecondition(void);

/********************************************************************/
/**                        Postcondition checking                  **/
/********************************************************************/

void setCheckPostcondition( bool check );
bool getCheckPostcondition(void);

/********************************************************************/
/**                        Trace Messages                          **/
/********************************************************************/
extern const char PRECONDITION_FAILED_MESSAGE[];
extern const char POSTCONDITION_FAILED_MESSAGE[];
extern const char SCENARIO_FUNCTION_FAILED_MESSAGE[];
extern const char INCORRECT_SET_OF_INTERACTIONS_MESSAGE[];
extern const char MEDIATOR_FAILED_MESSAGE[];
extern const char UNCONNECTED_GRAPH_MESSAGE[];
extern const char NONDETERMINISTIC_GRAPH_MESSAGE[];
extern const char SCENARIO_INITIALIZATION_FAILED_MESSAGE[];
extern const char SERIALIZATION_FAILED_MESSAGE[];
extern const char NONSTATIONARY_STATE_MESSAGE[];

#ifdef __cplusplus
}
#endif

#ifdef __SEC__
#pragma SEC force_use_end
#endif

#endif
