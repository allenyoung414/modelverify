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
 * File: c_tracer.c
 * Description:
 *       This file contains an implementation of the C interface
 *       of the tracer subsystem.
 */

#include <stdio.h>
#include <stdarg.h>
#include "c_tracer.h"
#include "Tracer.h"
#include "TraceConsoleWriter.h"
#include "TraceFileWriter.h"

#include <atl/string.h>
#include <utils/assertion.h>
#include <ts/convex.h>

#define BOOL_VALUE(x)  (x != 0)


/*
 * User environment
 */
void setTraceUserEnv(const char *name, const char *value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_setUserEnv(tracer, name, value);
}

/*
 * Encoding and format
 */

void setTraceEncoding(const char *encoding)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_setEncoding(tracer,encoding);
}


static short dataFormatXML = 1;

void setTraceDataFormatXML()
{
	dataFormatXML = 1;
}

void setTraceDataFormatString()
{
	dataFormatXML = 0;
}

String* toTrace(Object* ref)
{
	if (dataFormatXML)
		 return toXML(ref);
	else return toString(ref);
}

String* ts_to_trace_sectype(SECTypeDesc* desc,void* ptr)
{
	if (dataFormatXML)
		 return ts_to_XML_sectype(desc, ptr);
	else return ts_to_string_sectype(desc, ptr);
}

/*
 * Tracer configuration
 */

c_bool addTraceToConsole()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  if (tracer_hasWriter(tracer,"$console"))
   {
    tracer_addWriter(tracer,"$console",NULL);
    return tr_true;
   }
  return tracer_addWriter(tracer, "$console", TraceConsoleWriter_create() );
}

c_bool removeTraceToConsole(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  return tracer_removeWriter(tracer, "$console" );
}

c_bool addTraceToFile(const char *name)
{
  FILE *out;
  class_Tracer *tracer = tracer_getDefaultTracer();
  if (tracer_hasWriter(tracer,name))
   {
    tracer_addWriter(tracer,name,NULL);
    return tr_true;
   }
#ifdef __CYGWIN__
  out = fopen(name,"w");
#else
#  ifdef USE_FOPEN64
  out = fopen64(name,"w");
#  else
  out = fopen(name,"w");
#  endif
#endif
  if (out == NULL) return tr_false;
  return tracer_addWriter(tracer,name, TraceFileWriter_create(out));
}

c_bool removeTraceToFile(const char *name)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  return tracer_removeWriter(tracer,name);
}

c_bool isTraceEnabled(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  return tracer_isTraceEnabled(tracer);
}

c_bool setTraceAccidental(c_bool enable)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  return tracer_setTraceAccidental(tracer,BOOL_VALUE(enable));
}


/*
 * Trace level messages
 */

void startTrace()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_startTrace(tracer);
}

void endTrace()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_endTrace(tracer);
}


/*
 * Scenario level messages
 */

void traceScenarioStart(const char *name)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceScenarioStart(tracer,name);
}

void traceScenarioParameters(int argc, const char** argv)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceScenarioParameters(tracer,argc,argv);
}

void traceTestEngineProperty(const char* name,const char* value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceTestEngineProperty(tracer,name,value);
}

void traceScenarioEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceScenarioEnd(tracer);
}


/*
 * Transition level messages
 */

void traceScenarioValue (const char *kind, const char *type, const char *name, const char *value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceScenarioValue(tracer, kind, type, name, value);
}

void traceState(const char *id)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceState(tracer,id);
}

void traceTransitionStart(const char *id)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceTransitionStart(tracer,id);
}

void traceTransitionEnd(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceTransitionEnd(tracer);
}


/*
 * Oracle level messages
 */

void traceModelOperationStart(const char *kind,const char *subsystem,const char *signature)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationStart(tracer,kind,subsystem,signature);
}

void traceModelOperationArgument(const char *type, const char *name, const char *value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationArgument(tracer,type,name,value);
}

void traceModelOperationResult(const char *type, const char *value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationResult(tracer,type,value);
}

void traceModelOperationIdentifier(long id)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationIdentifier(tracer,id);
}

void traceModelOperationChannel(long chid)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationChannel(tracer,chid);
}

void traceModelOperationTimestamp(const char *timestamp)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationTimestamp(tracer,timestamp);
}

void traceOracleStart(const char *subsystem,const char *signature, long refId)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceOracleStart(tracer,subsystem,signature,refId);
}

void tracePreconditionEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_tracePreconditionEnd(tracer);
}

void traceOracleEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceOracleEnd(tracer);
}

void traceModelOperationEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceModelOperationEnd(tracer);
}

void traceSeriesStart(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceSeriesStart(tracer);
}

void traceSeriesEnd(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceSeriesEnd(tracer);
}


/*
 * Coverage level messages
 */

void tracePrimeFormula(int id, c_bool value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_tracePrimeFormula(tracer, id, BOOL_VALUE(value) );
}


void traceCoverageElement(const char *structureId, const char *coverageId, int branchId)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceCoverageElement(tracer,structureId,coverageId,branchId);
}

void traceMark(const char *mark)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceMark(tracer,mark);
}


void traceCoverageStructureStart( const char* name )
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceCoverageStructureStart(tracer, name);
}

void traceFormulaeStart()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceFormulaeStart(tracer);
}

void traceFormula(int id, const char *text)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceFormula(tracer,id,text);
}

void traceFormulaeEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceFormulaeEnd(tracer);
}

void traceCoverageStart(const char *id)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceCoverageStart(tracer,id);
}

void traceElement(int id, const char *name)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceElement(tracer,id,name);
}

void traceCoverageEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceCoverageEnd(tracer);
}

void traceCoverageStructureEnd()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceCoverageStructureEnd(tracer);
}


/*
 * Other messages
 */

void traceException(const char *kind)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceException(tracer,kind);
}

void traceInterimException(const char *kind)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceInterimException(tracer,kind);
}

void traceExceptionValue(const char *name, const char *value)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceExceptionValue(tracer,name,value);
}

void traceExceptionInfo(const char *info)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceExceptionInfo(tracer,info);
}

void traceInternalError(const char *info)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceInternalError(tracer,info);
}

void traceSystemInfo(const char *info)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceSystemInfo(tracer,info);
}

void traceUserInfo(const char *info)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  tracer_traceUserInfo(tracer,info);
}


void traceFormattedUserInfo(const char *format,...)
{
StringBuffer * buffer = create_StringBuffer();
int i, j, len;
va_list arg_list;
String *str, *res;

  class_Tracer *tracer = tracer_getDefaultTracer();
  va_start(arg_list, format);

  len = strlen(format);
  j = 0;
  for(i=0; i<len; i++) {
    if (format[i] == '$') {
      if (i < len - 5 && memcmp(format+i,"$(obj)",6) == 0) {
        void* obj = va_arg( arg_list, void* );

        appendCharArray_StringBuffer( r(buffer), format+j, i-j );
        appendString_StringBuffer( r(buffer), toString(obj) );

        i = i + 5;
        j = i + 1;
      }
    }
  }
  appendCharArray_StringBuffer( r(buffer), format+j, i-j );

  str = toString( buffer );		// destroy buffer
  res = vformat_String( toCharArray_String( r(str) ), arg_list );

  tracer_traceUserInfo(tracer, toCharArray_String( r(res) ));

  destroy(str);
  destroy(res);
  va_end(arg_list);
}


/*
 * Trace state functions
 */

c_bool isInTrace()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInTrace)) ? (*constrainer->isInTrace)(constrainer) : tr_false;
}

c_bool isInScenario()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInScenario)) ? (*constrainer->isInScenario)(constrainer) : tr_false;
}

c_bool isInTransition()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInTransition)) ? (*constrainer->isInTransition)(constrainer) : tr_false;
}

c_bool isInSeries(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInSeries)) ? (*constrainer->isInSeries)(constrainer) : tr_false;
}

c_bool isInModelOperation()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInModelOperation)) ? (*constrainer->isInModelOperation)(constrainer) : tr_false;
}

c_bool isInOracle()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInOracle)) ? (*constrainer->isInOracle)(constrainer) : tr_false;
}

c_bool isInPrecondition(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInPrecondition)) ? (*constrainer->isInPrecondition)(constrainer) : tr_false;
}

c_bool isInCoverageStructure()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInCoverage)) ? (*constrainer->isInCoverage)(constrainer) : tr_false;
}

c_bool isInFormulae()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInFormulae)) ? (*constrainer->isInFormulae)(constrainer) : tr_false;
}

c_bool isInCoverage()
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return (constrainer && (constrainer->isInCoverage)) ? (*constrainer->isInCoverage)(constrainer) : tr_false;
}

int getTraceId(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceConstrainer *constrainer = tracer_getConstrainer(tracer);
  return tracer_getTraceId(tracer);
}


/*
 * Abnormal close of trace
 */
void flushTrace(void)
{
  class_Tracer *tracer = tracer_getDefaultTracer();
  TraceWriteController *writer = tracer_getWriter(tracer);

  tracer_lock( tracer );
  (*writer->flush)(writer->state);
  tracer_unlock( tracer );
}

void closeTrace(void)
{
  if (isInFormulae())
    traceFormulaeEnd();
  if (isInCoverage())
    traceCoverageEnd();
  if (isInCoverageStructure())
    traceCoverageStructureEnd();
  if (isInOracle())
   {
    if (isInPrecondition())
      tracePreconditionEnd();
    traceOracleEnd();
   }
  if (isInModelOperation())
    traceModelOperationEnd();
  if (isInSeries())
    traceSeriesEnd();
  if (isInTransition()) {
    traceTransitionEnd();
  }
  if (isInScenario())
    traceScenarioEnd();
  if (isInTrace())
    endTrace();
  flushTrace();
}
