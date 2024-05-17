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
 * File: c_tracer.h
 * Description:
 *       This is a header file of the tracer subsystem.
 */

#ifndef __C_TRACER_H__
#define __C_TRACER_H__

/*
 * c_bool type is intended to present equally the Boolean type in both C and C++
 */
typedef int c_bool;


#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Environment
   */
  void setTraceUserEnv(const char *name, const char *value);

  /*
   * Encoding and format
   */
  void setTraceEncoding(const char *encoding);

  void setTraceDataFormatXML();
  void setTraceDataFormatString();

  /*
   * Tracer configuration
   */
  c_bool addTraceToConsole(void);
  c_bool removeTraceToConsole(void);

  c_bool addTraceToFile(const char *name);
  c_bool removeTraceToFile(const char *name);

  c_bool isTraceEnabled(void);

  c_bool setTraceAccidental(c_bool enable);

  /*
   * Trace level messages
   */
  void startTrace(void);
  void endTrace(void);

  /*
   * Scenario level messages
   */
  void traceScenarioStart(const char* name);
  void traceScenarioParameters(int argc, const char** argv);
  void traceTestEngineProperty(const char* name,const char* value);
  void traceScenarioEnd(void);

  /*
   * Transition level messages
   */

  void traceScenarioValue (const char *kind, const char *type, const char *name, const char *value);
  void traceState(const char *id);
  void traceTransitionStart(const char *id);
  void traceTransitionEnd(void);

  /*
   * Oracle level messages
   */
  void traceModelOperationStart(const char *kind,const char *subsystem,const char *signature);
  void traceModelOperationArgument(const char *type, const char *name, const char *value);
  void traceModelOperationResult(const char *type, const char *value);
  void traceModelOperationIdentifier(long id);
  void traceModelOperationChannel(long chid);
  void traceModelOperationTimestamp(const char *timestamp);
  void traceOracleStart(const char *subsystem,const char *signature, long refId);
  void tracePreconditionEnd(void);
  void traceOracleEnd(void);
  void traceModelOperationEnd(void);
  void traceSeriesStart(void);
  void traceSeriesEnd(void);

  /*
   * Coverage level messages
   */
  void tracePrimeFormula(int id, c_bool value);
  void traceCoverageElement(const char *structureId, const char *coverageId, int branchId);
  void traceMark(const char *mark);

  void traceCoverageStructureStart(const char* name);
  void traceFormulaeStart(void);
  void traceFormula(int id, const char *text);
  void traceFormulaeEnd(void);
  void traceCoverageStart(const char *id);
  void traceElement(int id, const char *name);
  void traceCoverageEnd(void);
  void traceCoverageStructureEnd(void);

  /*
   * Other messages
   */
  void traceException(const char *kind);
  void traceInterimException(const char *kind);
  void traceExceptionValue(const char *name, const char *value);
  void traceExceptionInfo(const char *info);
  void traceInternalError(const char *info);
  void traceSystemInfo(const char *info);
  void traceUserInfo(const char *info);
  void traceFormattedUserInfo(const char *format,...);


  /*
   * Trace state functions
   */
  c_bool isInTrace(void);
  c_bool isInScenario(void);
  c_bool isInTransition(void);
  c_bool isInSeries(void);
  c_bool isInModelOperation(void);
  c_bool isInOracle(void);
  c_bool isInPrecondition(void);
  c_bool isInCoverageStructure(void);
  c_bool isInFormulae(void);
  c_bool isInCoverage(void);
  int    getTraceId(void);

  /*
   * Abnormal close of trace
   */
  void flushTrace(void);
  void closeTrace(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __C_TRACER_H__ */
