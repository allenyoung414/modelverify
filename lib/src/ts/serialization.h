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
 * FILE:        serialization.h                     
 * AUTHOR:      Alexey V. Khoroshilov               
 * CREATION:    8 Jul 2003                          
 * DESCRIPTION: Declares functions needed           
 *              for serialization.                  
 * SCOPE:       It is an internal header.           
 */

#ifndef TS_SERIALIZATION_H
#define TS_SERIALIZATION_H

#include "ts/register_tsi.h"
#include "ts/engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                     Serialization Manager                      **/
/********************************************************************/
Serialization* createLinearSerialization(Interactions* interactions);
Serialization* createDistributedSerialization(Interactions* interactions);


/********************************************************************/
/**                           Hyper Oracle                         **/
/********************************************************************/
bool serialize_interactions(Interactions* interactions,ModelStateManager* model_state);


#ifdef __cplusplus
}
#endif

#endif
