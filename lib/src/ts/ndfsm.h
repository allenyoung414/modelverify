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
 * FILE:        ndfsm.h                            
 * AUTHOR:      Alexey V. Khoroshilov (hed)        
 * CREATION:    11 Apr 2005                        
 * DESCRIPTION: Implementation of the test engine  
 *              for testing a group of functions   
 *              by traversing a nondetermenictic   
 *              graph contained mutually connected 
 *              deterministic spanning subgraph.   
 * SCOPE:       It is an interface header.         
 * VERSION AND LIST OF CHANGES:                    
 */

#ifndef TS_NDFSM_H
#define TS_NDFSM_H

#include "ts/scenario.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************/
/**                      NDFSM data type                           **/
/********************************************************************/
typedef TestScenario ndfsm;


/********************************************************************/
/**                  Start NDFSM engine function                   **/
/********************************************************************/
bool start_ndfsm(int argc, char** argv, ndfsm* test);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
