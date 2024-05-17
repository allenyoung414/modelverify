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
 * File: TraceFormatter.h
 * Description:
 *       This is a header file of TraceFormatter.
 */

#ifndef __TRACE_FORMATTER_H__
#define __TRACE_FORMATTER_H__


/* CTesK Source Start */
#ifdef _MSC_VER
/* Disable warning C4786: identifier was truncated to 'x' characters in the debug information */
#pragma warning( disable : 4786 )
#ifdef __cplusplus
}
#endif

#endif
/* CTesK Source End */

#include <time.h>
#include <atl/string.h>
#include "TraceBool.h"
#include "TraceList.h"

#define TRACE_VERSION "2.3"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TraceFormatter TraceFormatter;
struct _TraceFormatter
{
  void * state;
  void (*delete_state)(void *state);
  /*
   * Trace level methods
   */
  String * (*startTrace)(void *state, const char *encoding);
  String * (*endTrace)(void *state, TraceBool original);

  /*
   * Scenario level methods
   */
  String * (*traceScenarioStart)(void *state, int traceId, const char *name, time_t now, const char *host, const char *os);
  String * (*traceEnvironmentProperties)(void *state, int traceId, TraceList *properties);
  String * (*traceScenarioParameters)(void *state, int traceId, int argc, const char** argv);
  String * (*traceTestEngineProperty)(void *state, int traceId, const char *name, const char *value);
  String * (*traceScenarioEnd)(void *state, int traceId, time_t now);

  /*
   * Transition level methods
   */
  String * (*traceScenarioValue)(void *state, int traceId, const char *kind, const char *type, const char *name, const char *value);
  String * (*traceState)(void *state, int traceId, const char *id);
  String * (*traceTransitionStart)(void *state, int traceId, const char *id);
  String * (*traceTransitionEnd)(void *state, int traceId);

  /*
   * Oracle level methods
   */
  String * (*traceModelOperationStart)(void *state, int traceId, const char *kind, const char *subsystem, const char *signature, unsigned refid);
  String * (*traceModelOperationArgument)(void *state, int traceId, const char *type, const char *name, const char *value);
  String * (*traceModelOperationResult)(void *state, int traceId, const char *type, const char *value);
  String * (*traceModelOperationChannel)(void *state, int traceId, const char *channel);
  String * (*traceModelOperationTimestamp)(void *state, int traceId, const char *timestamp);
  String * (*traceOracleStart)(void *state, int traceId, const char *subsystem, const char *signature, long ref);
  String * (*tracePreconditionEnd)(void *state, int traceId);
  String * (*traceOracleEnd)(void *state, int traceId);
  String * (*traceModelOperationEnd)(void *state, int traceId);
  String * (*traceSeriesStart)(void *state, int traceId);
  String * (*traceSeriesEnd)(void *state, int traceId);

  /*
   * Coverage level methods
   */
  String * (*tracePrimeFormula)(void *state, int traceId, int id, TraceBool value);
  String * (*traceCoverageElement)(void *state, int traceId, const char *structureId, const char *coverageId, int branchId);
  String * (*traceMark)(void *state, int traceId, const char *mark);

  String * (*traceCoverageStructureStart)(void *state, int traceId, const char* name);
  String * (*traceFormulaeStart)(void *state);
  String * (*traceFormula)(void *state, int id, const char *text);
  String * (*traceFormulaeEnd)(void *state);
  String * (*traceCoverageStart)(void *state, const char *id);
  String * (*traceElement)(void *state, int id, const char *name);
  String * (*traceCoverageEnd)(void *state);
  String * (*traceCoverageStructureEnd)(void *state);

  /*
   * Other messages
   */
  String * (*traceException)(void *state, int traceId, const char *kind, const TraceList *values, const TraceList *infos);
  String * (*traceInterimException)(void *state, int traceId, const char *kind, const TraceList *values, const TraceList *infos);
  String * (*traceInternalError)(void *state, int traceId, const char *info);
  String * (*traceSystemInfo)(void *state, int traceId, const char *info);
  String * (*traceUserInfo)(void *state, int traceId, const char *info);

  TraceBool (*valid)(TraceFormatter *tf);
};


void TraceFormatter_delete(TraceFormatter *tf);


#endif /* __TRACE_FORMATTER_H__ */
