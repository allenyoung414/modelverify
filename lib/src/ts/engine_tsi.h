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
 * FILE:        engine_tsi.h                        
 * AUTHOR:      Alexey V. Khoroshilov               
 * CREATION:    18 April 2005                       
 * DESCRIPTION: Common properties of test engines.  
 * SCOPE:       It is an internal header.           
 */

#ifndef TS_ENGINE_INTERNAL_INTERFACE_H
#define TS_ENGINE_INTERNAL_INTERFACE_H

#include "ts/engine.h"
#include "ts/generated.h"
#include "utils/assertion.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************/
/**                      Additional functions                      **/
/********************************************************************/
char* format_time(void);
bool file_exists(const char* name);


/********************************************************************/
/**              Quick Access to Finish Mode property              **/
/********************************************************************/
extern FinishMode tsFinishMode;
extern int tsNumberOfErrorsToFinish;


/********************************************************************/
/**                       Test System Mode                         **/
/********************************************************************/
void setTSMode(TSMode mode);


/********************************************************************/
/**               Test System Assertion Exit Function              **/
/********************************************************************/
AssertionExit ts_setup_assert_function(void);
void ts_restore_assert_function(AssertionExit prev_assert);


/********************************************************************/
/**                      Calculate TT filename                     **/
/********************************************************************/
char* getTTFileName(const char *basename, const char *prefix, const char *extension);


/********************************************************************/
/**                   Process Standard Parameters                  **/
/********************************************************************/
typedef struct TestSystemParameters
{
  bool   no_trace;
  bool   console;
  char** files;
  int    argc;
  char** argv;
} TestSystemParameters;

TestSystemParameters* processStandardParameters( const char* name
                                               , const char* prefix
                                               , ScenarioFunctionID* scenario_actions
                                               , int argc
                                               , char** argv
                                               );

void disableStandardParameters(TestSystemParameters* pars);


/********************************************************************/
/**                     Current Test Scenario                      **/
/********************************************************************/
void setCurrentTestScenarioName(const char* name);


/********************************************************************/
/**                      Model State Syncronizer                   **/
/********************************************************************/
bool onFrameEnd( ModelStateManager* model_state, bool verdict );
bool syncronize_ModelState( ModelStateManager* model_state );


/********************************************************************/
/**                           Arc Symbol                           **/
/********************************************************************/
typedef struct ArcSymbol
{
  int     kind;
  Object* value;
} ArcSymbol;

void init_ArcSymbol( ArcSymbol* symbol );
void destroy_ArcSymbol( ArcSymbol* symbol );
bool isNull_ArcSymbol( ArcSymbol* symbol );


/********************************************************************/
/**                        Arc Symbol Executor                     **/
/********************************************************************/
typedef struct ArcSymbolExecutor
{
  ModelStateManager   model_state;
  ScenarioFunctionID* actions;
} ArcSymbolExecutor;

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
