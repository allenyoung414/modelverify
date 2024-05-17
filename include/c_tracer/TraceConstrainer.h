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
 * File: TraceConstrainer.h
 * Description:
 *       This is a header file of TraceConstrainer.
 */

#ifndef __TRACE_CONSTRAINER_H__
#define __TRACE_CONSTRAINER_H__

//#include <string.h>
#include "TraceBool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TraceConstrainer TraceConstrainer;
struct _TraceConstrainer {
  TraceBool is_in_trace;
  TraceBool is_in_scenario;
  TraceBool is_in_transition;
  TraceBool is_in_series;
  TraceBool is_in_model_operation;
  TraceBool is_in_oracle;
  TraceBool is_in_precondition;
  TraceBool is_in_coverage_structure;
  TraceBool is_in_formulae;
  TraceBool is_in_coverage;

  TraceBool (*startTrace)(TraceConstrainer *tc);
  TraceBool (*endTrace)(TraceConstrainer *tc);
  TraceBool (*traceScenarioStart)(TraceConstrainer *tc, const char *name);
  TraceBool (*traceScenarioParameters)(TraceConstrainer *tc, int argc, const char** argv);
  TraceBool (*traceTestEngineProperty)(TraceConstrainer *tc, const char *name,const char *value);
  TraceBool (*traceScenarioEnd)(TraceConstrainer *tc);
  TraceBool (*traceScenarioValue)(TraceConstrainer *tc, const char *kind, const char *type, const char *name, const char *value);
  TraceBool (*traceState)(TraceConstrainer *tc, const char *id);
  TraceBool (*traceTransitionStart)(TraceConstrainer *tc, const char *id);
  TraceBool (*traceTransitionEnd)(TraceConstrainer *tc);
  TraceBool (*traceModelOperationStart)(TraceConstrainer *tc, const char *kind, const char *subsystem, const char *signature);
  TraceBool (*traceModelOperationArgument)(TraceConstrainer *tc, const char *type, const char *name, const char *value);
  TraceBool (*traceModelOperationResult)(TraceConstrainer *tc, const char *type, const char *value);
  TraceBool (*traceModelOperationIdentifier)(TraceConstrainer *tc, long id);
  TraceBool (*traceModelOperationChannel)(TraceConstrainer *tc, long chid);
  TraceBool (*traceModelOperationTimestamp)(TraceConstrainer *tc, const char *timestamp);
  TraceBool (*traceOracleStart)(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId);
  TraceBool (*tracePreconditionEnd)(TraceConstrainer *tc);
  TraceBool (*traceOracleEnd)(TraceConstrainer *tc);
  TraceBool (*traceModelOperationEnd)(TraceConstrainer *tc);
  TraceBool (*traceSeriesStart)(TraceConstrainer *tc);
  TraceBool (*traceSeriesEnd)(TraceConstrainer *tc);
  TraceBool (*keepOracleStart)(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId);
  TraceBool (*keepOracleEnd)(TraceConstrainer *tc);
  TraceBool (*tracePrimeFormula)(TraceConstrainer *tc, int id, TraceBool value);
  TraceBool (*traceCoverageElement)(TraceConstrainer *tc, const char *structureId, const char *coverageId, int branchId);
  TraceBool (*traceMark)(TraceConstrainer *tc, const char *mark);
  TraceBool (*traceCoverageStructureStart)(TraceConstrainer *tc, const char* name);
  TraceBool (*traceFormulaeStart)(TraceConstrainer *tc);
  TraceBool (*traceFormula)(TraceConstrainer *tc, int id, const char *text);
  TraceBool (*traceFormulaeEnd)(TraceConstrainer *tc);
  TraceBool (*traceCoverageStart)(TraceConstrainer *tc, const char *id);
  TraceBool (*traceElement)(TraceConstrainer *tc, int id, const char *name);
  TraceBool (*traceCoverageEnd)(TraceConstrainer *tc);
  TraceBool (*traceCoverageStructureEnd)(TraceConstrainer *tc);
  TraceBool (*traceException)(TraceConstrainer *tc, const char *info);
  TraceBool (*traceInterimException)(TraceConstrainer *tc, const char *info);
  TraceBool (*traceExceptionValue)(TraceConstrainer *tc, const char *name, const char *value);
  TraceBool (*traceExceptionInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*traceInternalError)(TraceConstrainer *tc, const char *info);
  TraceBool (*traceSystemInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*traceUserInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*isInTrace)(TraceConstrainer *tc);
  TraceBool (*isInScenario)(TraceConstrainer *tc);
  TraceBool (*isInTransition)(TraceConstrainer *tc);
  TraceBool (*isInSeries)(TraceConstrainer *tc);
  TraceBool (*isInModelOperation)(TraceConstrainer *tc);
  TraceBool (*isInOracle)(TraceConstrainer *tc);
  TraceBool (*isInPrecondition)(TraceConstrainer *tc);
  TraceBool (*isInCoverageStructure)(TraceConstrainer *tc);
  TraceBool (*isInFormulae)(TraceConstrainer *tc);
  TraceBool (*isInCoverage)(TraceConstrainer *tc);

  /*
   * User-defined hooks
   */
  TraceBool (*onStartTrace)(TraceConstrainer *tc);
  TraceBool (*onEndTrace)(TraceConstrainer *tc);
  TraceBool (*onTraceScenarioStart)(TraceConstrainer *tc, const char *name);
  TraceBool (*onTraceScenarioParameters)(TraceConstrainer *tc, int argc, const char** argv);
  TraceBool (*onTraceTestEngineProperty)(TraceConstrainer *tc, const char *name,const char *value);
  TraceBool (*onTraceScenarioEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceScenarioValue)(TraceConstrainer *tc, const char *kind, const char *type, const char *name, const char *value);
  TraceBool (*onTraceState)(TraceConstrainer *tc, const char *id);
  TraceBool (*onTraceTransitionStart)(TraceConstrainer *tc, const char *id);
  TraceBool (*onTraceTransitionEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceModelOperationStart)(TraceConstrainer *tc, const char *kind, const char *subsystem, const char *signature);
  TraceBool (*onTraceModelOperationArgument)(TraceConstrainer *tc, const char *type, const char *name, const char *value);
  TraceBool (*onTraceModelOperationResult)(TraceConstrainer *tc, const char *type, const char *value);
  TraceBool (*onTraceModelOperationIdentifier)(TraceConstrainer *tc, long id);
  TraceBool (*onTraceModelOperationChannel)(TraceConstrainer *tc, long chid);
  TraceBool (*onTraceModelOperationTimestamp)(TraceConstrainer *tc, const char *timestamp);
  TraceBool (*onTraceOracleStart)(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId);
  TraceBool (*onTracePreconditionEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceOracleEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceModelOperationEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceSeriesStart)(TraceConstrainer *tc);
  TraceBool (*onTraceSeriesEnd)(TraceConstrainer *tc);
  TraceBool (*onKeepOracleStart)(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId);
  TraceBool (*onKeepOracleEnd)(TraceConstrainer *tc);
  TraceBool (*onTracePrimeFormula)(TraceConstrainer *tc, int id, TraceBool value);
  TraceBool (*onTraceCoverageElement)(TraceConstrainer *tc, const char *structureId, const char *coverageId, int branchId);
  TraceBool (*onTraceMark)(TraceConstrainer *tc, const char *mark);
  TraceBool (*onTraceCoverageStructureStart)(TraceConstrainer *tc, const char* name);
  TraceBool (*onTraceFormulaeStart)(TraceConstrainer *tc);
  TraceBool (*onTraceFormula)(TraceConstrainer *tc, int id, const char *text);
  TraceBool (*onTraceFormulaeEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceCoverageStart)(TraceConstrainer *tc, const char *id);
  TraceBool (*onTraceElement)(TraceConstrainer *tc, int id, const char *name);
  TraceBool (*onTraceCoverageEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceCoverageStructureEnd)(TraceConstrainer *tc);
  TraceBool (*onTraceException)(TraceConstrainer *tc, const char *info);
  TraceBool (*onTraceInterimException)(TraceConstrainer *tc, const char *info);
  TraceBool (*onTraceExceptionValue)(TraceConstrainer *tc, const char *name, const char *value);
  TraceBool (*onTraceExceptionInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*onTraceInternalError)(TraceConstrainer *tc, const char *info);
  TraceBool (*onTraceSystemInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*onTraceUserInfo)(TraceConstrainer *tc, const char *info);
  TraceBool (*valid)(TraceConstrainer *tc);
};

  /*
   * Constructor
   */
TraceConstrainer * TraceConstrainer_TraceConstrainer();
void TraceConstrainer_delete(TraceConstrainer *tc);


#ifdef __cplusplus
}
#endif

#endif /* __TRACE_CONSTRAINER_H__ */
