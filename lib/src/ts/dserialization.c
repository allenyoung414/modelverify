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
 * FILE:        dserialization.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    8 Jul 2003
 * DESCRIPTION: Implementation of functions that
 *              needs for serialization within
 *              distributed time model.
 */

#include <ts/serialization.h>
#include <utils/assertion.h>
#include <stdlib.h>
#include <memory.h>


/********************************************************************/
/**                          Choise Point                          **/
/********************************************************************/
typedef struct ChoisePoint ChoisePoint;

struct ChoisePoint
{
  Interaction*        interaction;
  InteractionChannel* channel;
  ChoisePoint*        prevPtr;
  ChoisePoint*        nextPtr;
};

static
ChoisePoint* createChoisePoint(Interaction* interaction,InteractionChannel* channel)
{
ChoisePoint* res;

  res = (ChoisePoint*)malloc(sizeof(ChoisePoint));
  assertion( res != NULL, "createChoisePoint()#not enough memory" );
  
  res->interaction = interaction;
  res->channel     = channel;
  res->prevPtr     = NULL;
  res->nextPtr     = NULL;

  return res;
}

static
void destroyChoisePoint(ChoisePoint* point)
{
  if (point == NULL)
    return;
  free(point);
}


/********************************************************************/
/**                     Serialization Manager                      **/
/********************************************************************/
static SerializationManager distributedSerializationManager;


/********************************************************************/
/**                     Interaction Properties                     **/
/********************************************************************/
typedef struct InteractionProperties
{
  Interaction* interaction;
  bool         is_serialized;
}
InteractionProperties;

static
void init_InteractionProperties(InteractionProperties* obj,Interaction* interaction)
{
  obj->interaction   = interaction;
  obj->is_serialized = false;
}


/********************************************************************/
/**                          Serialization                         **/
/********************************************************************/
struct Serialization
{
  SerializationManager*  manager;
  Interactions*          interactions;
  InteractionProperties* properties;   // interaction id -m-> { interaction*, is_serialized }
  char*                  dependencies; // dependencies[i][j]!=0 iff interaction[i]->time < interaction[j]->time
  ChoisePoint*           first;
  ChoisePoint*           last;
  ChoisePoint*           startChoisePoint;
  ChoisePoint*           startReturnedPoint;
  ChoisePoint*           curChoisePoint;
};

static
void setDependencies(Serialization* serialization,InteractionId id1,InteractionId id2,bool value)
{
  serialization->dependencies[serialization->interactions->numInteractions*id1+id2] = value ? '\1' : '\0';
}

static
bool isDependencies(Serialization* serialization,InteractionId id1,InteractionId id2)
{
  return (serialization->dependencies[serialization->interactions->numInteractions*id1+id2] != '\0');
}

Serialization* createDistributedSerialization(Interactions* interactions)
{
Serialization* res;
InteractionChannel *tmp,*tmp2;
Interaction *interaction,*interaction2;
size_t size;

  assertion(interactions->is_correct,"createDistributedSerialization: interactions is not correct");

  res = (Serialization*)malloc(sizeof(Serialization));
  assertion( res != NULL, "createDistributedSerialization()#not enough memory" );
  
  res->manager            = &distributedSerializationManager;
  res->interactions = interactions;

  res->properties = (InteractionProperties*)calloc(interactions->numInteractions,sizeof(InteractionProperties));
  assertion( res->properties != NULL, "createInteractionDependenciesGraph()#not enough memory-2" );
  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
   {for(interaction=tmp->first;interaction!=NULL;interaction=interaction->nextPtr)
      init_InteractionProperties( &(res->properties[interaction->id]), interaction );
   }

  size = interactions->numInteractions * interactions->numInteractions;
  res->dependencies = (char*)calloc(size,sizeof(char));
  assertion( res->dependencies != NULL, "createInteractionDependenciesGraph()#not enough memory-3" );
  memset(res->dependencies, '\0', size);

  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
   {for(tmp2=tmp->nextPtr;tmp2!=NULL;tmp2=tmp2->nextPtr)
     {for(interaction=tmp->first;interaction!=NULL;interaction=interaction->nextPtr)
       {for(interaction2=tmp2->first;interaction2!=NULL;interaction2=interaction2->nextPtr)
         {if (lessTimeMarks(interaction->interval.maxMark,interaction2->interval.minMark))
            setDependencies(res,interaction->id,interaction2->id,true);
          else if (greaterTimeMarks(interaction->interval.minMark,interaction2->interval.maxMark))
            setDependencies(res,interaction2->id,interaction->id,true);
         }
       }
     }
   }

  res->first              = NULL;
  res->last               = NULL;
  res->startChoisePoint   = NULL;
  res->startReturnedPoint = NULL;
  res->curChoisePoint     = NULL;
  
  return res;
}

/////////////////////
// Internal functions
static void appendChoisePoint(Serialization* serialization,ChoisePoint* point);
static Interaction* getNextFreeInteraction(Serialization* serialization,InteractionChannel* channel);
static bool isConformingChoice(Serialization* serialization,Interaction* interaction);
static Interaction* getNextFreeConformingInteraction(Serialization* serialization,InteractionChannel* channel);
static void onAdditionInteraction(Serialization* serialization,Interaction* interaction);
static void onRemoveInteraction(Serialization* serialization,Interaction* interaction);
void restartDistributedSerialization(Serialization* serialization);


// appends the point to the serialization object's list
static void appendChoisePoint(Serialization* serialization,ChoisePoint* point)
{
  if (serialization->first == NULL)
   {serialization->first = point;
    point->prevPtr = NULL;
   }
  else
   {serialization->last->nextPtr = point;
    point->prevPtr = serialization->last;
   }
  serialization->last = point;
}

// returns the next interaction from the channel that does not belong to the serialization yet
static Interaction* getNextFreeInteraction(Serialization* serialization,InteractionChannel* channel)
{
ChoisePoint* point;

  for(point=serialization->last;point!=NULL;point=point->prevPtr)
    if (point->channel == channel)
      return point->interaction->nextPtr;
  return channel->first;
}

// checks that the interaction should not be after some interaction from the rest of the current series
static bool isConformingChoice(Serialization* serialization,Interaction* interaction)
{
InteractionId i,pretender_id;

  pretender_id = interaction->id;
  for(i=serialization->interactions->numInteractions-1;i>0;i--)
    if (    !serialization->properties[i].is_serialized
         && isDependencies(serialization,i,pretender_id)
       )
      return false;
  return true;
}

// returns the next interaction from the channel that does not belong to the serialization yet
//   and conforms to the partial order of interactions
static Interaction* getNextFreeConformingInteraction(Serialization* serialization,InteractionChannel* channel)
{
Interaction* pretender;

  pretender = getNextFreeInteraction(serialization,channel);
  if (pretender == NULL)
    return NULL;
  if (isConformingChoice(serialization,pretender))
    return pretender;
  return NULL;
}

// handles 'addition interaction to the current series' event
static void onAdditionInteraction(Serialization* serialization,Interaction* interaction)
{
  serialization->properties[interaction->id].is_serialized = true;
}

// handles 'remove interaction from the current series' event
static void onRemoveInteraction(Serialization* serialization,Interaction* interaction)
{
  serialization->properties[interaction->id].is_serialized = false;
}

//////////////////////
// Interface functions

// fixes the interaction as the first one in all series
// if ignore is true this interaction will not be returned during enumeration series 
bool fixPrefixDistributed(Serialization* serialization,Interaction* interaction,bool ignore)
{
InteractionChannel* channel;
ChoisePoint* point;

  if (serialization->curChoisePoint == NULL)
   {channel = getChannelOfInteraction(serialization->interactions,interaction);
    if (channel == NULL)
      return false;
    if (interaction != channel->first)
      return false;
    if (!isConformingChoice(serialization,interaction))
      return false;
    point = createChoisePoint( interaction, channel );
    appendChoisePoint( serialization, point );
    serialization->startChoisePoint = point;
    if (ignore)
     {serialization->startReturnedPoint = point;
      serialization->curChoisePoint = serialization->startReturnedPoint;
      onAdditionInteraction(serialization,interaction);
     }
    return true;
   }

  // Temporary restriction: this case is not needed for current implementation
  assertion( false, "fixPrefix should be used with clear serialization" );  
  return false;
}

// switches the serialization object to enumeration the next series
//   all series, which wasn't enumerated and has a prefix equaled
//   the enumerated prefix of the current series, are omitted
bool nextSeriesDistributed(Serialization* serialization)
{
ChoisePoint *tmp,*coffin;
Interaction* interaction;
InteractionChannel* channel;

  if (serialization->curChoisePoint == serialization->startChoisePoint)
    return false;
  
  // Destroy the rest
  for(tmp=serialization->curChoisePoint->nextPtr;tmp!=NULL;)
   {coffin = tmp;
    tmp = tmp->nextPtr;
    destroyChoisePoint(coffin);
   }
  serialization->curChoisePoint->nextPtr = NULL;
  serialization->last = serialization->curChoisePoint;

  // Find next series
  for(;serialization->last!=serialization->startChoisePoint;)
   {// Remove the last choice point
    coffin = serialization->last;
    serialization->last = serialization->last->prevPtr;
    if (serialization->last == NULL)
      serialization->first = NULL;
    else
      serialization->last->nextPtr = NULL;
    serialization->curChoisePoint = serialization->last;
    onRemoveInteraction(serialization,coffin->interaction);
    // Try to find a new choice point
    for(channel=coffin->channel->nextPtr;channel!=NULL;channel=channel->nextPtr)
     {interaction = getNextFreeConformingInteraction(serialization,channel);
      if (interaction != NULL)
       {// Append a new choice point in old memory block
        coffin->interaction = interaction;
        coffin->channel = channel;
        appendChoisePoint(serialization,coffin);
        serialization->curChoisePoint = coffin;
        restartDistributedSerialization(serialization);
        return true;
       }
     }
    // Destroy the last choice point
    destroyChoisePoint(coffin);
   }

  return false;
}

// restarts the enumeration of the current series
void restartDistributedSerialization(Serialization* serialization)
{
  serialization->curChoisePoint = serialization->startReturnedPoint;
}

// returns the next interaction of the current series
Interaction* getNextInteractionDistributed(Serialization* serialization)
{
Interaction*        interaction;
InteractionChannel* tmp;
ChoisePoint* point;

  // Obtain next choise point if it exists
  if (serialization->curChoisePoint == NULL)
    point = serialization->first;
  else
    point = serialization->curChoisePoint->nextPtr;

  // Set it as current choise point and return it
  if (point != NULL)
   {serialization->curChoisePoint = point;
    onAdditionInteraction(serialization,point->interaction);
    return point->interaction;
   }

  // Append new choise point if it is possible
  for(tmp=serialization->interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
   {interaction = getNextFreeConformingInteraction(serialization,tmp);
    if (interaction != NULL)
     {point = createChoisePoint(interaction,tmp);
      appendChoisePoint(serialization,point);
      serialization->curChoisePoint = point;
      onAdditionInteraction(serialization,point->interaction);
      return point->interaction;
     }
   }
  return NULL;
}

// destroys the serialization object
void destroyDistributedSerialization(Serialization* serialization)
{
ChoisePoint *tmp,*coffin;

  if (serialization == NULL)
    return;

  free( serialization->properties );
  free( serialization->dependencies );
  for(tmp=serialization->first;tmp!=NULL;)
   {coffin = tmp;
    tmp = tmp->nextPtr;
    destroyChoisePoint(coffin);
   }
  free(serialization);
}


/********************************************************************/
/**                     Serialization Manager                      **/
/********************************************************************/
static SerializationManager distributedSerializationManager = 
 {
  createDistributedSerialization,
  fixPrefixDistributed,
  nextSeriesDistributed,
  restartDistributedSerialization,
  getNextInteractionDistributed,
  destroyDistributedSerialization
 };
