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
 * File: TraceConstrainer.c
 * Description:
 *       This file contains an implementation of TraceConstrainer.
 */

#include <stdio.h>
#include <stdlib.h>

#include "TraceConstrainer.h"

extern int tracer_debug_level;

#define TRACE_CONSTRAINER_DEBUG_LEVEL  3

/*
 * trace
 *     scenario
 *         transition
 *             model_operation or series
 *                 oracle
 *                     coverage_structure
 *                         formulae
 *                         coverage
 */


TraceBool TraceConstrainer_valid(TraceConstrainer *tc) {
  if( (NULL == tc) ||
      (NULL == tc->startTrace) ||
      (NULL == tc->endTrace) ||
      (NULL == tc->traceScenarioStart) ||
      (NULL == tc->traceScenarioParameters) ||
      (NULL == tc->traceTestEngineProperty) ||
      (NULL == tc->traceScenarioEnd) ||
      (NULL == tc->traceScenarioValue) ||
      (NULL == tc->traceState) ||
      (NULL == tc->traceTransitionStart) ||
      (NULL == tc->traceTransitionEnd) ||
      (NULL == tc->traceModelOperationStart) ||
      (NULL == tc->traceModelOperationArgument) ||
      (NULL == tc->traceModelOperationResult) ||
      (NULL == tc->traceModelOperationIdentifier) ||
      (NULL == tc->traceModelOperationChannel) ||
      (NULL == tc->traceModelOperationTimestamp) ||
      (NULL == tc->traceOracleStart) ||
      (NULL == tc->tracePreconditionEnd) ||
      (NULL == tc->traceOracleEnd) ||
      (NULL == tc->traceModelOperationEnd) ||
      (NULL == tc->traceSeriesStart) ||
      (NULL == tc->traceSeriesEnd) ||
      (NULL == tc->keepOracleStart) ||
      (NULL == tc->keepOracleEnd) ||
      (NULL == tc->tracePrimeFormula) ||
      (NULL == tc->traceCoverageElement) ||
      (NULL == tc->traceMark) ||
      (NULL == tc->traceCoverageStructureStart) ||
      (NULL == tc->traceFormulaeStart) ||
      (NULL == tc->traceFormula) ||
      (NULL == tc->traceFormulaeEnd) ||
      (NULL == tc->traceCoverageStart) ||
      (NULL == tc->traceElement) ||
      (NULL == tc->traceCoverageEnd) ||
      (NULL == tc->traceCoverageStructureEnd) ||
      (NULL == tc->traceException) ||
      (NULL == tc->traceInterimException) ||
      (NULL == tc->traceExceptionValue) ||
      (NULL == tc->traceExceptionInfo) ||
      (NULL == tc->traceInternalError) ||
      (NULL == tc->traceSystemInfo) ||
      (NULL == tc->traceUserInfo) ||
      (NULL == tc->isInTrace) ||
      (NULL == tc->isInScenario) ||
      (NULL == tc->isInTransition) ||
      (NULL == tc->isInSeries) ||
      (NULL == tc->isInModelOperation) ||
      (NULL == tc->isInOracle) ||
      (NULL == tc->isInPrecondition) ||
      (NULL == tc->isInCoverageStructure) ||
      (NULL == tc->isInFormulae) ||
      (NULL == tc->isInCoverage) ||
      (NULL == tc->onStartTrace) ||
      (NULL == tc->onEndTrace) ||
      (NULL == tc->onTraceScenarioStart) ||
      (NULL == tc->onTraceScenarioParameters) ||
      (NULL == tc->onTraceTestEngineProperty) ||
      (NULL == tc->onTraceScenarioEnd) ||
      (NULL == tc->onTraceScenarioValue) ||
      (NULL == tc->onTraceState) ||
      (NULL == tc->onTraceTransitionStart) ||
      (NULL == tc->onTraceTransitionEnd) ||
      (NULL == tc->onTraceModelOperationStart) ||
      (NULL == tc->onTraceModelOperationArgument) ||
      (NULL == tc->onTraceModelOperationResult) ||
      (NULL == tc->onTraceModelOperationIdentifier) ||
      (NULL == tc->onTraceModelOperationChannel) ||
      (NULL == tc->onTraceModelOperationTimestamp) ||
      (NULL == tc->onTraceOracleStart) ||
      (NULL == tc->onTracePreconditionEnd) ||
      (NULL == tc->onTraceOracleEnd) ||
      (NULL == tc->onTraceModelOperationEnd) ||
      (NULL == tc->onTraceSeriesStart) ||
      (NULL == tc->onTraceSeriesEnd) ||
      (NULL == tc->onKeepOracleStart) ||
      (NULL == tc->onKeepOracleEnd) ||
      (NULL == tc->onTracePrimeFormula) ||
      (NULL == tc->onTraceCoverageElement) ||
      (NULL == tc->onTraceMark) ||
      (NULL == tc->onTraceCoverageStructureStart) ||
      (NULL == tc->onTraceFormulaeStart) ||
      (NULL == tc->onTraceFormula) ||
      (NULL == tc->onTraceFormulaeEnd) ||
      (NULL == tc->onTraceCoverageStart) ||
      (NULL == tc->onTraceElement) ||
      (NULL == tc->onTraceCoverageEnd) ||
      (NULL == tc->onTraceCoverageStructureEnd) ||
      (NULL == tc->onTraceException) ||
      (NULL == tc->onTraceInterimException) ||
      (NULL == tc->onTraceExceptionValue) ||
      (NULL == tc->onTraceExceptionInfo) ||
      (NULL == tc->onTraceInternalError) ||
      (NULL == tc->onTraceSystemInfo) ||
      (NULL == tc->onTraceUserInfo) )
    return tr_false;
  return tr_true;
}



/*
 * User-defined hooks
 */

TraceBool TraceConstrainer_onStartTrace(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onEndTrace(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceScenarioStart(TraceConstrainer *tc, const char *name) { return tr_true; }
TraceBool TraceConstrainer_onTraceScenarioParameters(TraceConstrainer *tc, int argc, const char** argv) { return tr_true; }
TraceBool TraceConstrainer_onTraceTestEngineProperty(TraceConstrainer *tc, const char *name,const char *value) { return tr_true; }
TraceBool TraceConstrainer_onTraceScenarioEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceScenarioValue(TraceConstrainer *tc, const char *kind, const char *type, const char *name, const char *value) { return tr_true; }
TraceBool TraceConstrainer_onTraceState(TraceConstrainer *tc, const char *id) { return tr_true; }
TraceBool TraceConstrainer_onTraceTransitionStart(TraceConstrainer *tc, const char *id) { return tr_true; }
TraceBool TraceConstrainer_onTraceTransitionEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationStart(TraceConstrainer *tc, const char *kind, const char *subsystem, const char *signature) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationArgument(TraceConstrainer *tc, const char *type, const char *name, const char *value) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationResult(TraceConstrainer *tc, const char *type, const char *value) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationIdentifier(TraceConstrainer *tc, long id) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationChannel(TraceConstrainer *tc, long chid) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationTimestamp(TraceConstrainer *tc, const char *timestamp) { return tr_true; }
TraceBool TraceConstrainer_onTraceOracleStart(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId) { return tr_true; }
TraceBool TraceConstrainer_onTracePreconditionEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceOracleEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceModelOperationEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceSeriesStart(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceSeriesEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onKeepOracleStart(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId) { return tr_true; }
TraceBool TraceConstrainer_onKeepOracleEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTracePrimeFormula(TraceConstrainer *tc, int id, TraceBool value) { return tr_true; }
TraceBool TraceConstrainer_onTraceCoverageElement(TraceConstrainer *tc, const char *structureId, const char *coverageId, int branchId) { return tr_true; }
TraceBool TraceConstrainer_onTraceMark(TraceConstrainer *tc, const char *mark) { return tr_true; }
TraceBool TraceConstrainer_onTraceCoverageStructureStart(TraceConstrainer *tc, const char* name) { return tr_true; }
TraceBool TraceConstrainer_onTraceFormulaeStart(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceFormula(TraceConstrainer *tc, int id, const char *text) { return tr_true; }
TraceBool TraceConstrainer_onTraceFormulaeEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceCoverageStart(TraceConstrainer *tc, const char *id) { return tr_true; }
TraceBool TraceConstrainer_onTraceElement(TraceConstrainer *tc, int id, const char *name) { return tr_true; }
TraceBool TraceConstrainer_onTraceCoverageEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceCoverageStructureEnd(TraceConstrainer *tc) { return tr_true; }
TraceBool TraceConstrainer_onTraceException(TraceConstrainer *tc, const char *kind) { return tr_true; }
TraceBool TraceConstrainer_onTraceInterimException(TraceConstrainer *tc, const char *kind) { return tr_true; }
TraceBool TraceConstrainer_onTraceExceptionValue(TraceConstrainer *tc, const char *name, const char *value) { return tr_true; }
TraceBool TraceConstrainer_onTraceExceptionInfo(TraceConstrainer *tc, const char *info) { return tr_true; }
TraceBool TraceConstrainer_onTraceInternalError(TraceConstrainer *tc, const char *info) { return tr_true; }
TraceBool TraceConstrainer_onTraceSystemInfo(TraceConstrainer *tc, const char *info) { return tr_true; }
TraceBool TraceConstrainer_onTraceUserInfo(TraceConstrainer *tc, const char *info) { return tr_true; }



/*
 * Trace level methods
 */
TraceBool TraceConstrainer_startTrace(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (tc->is_in_trace) return tr_false;
  tc->is_in_trace = tr_true;
  return TraceConstrainer_onStartTrace(tc);
}

TraceBool TraceConstrainer_endTrace(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_trace || tc->is_in_scenario) return tr_false;
  tc->is_in_trace = tr_false;
  return TraceConstrainer_onEndTrace(tc);
}


/*
 * Scenario level methods
 */
TraceBool TraceConstrainer_traceScenarioStart(TraceConstrainer *tc, const char *name) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_trace || tc->is_in_scenario) return tr_false;
  tc->is_in_scenario = tr_true;
  return TraceConstrainer_onTraceScenarioStart(tc, name);
}

TraceBool TraceConstrainer_traceScenarioParameters(TraceConstrainer *tc, int argc, const char** argv) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_transition) return tr_false;
  return TraceConstrainer_onTraceScenarioParameters(tc, argc,argv);
}

TraceBool TraceConstrainer_traceTestEngineProperty(TraceConstrainer *tc, const char *name,const char *value) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_transition) return tr_false;
  return TraceConstrainer_onTraceTestEngineProperty(tc, name,value);
}

TraceBool TraceConstrainer_traceScenarioEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_transition) return tr_false;
  tc->is_in_scenario = tr_false;
  return TraceConstrainer_onTraceScenarioEnd(tc);
}


/*
 * Transition level methods
 */
TraceBool TraceConstrainer_traceScenarioValue(TraceConstrainer *tc, const char *kind, const char *type, const char *name, const char *value) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario) return tr_false;
  return TraceConstrainer_onTraceScenarioValue(tc, kind, type, name, value);
}

TraceBool TraceConstrainer_traceState(TraceConstrainer *tc, const char *id) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario) return tr_false;
  return TraceConstrainer_onTraceState(tc, id);
}

TraceBool TraceConstrainer_traceTransitionStart(TraceConstrainer *tc, const char *id) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_transition) return tr_false;
  tc->is_in_transition = tr_true;
  return TraceConstrainer_onTraceTransitionStart(tc, id);
}

TraceBool TraceConstrainer_traceTransitionEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_transition || tc->is_in_model_operation) return tr_false;
  tc->is_in_transition = tr_false;
  return TraceConstrainer_onTraceTransitionEnd(tc);
}


/*
 * Oracle level methods
 */
TraceBool TraceConstrainer_traceModelOperationStart(TraceConstrainer *tc, const char *kind, const char *subsystem, const char *signature) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_model_operation || tc->is_in_series) return tr_false;
  tc->is_in_model_operation = tr_true;
  return TraceConstrainer_onTraceModelOperationStart(tc, kind,subsystem,signature);
}

TraceBool TraceConstrainer_traceModelOperationArgument(TraceConstrainer *tc, const char *type, const char *name, const char *value) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation || tc->is_in_oracle) return tr_false;
  return TraceConstrainer_onTraceModelOperationArgument(tc, type,name,value);
}

TraceBool TraceConstrainer_traceModelOperationResult(TraceConstrainer *tc, const char *type, const char *value) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation || tc->is_in_oracle) return tr_false;
  return TraceConstrainer_onTraceModelOperationResult(tc, type,value);
}

TraceBool TraceConstrainer_traceModelOperationIdentifier(TraceConstrainer *tc, long id) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation) return tr_false;
  return TraceConstrainer_onTraceModelOperationIdentifier(tc, id);
}

TraceBool TraceConstrainer_traceModelOperationChannel(TraceConstrainer *tc, long chid) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation) return tr_false;
  return TraceConstrainer_onTraceModelOperationChannel(tc, chid);
}

TraceBool TraceConstrainer_traceModelOperationTimestamp(TraceConstrainer *tc, const char *timestamp) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation) return tr_false;
  return TraceConstrainer_onTraceModelOperationTimestamp(tc, timestamp);
}

TraceBool TraceConstrainer_traceOracleStart(TraceConstrainer *tc, const char *subsystem, const char *signature, long refId) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!(tc->is_in_model_operation || tc->is_in_series) || tc->is_in_oracle) return tr_false;
  tc->is_in_oracle = tr_true;
  tc->is_in_precondition = tr_true;
  return TraceConstrainer_onTraceOracleStart(tc, subsystem,signature,refId);
}

TraceBool TraceConstrainer_tracePreconditionEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_oracle || tc->is_in_coverage_structure) return tr_false;
  tc->is_in_precondition = tr_false;
  return TraceConstrainer_onTracePreconditionEnd(tc);
}

TraceBool TraceConstrainer_traceOracleEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!(tc->is_in_model_operation || tc->is_in_series) || !tc->is_in_oracle || tc->is_in_coverage_structure) return tr_false;
  tc->is_in_oracle = tr_false;
  return TraceConstrainer_onTraceOracleEnd(tc);
}

TraceBool TraceConstrainer_traceModelOperationEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_model_operation || tc->is_in_oracle) return tr_false;
  tc->is_in_model_operation = tr_false;
  return TraceConstrainer_onTraceModelOperationEnd(tc);
}

TraceBool TraceConstrainer_traceSeriesStart(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario || tc->is_in_model_operation || tc->is_in_series) return tr_false;
  tc->is_in_series = tr_true;
  return TraceConstrainer_onTraceSeriesStart(tc);
}

TraceBool TraceConstrainer_traceSeriesEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_series || tc->is_in_oracle) return tr_false;
  tc->is_in_series = tr_false;
  return TraceConstrainer_onTraceSeriesEnd(tc);
}

TraceBool
    TraceConstrainer_keepOracleStart( TraceConstrainer *tc
                                    , const char *subsystem
                                    , const char *signature
                                    , long refId
                                    )
{
  if(    ( tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL )
      && !(TraceConstrainer_valid( tc ))
    )
    return tr_false;

  if(    !tc->is_in_scenario
      || tc->is_in_oracle
    )
    return tr_false;

  tc->is_in_oracle = tr_true;
  return TraceConstrainer_onKeepOracleStart( tc, subsystem, signature, refId );
}

TraceBool TraceConstrainer_keepOracleEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_oracle || tc->is_in_coverage_structure) return tr_false;
  tc->is_in_oracle = tr_false;
  return TraceConstrainer_onKeepOracleEnd(tc);
}


/*
 * Coverage level methods
 */
TraceBool TraceConstrainer_tracePrimeFormula(TraceConstrainer *tc, int id, TraceBool value) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_oracle || tc->is_in_coverage_structure) return tr_false;
  return TraceConstrainer_onTracePrimeFormula(tc, id,value);
}

TraceBool TraceConstrainer_traceCoverageElement(TraceConstrainer *tc, const char *structureId, const char *coverageId, int branchId) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if( tc->is_in_coverage_structure ) return tr_false;
  return TraceConstrainer_onTraceCoverageElement(tc,structureId,coverageId,branchId);
}

TraceBool TraceConstrainer_traceMark(TraceConstrainer *tc, const char *mark) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_scenario) return tr_false;
  return TraceConstrainer_onTraceMark(tc, mark);
}


TraceBool TraceConstrainer_traceCoverageStructureStart(TraceConstrainer *tc, const char* name)
{
  if(    (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL)
      && !TraceConstrainer_valid(tc)
    )
  {
    return tr_false;
  }
  if( tc->is_in_coverage_structure ) return tr_false;
  tc->is_in_coverage_structure = tr_true;
  return TraceConstrainer_onTraceCoverageStructureStart(tc, name);
}

TraceBool TraceConstrainer_traceFormulaeStart(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_coverage_structure || tc->is_in_formulae || tc->is_in_coverage) return tr_false;
  tc->is_in_formulae = tr_true;
  return TraceConstrainer_onTraceFormulaeStart(tc);
}

TraceBool TraceConstrainer_traceFormula(TraceConstrainer *tc, int id, const char *text) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_formulae) return tr_false;
  return TraceConstrainer_onTraceFormula(tc, id,text);
}

TraceBool TraceConstrainer_traceFormulaeEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_formulae) return tr_false;
  tc->is_in_formulae = tr_false;
  return TraceConstrainer_onTraceFormulaeEnd(tc);
}

TraceBool TraceConstrainer_traceCoverageStart(TraceConstrainer *tc, const char *id) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_coverage_structure || tc->is_in_formulae || tc->is_in_coverage) return tr_false;
  tc->is_in_coverage = tr_true;
  return TraceConstrainer_onTraceCoverageStart(tc, id);
}

TraceBool TraceConstrainer_traceElement(TraceConstrainer *tc, int id, const char *name) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_coverage) return tr_false;
  return TraceConstrainer_onTraceElement(tc, id,name);
}

TraceBool TraceConstrainer_traceCoverageEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_coverage) return tr_false;
  tc->is_in_coverage = tr_false;
  return TraceConstrainer_onTraceCoverageEnd(tc);
}

TraceBool TraceConstrainer_traceCoverageStructureEnd(TraceConstrainer *tc) {
  if( (tracer_debug_level >= TRACE_CONSTRAINER_DEBUG_LEVEL) && !TraceConstrainer_valid(tc) ) return tr_false;
  if (!tc->is_in_coverage_structure || tc->is_in_formulae || tc->is_in_coverage) return tr_false;
  tc->is_in_coverage_structure = tr_false;
  return TraceConstrainer_onTraceCoverageStructureEnd(tc);
}


/*
 * Other messages
 */
TraceBool TraceConstrainer_traceException(TraceConstrainer *tc, const char *kind) {
  return TraceConstrainer_onTraceException(tc, kind);
}

TraceBool TraceConstrainer_traceInterimException(TraceConstrainer *tc, const char *kind) {
  if (!tc->is_in_scenario) return tr_false;
  return TraceConstrainer_onTraceInterimException(tc, kind);
}

TraceBool TraceConstrainer_traceExceptionValue(TraceConstrainer *tc, const char *name, const char *value) {
  return TraceConstrainer_onTraceExceptionValue(tc, name,value);
}

TraceBool TraceConstrainer_traceExceptionInfo(TraceConstrainer *tc, const char *info) {
  return TraceConstrainer_onTraceExceptionInfo(tc, info);
}

TraceBool TraceConstrainer_traceInternalError(TraceConstrainer *tc, const char *info) {
  return TraceConstrainer_onTraceInternalError(tc, info);
}

TraceBool TraceConstrainer_traceSystemInfo(TraceConstrainer *tc, const char *info) {
  return TraceConstrainer_onTraceSystemInfo(tc, info);
}

TraceBool TraceConstrainer_traceUserInfo(TraceConstrainer *tc, const char *info) {
  return TraceConstrainer_onTraceUserInfo(tc, info);
}


/*
 * Current state methods
 */
TraceBool TraceConstrainer_isInTrace(TraceConstrainer *tc) {
  return tc->is_in_trace;
}

TraceBool TraceConstrainer_isInScenario(TraceConstrainer *tc) {
  return tc->is_in_scenario;
}

TraceBool TraceConstrainer_isInTransition(TraceConstrainer *tc) {
  return tc->is_in_transition;
}

TraceBool TraceConstrainer_isInSeries(TraceConstrainer *tc) {
  return tc->is_in_series;
}

TraceBool TraceConstrainer_isInModelOperation(TraceConstrainer *tc) {
  return tc->is_in_model_operation;
}

TraceBool TraceConstrainer_isInOracle(TraceConstrainer *tc) {
  return tc->is_in_oracle;
}

TraceBool TraceConstrainer_isInPrecondition(TraceConstrainer *tc) {
  return tc->is_in_precondition;
}

TraceBool TraceConstrainer_isInCoverageStructure(TraceConstrainer *tc) {
  return tc->is_in_coverage_structure;
}

TraceBool TraceConstrainer_isInFormulae(TraceConstrainer *tc) {
  return tc->is_in_formulae;
}

TraceBool TraceConstrainer_isInCoverage(TraceConstrainer *tc) {
  return tc->is_in_coverage;
}




/*
 * Constructor and destructor
 */
TraceConstrainer * TraceConstrainer_TraceConstrainer() {
  TraceConstrainer *tc = (TraceConstrainer *)malloc(sizeof(*tc));
  if(tc != NULL) {
    tc->is_in_trace = tr_false;
    tc->is_in_scenario = tr_false;
    tc->is_in_transition = tr_false;
    tc->is_in_series = tr_false;
    tc->is_in_model_operation = tr_false;
    tc->is_in_oracle = tr_false;
    tc->is_in_precondition = tr_false;
    tc->is_in_coverage_structure = tr_false;
    tc->is_in_formulae = tr_false;
    tc->is_in_coverage = tr_false;

    tc->startTrace = TraceConstrainer_startTrace;
    tc->endTrace = TraceConstrainer_endTrace;
    tc->traceScenarioStart = TraceConstrainer_traceScenarioStart;
    tc->traceScenarioParameters = TraceConstrainer_traceScenarioParameters;
    tc->traceTestEngineProperty = TraceConstrainer_traceTestEngineProperty;
    tc->traceScenarioEnd = TraceConstrainer_traceScenarioEnd;
    tc->traceScenarioValue = TraceConstrainer_traceScenarioValue;
    tc->traceState = TraceConstrainer_traceState;
    tc->traceTransitionStart = TraceConstrainer_traceTransitionStart;
    tc->traceTransitionEnd = TraceConstrainer_traceTransitionEnd;
    tc->traceModelOperationStart = TraceConstrainer_traceModelOperationStart;
    tc->traceModelOperationArgument = TraceConstrainer_traceModelOperationArgument;
    tc->traceModelOperationResult = TraceConstrainer_traceModelOperationResult;
    tc->traceModelOperationIdentifier = TraceConstrainer_traceModelOperationIdentifier;
    tc->traceModelOperationChannel = TraceConstrainer_traceModelOperationChannel;
    tc->traceModelOperationTimestamp = TraceConstrainer_traceModelOperationTimestamp;
    tc->traceOracleStart = TraceConstrainer_traceOracleStart;
    tc->tracePreconditionEnd = TraceConstrainer_tracePreconditionEnd;
    tc->traceOracleEnd = TraceConstrainer_traceOracleEnd;
    tc->traceModelOperationEnd = TraceConstrainer_traceModelOperationEnd;
    tc->traceSeriesStart = TraceConstrainer_traceSeriesStart;
    tc->traceSeriesEnd = TraceConstrainer_traceSeriesEnd;
    tc->keepOracleStart = TraceConstrainer_keepOracleStart;
    tc->keepOracleEnd = TraceConstrainer_keepOracleEnd;
    tc->tracePrimeFormula = TraceConstrainer_tracePrimeFormula;
    tc->traceCoverageElement = TraceConstrainer_traceCoverageElement;
    tc->traceMark = TraceConstrainer_traceMark;
    tc->traceCoverageStructureStart = TraceConstrainer_traceCoverageStructureStart;
    tc->traceFormulaeStart = TraceConstrainer_traceFormulaeStart;
    tc->traceFormula = TraceConstrainer_traceFormula;
    tc->traceFormulaeEnd = TraceConstrainer_traceFormulaeEnd;
    tc->traceCoverageStart = TraceConstrainer_traceCoverageStart;
    tc->traceElement = TraceConstrainer_traceElement;
    tc->traceCoverageEnd = TraceConstrainer_traceCoverageEnd;
    tc->traceCoverageStructureEnd = TraceConstrainer_traceCoverageStructureEnd;
    tc->traceException = TraceConstrainer_traceException;
    tc->traceInterimException = TraceConstrainer_traceInterimException;
    tc->traceExceptionValue = TraceConstrainer_traceExceptionValue;
    tc->traceExceptionInfo = TraceConstrainer_traceExceptionInfo;
    tc->traceInternalError = TraceConstrainer_traceInternalError;
    tc->traceSystemInfo = TraceConstrainer_traceSystemInfo;
    tc->traceUserInfo = TraceConstrainer_traceUserInfo;
    tc->isInTrace = TraceConstrainer_isInTrace;
    tc->isInScenario = TraceConstrainer_isInScenario;
    tc->isInTransition = TraceConstrainer_isInTransition;
    tc->isInSeries = TraceConstrainer_isInSeries;
    tc->isInModelOperation = TraceConstrainer_isInModelOperation;
    tc->isInOracle = TraceConstrainer_isInOracle;
    tc->isInPrecondition = TraceConstrainer_isInPrecondition;
    tc->isInCoverageStructure = TraceConstrainer_isInCoverageStructure;
    tc->isInFormulae = TraceConstrainer_isInFormulae;
    tc->isInCoverage = TraceConstrainer_isInCoverage;

    tc->onStartTrace = TraceConstrainer_onStartTrace;
    tc->onEndTrace = TraceConstrainer_onEndTrace;
    tc->onTraceScenarioStart = TraceConstrainer_onTraceScenarioStart;
    tc->onTraceScenarioParameters = TraceConstrainer_onTraceScenarioParameters;
    tc->onTraceTestEngineProperty = TraceConstrainer_onTraceTestEngineProperty;
    tc->onTraceScenarioEnd = TraceConstrainer_onTraceScenarioEnd;
    tc->onTraceScenarioValue = TraceConstrainer_onTraceScenarioValue;
    tc->onTraceState = TraceConstrainer_onTraceState;
    tc->onTraceTransitionStart = TraceConstrainer_onTraceTransitionStart;
    tc->onTraceTransitionEnd = TraceConstrainer_onTraceTransitionEnd;
    tc->onTraceModelOperationStart = TraceConstrainer_onTraceModelOperationStart;
    tc->onTraceModelOperationArgument = TraceConstrainer_onTraceModelOperationArgument;
    tc->onTraceModelOperationResult = TraceConstrainer_onTraceModelOperationResult;
    tc->onTraceModelOperationIdentifier = TraceConstrainer_onTraceModelOperationIdentifier;
    tc->onTraceModelOperationChannel = TraceConstrainer_onTraceModelOperationChannel;
    tc->onTraceModelOperationTimestamp = TraceConstrainer_onTraceModelOperationTimestamp;
    tc->onTraceOracleStart = TraceConstrainer_onTraceOracleStart;
    tc->onTracePreconditionEnd = TraceConstrainer_onTracePreconditionEnd;
    tc->onTraceOracleEnd = TraceConstrainer_onTraceOracleEnd;
    tc->onTraceModelOperationEnd = TraceConstrainer_onTraceModelOperationEnd;
    tc->onTraceSeriesStart = TraceConstrainer_onTraceSeriesStart;
    tc->onTraceSeriesEnd = TraceConstrainer_onTraceSeriesEnd;
    tc->onKeepOracleStart = TraceConstrainer_onKeepOracleStart;
    tc->onKeepOracleEnd = TraceConstrainer_onKeepOracleEnd;
    tc->onTracePrimeFormula = TraceConstrainer_onTracePrimeFormula;
    tc->onTraceCoverageElement = TraceConstrainer_onTraceCoverageElement;
    tc->onTraceMark = TraceConstrainer_onTraceMark;
    tc->onTraceCoverageStructureStart = TraceConstrainer_onTraceCoverageStructureStart;
    tc->onTraceFormulaeStart = TraceConstrainer_onTraceFormulaeStart;
    tc->onTraceFormula = TraceConstrainer_onTraceFormula;
    tc->onTraceFormulaeEnd = TraceConstrainer_onTraceFormulaeEnd;
    tc->onTraceCoverageStart = TraceConstrainer_onTraceCoverageStart;
    tc->onTraceElement = TraceConstrainer_onTraceElement;
    tc->onTraceCoverageEnd = TraceConstrainer_onTraceCoverageEnd;
    tc->onTraceCoverageStructureEnd = TraceConstrainer_onTraceCoverageStructureEnd;
    tc->onTraceException = TraceConstrainer_onTraceException;
    tc->onTraceInterimException = TraceConstrainer_onTraceInterimException;
    tc->onTraceExceptionValue = TraceConstrainer_onTraceExceptionValue;
    tc->onTraceExceptionInfo = TraceConstrainer_onTraceExceptionInfo;
    tc->onTraceInternalError = TraceConstrainer_onTraceInternalError;
    tc->onTraceSystemInfo = TraceConstrainer_onTraceSystemInfo;
    tc->onTraceUserInfo = TraceConstrainer_onTraceUserInfo;
    tc->valid = TraceConstrainer_valid;
  }
  return tc;
}

void TraceConstrainer_delete(TraceConstrainer *tc)
{
  if(tc != NULL) free(tc);
}

