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
 * File: Tracer.h
 * Description:
 *       This is a header file of Tracer.
 */

#ifndef __TRACER_H__
#define __TRACER_H__

#include "TraceWriter.h"
#include "TraceMultiWriter.h"
#include "TraceFormatter.h"
#include "TraceConstrainer.h"
#include "TraceBool.h"
#include <atl/stringbuffer.h>
#include <ts/system.h>

#define ENCODING_STR_LEN 1023

#ifdef __cplusplus
extern "C" {
#endif

struct _Tracer
{
  /*
   * Environment field names
   */
  char* ENV_HOST;
  char* ENV_OS;
  char* ENV_PRODUCT_NAME;
  char* ENV_PRODUCT_VERSION;
  char* ENV_PRODUCT_BUILD;

//  std::map<string,string> environment;
  TraceList environment;

  /*
   * Encoding
   */
  char encoding[ENCODING_STR_LEN + 1];

  /* Semaphore */
  TSSemaphore semaphore;

  /*
   * Main actors
   */
  TraceWriteController* writer;
  TraceFormatter*       formatter;
  TraceConstrainer*     constrainer;

  /*
   * Trace identification subsystem
   */
  // int traceIdSeq;
  int traceId;

  /*
   * Transition identification subsystem
   */
  int transitionIdSeq;
//  std::map<string,int> transitions;
  TraceList transitions;

  /*
   * Fields for accidental transitions
   */
  TraceBool traceAccidentalTransitions;
  StringBuffer *transitionBuffer;
  TraceWriter *transitionBuffer_wr;
  TraceBool needTrace;
  TraceBool needTraceState;

  /*
   * Model operation identification subsystem
   */
  unsigned int modelOperationIdSeq;
//  std::map<long,unsigned> model_operations;
  TraceList model_operations;

  /*
   * Fields for keeping
   */
  TraceBool wasSeries;
  StringBuffer *keepBuffer;
  TraceWriter *keepBuffer_wr;

  /*
   * Exception values
   */
//  std::list< pair<string,string> > exception_values;
//  std::list< string >              exception_infos;
  TraceList exception_values;
  TraceList exception_infos;

};
typedef struct _Tracer class_Tracer;



/*
 * Get/set default server
 */
void tracer_setDefaultTracer(class_Tracer * tracer);
class_Tracer * tracer_getDefaultTracer();

/*
 * Constructor and destructor
 */
class_Tracer * tracer_createTracer();
void tracer_deleteTracer(class_Tracer * tracer);

/*
 * Locking (for multithreaded environment). Cannot be nested!
 */
void tracer_lock(class_Tracer * tracer);
void tracer_unlock(class_Tracer * tracer);

/*
 * Methods for getting current servers
 */
TraceWriteController *tracer_getWriter(class_Tracer * tracer);
TraceFormatter *tracer_getFormatter(class_Tracer * tracer);
TraceConstrainer *tracer_getConstrainer(class_Tracer * tracer);

/*
 * Methods for setting user environment
 */
void tracer_setUserEnv(class_Tracer *tracer, const char *name, const char *value);

/*
 * Methods for getting and setting encoding
 */
void tracer_setEncoding(class_Tracer * tracer, const char* encoding);

/*
 * Methods for setting current servers
 */
TraceBool tracer_addWriter(class_Tracer * tracer, const char* name,TraceWriter* writer);
TraceBool tracer_hasWriter(class_Tracer * tracer, const char* name);
TraceBool tracer_removeWriter(class_Tracer * tracer, const char* name);
TraceBool tracer_isTraceEnabled(class_Tracer * tracer);
TraceFormatter* tracer_setFormatter(class_Tracer * tracer, TraceFormatter* formatter);
TraceConstrainer* tracer_setConstrainer(class_Tracer * tracer, TraceConstrainer* constrainer);

/*
 * Method for getting trace id
 */
int tracer_getTraceId(class_Tracer * tracer);

/*
 * Enable/disable tracing of accidental transitions (disabled by default)
 */
TraceBool tracer_setTraceAccidental(class_Tracer * tracer, TraceBool enable);

/*
 * Trace level methods
 */
void tracer_startTrace(class_Tracer * tracer);
void tracer_endTrace(class_Tracer * tracer);

/*
 * Scenario level methods
 */
void tracer_traceScenarioStart(class_Tracer * tracer, const char* name);
void tracer_traceScenarioParameters(class_Tracer * tracer, int argc, const char** argv);
void tracer_traceTestEngineProperty(class_Tracer * tracer, const char* name, const char* value);
void tracer_traceScenarioEnd(class_Tracer * tracer);

/*
 * Transition level methods
 */
void tracer_traceScenarioValue(class_Tracer * tracer, const char* kind, const char* type, const char* name, const char* value);
void tracer_traceState(class_Tracer * tracer, const char* id);
void tracer_traceTransitionStart(class_Tracer * tracer, const char* id);
void tracer_traceTransitionEnd(class_Tracer * tracer);

/*
 * Oracle level methods
 */
void tracer_traceModelOperationStart(class_Tracer * tracer, const char* kind, const char* subsystem, const char* signature);
void tracer_traceModelOperationArgument(class_Tracer * tracer, const char* type, const char* name, const char* value);
void tracer_traceModelOperationResult(class_Tracer * tracer, const char* type, const char* value);
void tracer_traceModelOperationChannel(class_Tracer * tracer, long chid);
void tracer_traceModelOperationIdentifier(class_Tracer * tracer, long id);
void tracer_traceModelOperationTimestamp(class_Tracer * tracer, const char* timestamp);
void tracer_traceOracleStart(class_Tracer * tracer, const char* subsystem, const char* signature, long refId);
void tracer_tracePreconditionEnd(class_Tracer * tracer);
void tracer_traceOracleEnd(class_Tracer * tracer);
void tracer_traceModelOperationEnd(class_Tracer * tracer);
void tracer_traceSeriesStart(class_Tracer * tracer);
void tracer_traceSeriesEnd(class_Tracer * tracer);

/*
 * Coverage level methods
 */
void tracer_tracePrimeFormula(class_Tracer * tracer, int id, TraceBool value);
void tracer_traceCoverageElement(class_Tracer * tracer, const char *structureId, const char* coverageId, int branchId);
void tracer_traceMark(class_Tracer * tracer, const char* mark);

void tracer_traceCoverageStructureStart(class_Tracer * tracer, const char* name);
void tracer_traceFormulaeStart(class_Tracer * tracer);
void tracer_traceFormula(class_Tracer * tracer, int id, const char* text);
void tracer_traceFormulaeEnd(class_Tracer * tracer);
void tracer_traceCoverageStart(class_Tracer * tracer, const char* id);
void tracer_traceElement(class_Tracer * tracer, int id, const char* name);
void tracer_traceCoverageEnd(class_Tracer * tracer);
void tracer_traceCoverageStructureEnd(class_Tracer * tracer);

/*
 * Other messages
 */
void tracer_traceException(class_Tracer * tracer, const char* kind);
void tracer_traceInterimException(class_Tracer * tracer, const char* kind);
void tracer_traceExceptionValue(class_Tracer * tracer, const char* name, const char* value);
void tracer_traceExceptionInfo(class_Tracer * tracer, const char* info);
void tracer_traceInternalError(class_Tracer * tracer, const char* info);
void tracer_traceSystemInfo(class_Tracer * tracer, const char* info);
void tracer_traceUserInfo(class_Tracer * tracer, const char* info);



#ifdef __cplusplus
}
#endif

#endif /* __TRACER_H__ */
