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
 * FILE:        serialization.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    8 Jul 2003
 * DESCRIPTION: Implementation of functions that
 *              needs for serialization within
 *              linear time model.
 */

#include <ts/scenario.h>
#include <ts/serialization.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>
#include <stdlib.h>


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
static SerializationManager linearSerializationManager;


/********************************************************************/
/**                          Serialization                         **/
/********************************************************************/
struct Serialization
{
  SerializationManager* manager;
  Interactions*  interactions;
  ChoisePoint*   first;
  ChoisePoint*   last;
  ChoisePoint*   startChoisePoint;
  ChoisePoint*   startReturnedPoint;
  ChoisePoint*   curChoisePoint;
  LinearTimeMark minMaxMark;         // Cached value of min{ maxMark(interactions) }
  LinearTimeMark minSuffixMark;      // Cached value of min{ maxMark(interactions from the rest) }
  bool           cacheIsBehind;      // is true if minSuffixMark is less than its real value
};

static void normalizeInteractions(Interactions* interactions);
static LinearTimeMark calculateMinMaxMark(Interactions* interactions);

Serialization* createLinearSerialization(Interactions* interactions)
{
Serialization* res;

  assertion(interactions->is_correct,"createSerialization: interactions is not correct");

  res = (Serialization*)malloc(sizeof(Serialization));
  assertion( res != NULL, "createSerialization()#not enough memory" );

  normalizeInteractions(interactions);
  res->manager            = &linearSerializationManager;
  res->interactions       = interactions;
  res->first              = NULL;
  res->last               = NULL;
  res->startChoisePoint   = NULL;
  res->startReturnedPoint = NULL;
  res->curChoisePoint     = NULL;
  res->minMaxMark         = calculateMinMaxMark(interactions);
  res->minSuffixMark      = res->minMaxMark;
  res->cacheIsBehind      = false;

  return res;
}

/////////////////////
// Internal functions
static LinearTimeMark calculateMinMaxMark(Interactions* interactions);
static void normalizeInteractions(Interactions* interactions);
static void normalizeChannel(InteractionChannel* channel);
static void updateCache(Serialization* serialization);
static void appendChoisePoint(Serialization* serialization,ChoisePoint* point);
static Interaction* getNextFreeInteraction(Serialization* serialization,InteractionChannel* channel);
static bool isConformingChoice(Serialization* serialization,Interaction* interaction);
static Interaction* getNextFreeConformingInteraction(Serialization* serialization,InteractionChannel* channel);
static void onAdditionInteraction(Serialization* serialization,Interaction* interaction);
static void onRemoveInteraction(Serialization* serialization,Interaction* interaction);
void restartLinearSerialization(Serialization* serialization);

// calculates min{ maxMark(interactions) }
static LinearTimeMark calculateMinMaxMark(Interactions* interactions)
{
InteractionChannel* tmp;
LinearTimeMark res;

  res = maxTimeMark.timemark;
  // We use that min{maxMark(channel)} = maxMark(first)
  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
    if (tmp->first->interval.maxMark.timemark < res)
      res = tmp->first->interval.maxMark.timemark;
  return res;
}

// normalizes interactions
static void normalizeInteractions(Interactions* interactions)
{
InteractionChannel* tmp;

  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
    normalizeChannel(tmp);
}

// normalizes interaction channel: minMark = max{minMark(prefix)},maxMark = min{maxMark(suffix)}
static void normalizeChannel(InteractionChannel* channel)
{
Interaction* tmp;

  if (channel->first == channel->last)
    return;
  for(tmp=channel->first;tmp->nextPtr!=NULL;tmp=tmp->nextPtr)
    if (tmp->nextPtr->interval.minMark.timemark < tmp->interval.minMark.timemark)
      tmp->nextPtr->interval.minMark.timemark = tmp->interval.minMark.timemark;
  for(tmp=channel->last;tmp->prevPtr!=NULL;tmp=tmp->prevPtr)
    if (tmp->prevPtr->interval.maxMark.timemark > tmp->interval.maxMark.timemark)
      tmp->prevPtr->interval.maxMark.timemark = tmp->interval.maxMark.timemark;
}

// recalculates cached values
static void updateCache(Serialization* serialization)
{
InteractionChannel* tmp;
Interaction* tmp2;

  serialization->minSuffixMark = maxTimeMark.timemark;
  for(tmp=serialization->interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
   {tmp2 = getNextFreeInteraction(serialization,tmp);
    if (tmp2 != NULL)
      if (tmp2->interval.maxMark.timemark < serialization->minSuffixMark)
        serialization->minSuffixMark = tmp2->interval.maxMark.timemark;
   }
  serialization->cacheIsBehind = false;
}

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
  if (interaction->interval.minMark.timemark <= serialization->minSuffixMark)
    return true;
  if (!serialization->cacheIsBehind)
    return false;
  updateCache(serialization);
  // After update serialization->minSuffixMark is min{maxMark(postfix)},
  // but min{maxMark(postfix)} <= min{maxMark(postfix\interaction)}, that is needed really here
  return (interaction->interval.minMark.timemark <= serialization->minSuffixMark);
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
  if (interaction->interval.maxMark.timemark == serialization->minSuffixMark)
    serialization->cacheIsBehind = true;
}

// handles 'remove interaction from the current series' event
static void onRemoveInteraction(Serialization* serialization,Interaction* interaction)
{
  if (interaction->interval.maxMark.timemark < serialization->minSuffixMark)
    serialization->minSuffixMark = interaction->interval.maxMark.timemark;
  if (interaction->interval.maxMark.timemark == serialization->minSuffixMark)
    serialization->cacheIsBehind = false;
}

//////////////////////
// Interface functions

// fixes the interaction as the first one in all series
// if ignore is true this interaction will not be returned during enumeration series
bool fixPrefixLinear(Serialization* serialization,Interaction* interaction,bool ignore)
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
bool nextSeriesLinear(Serialization* serialization)
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
        restartLinearSerialization(serialization);
        return true;
       }
     }
    // Destroy the last choice point
    destroyChoisePoint(coffin);
   }

  return false;
}

// restarts the enumeration of the current series
void restartLinearSerialization(Serialization* serialization)
{
  serialization->curChoisePoint = serialization->startReturnedPoint;
  serialization->minSuffixMark = serialization->minMaxMark;
  serialization->cacheIsBehind = true;
}

// returns the next interaction of the current series
Interaction* getNextInteractionLinear(Serialization* serialization)
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
void destroyLinearSerialization(Serialization* serialization)
{
ChoisePoint *tmp,*coffin;

  if (serialization == NULL)
    return;

  for(tmp=serialization->first;tmp!=NULL;)
   {coffin = tmp;
    tmp = tmp->nextPtr;
    destroyChoisePoint(coffin);
   }
  free(serialization);
}


/********************************************************************/
/**                      Serialization Manager                     **/
/********************************************************************/
static SerializationManager linearSerializationManager =
 {
  createLinearSerialization,
  fixPrefixLinear,
  nextSeriesLinear,
  restartLinearSerialization,
  getNextInteractionLinear,
  destroyLinearSerialization
 };


/********************************************************************/
/**                           Hyper Oracle                         **/
/********************************************************************/
static
bool shouldFindFirstSeriesOnly(Interactions* interactions)
{
int bound = getFindFirstSeriesOnlyBound();

  if (bound > 0)
    return (interactions->numInteractions >= bound);
  else
    return false;
}

static
OracleVerdict call_pure_oracle(Interaction* interaction)
{
CallContextType call_context;

  call_context.params = clone_Object(r(interaction->data));
  call_context.time   = interaction->interval;
  call_context.refId  = interaction->id;
  return interaction->specificationDesc->call_pure_oracle( call_context );
}

static
bool serialize(Serialization* serialization,ModelStateManager* model_state)
{
Interaction*        interaction;
Object*             startModelState;
Object*             finalModelState;
OracleVerdict       lastVerdict;
bool                serializationVerdict;
bool                findFirstSeriesOnly;

  // Save start model state
  startModelState = model_state_save(model_state);
  finalModelState = NULL;

  // Start serializaton
  findFirstSeriesOnly = shouldFindFirstSeriesOnly(serialization->interactions);
  serializationVerdict = false;
  for(;;)
   {// Cycle by serializations
    traceSeriesStart();
    for(;;)
     {// Cycle by interactions inside one serialization
      interaction = getNextInteraction(serialization);
      if (interaction == NULL)
        break;
      // call oracle of the current interaction
      lastVerdict = call_pure_oracle( interaction );
      if (lastVerdict != OracleVerdict_Ok)
       {if (lastVerdict == OracleVerdict_PreconditionFailed)
         {// Precondition was failed
          if (interaction->is_stimulus)
            assertion( false, "Test is incorrect: stimulus precondition failed" );
         }
        traceSeriesEnd();
        break;
       }
     }
    if (interaction == NULL)
     {if (model_state_is_stationary(model_state))
       {// Serialization was successfully
	    if (!serializationVerdict)
         {serializationVerdict = true;
          traceSeriesEnd();
          if (findFirstSeriesOnly)
            break;
          finalModelState = model_state_save( model_state );
         }
        else
         {
          traceSeriesEnd();
          if (!equals(r(finalModelState),model_state_save(model_state)))
		   {
            String* message;

            traceExceptionInfo("Non-deterministic serialization. Two successful serializations have been finished at different model states");

            message = create_String("The last serialization has been finished at ");
            message = concat_String( message, toString(model_state_save(model_state)) );
            traceExceptionInfo(toCharArray_String(r(message)));
            destroy(message);

            message = create_String("The previous successful serialization has been finished at ");
            message = concat_String( message, toString(r(finalModelState)) );
            traceExceptionInfo(toCharArray_String(r(message)));
            destroy(message);

            traceException( SERIALIZATION_FAILED_MESSAGE );

            // Unsuccessful end of serialization
            serializationVerdict = false;
            break;
		   }
         }
       }
      else
       {
    	  traceInterimException( NONSTATIONARY_STATE_MESSAGE );
          traceSeriesEnd();
       }
     }
    if (!nextSeries(serialization))
     {if (!serializationVerdict)
         traceException( SERIALIZATION_FAILED_MESSAGE );
      break;
     }
    // Prepare for the next series
    model_state_restore(model_state, clone_Object(r(startModelState)) );
   }

  // Restore the correct final model state
  if (serializationVerdict && !isFindFirstSeriesOnly())
    model_state_restore(model_state, r(finalModelState) );

  // Finish serialization
  destroySerialization( serialization );
  destroy(startModelState);
  destroy(finalModelState);

  if (!serializationVerdict)
    testVerdict = TD_bad_verdict;
  return serializationVerdict;
}

static
bool stimuluses_are_sequential(Interactions* interactions)
{
Interaction*        stimulus;
Interaction*        interaction;
InteractionChannel* channel;

  stimulus = NULL;
  for(channel=interactions->first;channel!=NULL;channel=channel->nextPtr)
   {for(interaction=channel->first;interaction!=NULL;interaction=interaction->nextPtr)
     {if (interaction->is_stimulus)
       {if ( (interaction != channel->first) || (stimulus != NULL) )
          return false;
        stimulus = interaction;
        break;
       }
     }
   }
  return (stimulus != NULL);
}

static
bool serialize_reactions_on_the_sequential_stimulus(Interactions* interactions,ModelStateManager* model_state)
{
Interaction*        stimulus;
Interaction*        interaction;
InteractionChannel* channel;
Serialization*      serialization;
OracleVerdict       lastVerdict;

  // Precondition: interactions contains the only channel containing stimulus
  stimulus = NULL;
  for(channel=interactions->first;channel!=NULL;channel=channel->nextPtr)
   {for(interaction=channel->first;interaction!=NULL;interaction=interaction->nextPtr)
     {if (interaction->is_stimulus)
       {assertion( interaction == channel->first,
                   "serialize_reactions_on_the_sequential_stimulus: the stimulus channel contains reactions"
                 );
        assertion( stimulus == NULL,
                   "serialize_reactions_on_the_sequential_stimulus: more than 1 channel with stimulus"
                 );
        stimulus = interaction;
        break;
       }
     }
   }
  assertion( stimulus != NULL, "serialize_reactions_on_the_sequential_stimulus: stimulus isn't registered" );

  // Process the first stimulus
  lastVerdict = call_pure_oracle(stimulus);
  assertion( lastVerdict != OracleVerdict_PreconditionFailed,
            "Test is incorrect: stimulus precondition failed"
           );
  if (lastVerdict == OracleVerdict_PostconditionFailed)
   {// trace oracle failure
    traceSeriesStart();
    traceSeriesEnd();
//    traceException( SERIALIZATION_FAILED_MESSAGE );
    // set bad verdict
    testVerdict = TD_bad_verdict;
    return false;
   }

  // Start serializaton
  serialization = createSerialization( interactions );
  if (!fixPrefix(serialization,stimulus,true))
    assertion(false,"The first stimulus cannot be the first interaction in series");

  return serialize( serialization, model_state );
}

bool serialize_interactions( Interactions* interactions, ModelStateManager* model_state )
{
  if (interactions->numInteractions == 0)
    return true;
  if (stimuluses_are_sequential(interactions))
    return serialize_reactions_on_the_sequential_stimulus( interactions, model_state );
  return serialize( createSerialization( interactions ), model_state );
}


