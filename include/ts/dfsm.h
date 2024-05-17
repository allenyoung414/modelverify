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
 * FILE:        dfsm.h                              
 * AUTHOR:      Vitaliy Omelchenko (vitaliy)        
 * CREATION:    2 Aug 2001                          
 * DESCRIPTION: Contents data structures and        
 *              forward declarations functions      
 *              that needs for testing group of     
 *              functions by browsing determenistic 
 *              mutually connected graph.           
 * SCOPE:       It is an interface header.          
 * VERSION AND LIST OF CHANGES:                     
 *  1 Jul 2003  (hed)                               
 *   Support of deferred reactions has been added   
 */

#ifndef TS_scenario_H
#define TS_scenario_H

#include "ts/scenario.h"

/********************************************************************/
/**                     DFSM data field types                      **/
/********************************************************************/
/*
 */
typedef bool (*PtrInit)(int, char**);


/*
 */
typedef void (*PtrFinish)(void);


/*
  getState - specific for test (and maybe realization),
             it evaluates and returns current state (node in
             traversing graph) of test, state is always the
             object type;
*/
typedef Object* (*PtrGetState)(void);

typedef String* (*PtrGetStateId)(Object*);

/********************************************************************/
/**                      DFSM data type                            **/
/********************************************************************/

typedef TestScenario dfsm;

/********************************************************************/
/**                  Start DFSM engine function                    **/
/********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

bool start_dfsm(int argc, char** argv, dfsm* td);
#ifdef __cplusplus
}

#endif

#endif
