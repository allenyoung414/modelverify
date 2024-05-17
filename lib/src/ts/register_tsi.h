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
 * FILE:        register_tsi.h                      
 * AUTHOR:      Alexey V. Khoroshilov               
 * CREATION:    13 Apr 2003                         
 * DESCRIPTION: Registrar of interactions with      
 *              system under test.                  
 * SCOPE:       It is an internal header.           
 */

#ifndef REGISTER_INTERNAL_INTERFACE_H
#define REGISTER_INTERNAL_INTERFACE_H

#include "ts/register.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                          Interaction                           **/
/********************************************************************/
typedef struct Interaction Interaction;

struct Interaction
{
  InteractionId   id;
  bool            is_native;
  bool            is_stimulus;
  char*           name;
  SpecificationID specificationDesc;
  Object*         data;
  TimeInterval    interval;
  Interaction*    nextPtr;
  Interaction*    prevPtr;
};


/********************************************************************/
/**                       InteractionChannel                       **/
/********************************************************************/
typedef struct InteractionChannel InteractionChannel;

struct InteractionChannel
{
  ChannelID           chid;
  Interaction*        first;
  Interaction*        last;
  LinearTimeMark      maxMinMark;  // the maximum of { minMark(interactions) } - linear time model only
  InteractionChannel* nextPtr;
};


/********************************************************************/
/**                        InteractionList                         **/
/********************************************************************/
typedef
struct Interactions
{
  InteractionChannel* first;
  InteractionChannel* last;
  long                numInteractions;
  bool                is_correct;
  char*               defect_description;
} 
Interactions;

ChannelID getChannelIDOfInteraction(Interactions* list,Interaction* interaction);
InteractionChannel* getChannelOfInteraction(Interactions* list,Interaction* interaction);
void destroyInteractions(Interactions* list);


/********************************************************************/
/**                        Get Gathered Data                       **/
/********************************************************************/
// calls all registered catchers and returns the list of registered
// stimulus and reactions. The internal store is cleared.
Interactions* getGatheredInteractions(void);

// Clear the internal store.
void clearGatheredInteractions(void);


/********************************************************************/
/**                         Gather Reactions                       **/
/********************************************************************/
bool gatherReactions(void);


/********************************************************************/
/**                    Trace Alien Interactions                    **/
/********************************************************************/
void traceAlienInteractions(Interactions* interactions);


/********************************************************************/
/**                          Serialization                         **/
/********************************************************************/
typedef struct Serialization Serialization;

Serialization* createSerialization(Interactions* interactions);
bool fixPrefix(Serialization* serialization,Interaction* interaction,bool ignore);
bool nextSeries(Serialization* serialization);
void restartSerialization(Serialization* serialization);
Interaction* getNextInteraction(Serialization* serialization);
void destroySerialization(Serialization* serialization);


/********************************************************************/
/**                     Serialization Manager                      **/
/********************************************************************/
typedef struct SerializationManager SerializationManager;

struct SerializationManager
{
  Serialization* (*createSerialization) (Interactions* interactions);
  bool           (*fixPrefix)(Serialization* serialization,Interaction* interaction,bool ignore);
  bool           (*nextSeries)          (Serialization* serialization);
  void           (*restartSerialization)(Serialization* serialization);
  Interaction*   (*getNextInteraction)  (Serialization* serialization);
  void           (*destroySerialization)(Serialization* serialization);
};


#ifdef __cplusplus
}
#endif

#endif
