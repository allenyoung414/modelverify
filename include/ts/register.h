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
 * FILE:        register.h                          
 * AUTHOR:      Alexey V. Khoroshilov               
 * CREATION:    13 Apr 2003                         
 * DESCRIPTION: Contains functions needed           
 *              for work with reactions.            
 * SCOPE:       It is an interface header.          
 */

#ifndef TS_REGISTER_H
#define TS_REGISTER_H

#include "atl/object.h"
#include "ts/timemark.h"
#include "ts/generated.h"
#include "utils/boolean.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                        Register Reaction                       **/
/********************************************************************/
void registerReaction(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data
                     );

void registerReactionWithTimeMark(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data,
         TimeMark        mark
                                 );

void registerReactionWithTimeInterval(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data,
         TimeInterval    interval
                                     );


/********************************************************************/
/**                     Register Wrong Reaction                    **/
/********************************************************************/
void registerWrongReaction(const char* info);


/********************************************************************/
/**                            Catchers                            **/
/********************************************************************/
typedef bool (*ReactionCatcherFuncType)(void*);

void registerReactionCatcher(ReactionCatcherFuncType catcher,void* par);
bool unregisterReactionCatcher(ReactionCatcherFuncType catcher,void* par);
bool unregisterReactionCatchers(ReactionCatcherFuncType catcher);


/********************************************************************/
/**                       Register Stimulus                        **/
/********************************************************************/
ChannelID setStimulusChannel(ChannelID chid);
ChannelID getStimulusChannel(void);

void registerStimulus(
         const char*     name,
         SpecificationID stimulusID,
         Object*         data,
         TimeInterval    interval
                     );

/* The variable argument list should be the following: inpars, outpars, returned_value          */
/*   where:                                                                                     */
/*     inpars         - a list of input values of the specification function arguments          */
/*     outpars        - a list of output values of the specification function arguments         */
/*     returned_value - a value returned by specification function if returned type is not void */
void registerStimulusWithTimeInterval(
         ChannelID       chid,
         const char*     name,
         SpecificationID stimulusID,
         TimeInterval    interval,
         ...
                                     );


#ifdef __cplusplus
}
#endif

#endif
