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
 * FILE:        register.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    13 Apr 2003
 * DESCRIPTION: Registrar of interactions with
 *              system under test.
 */

#include <string.h>
#include <stdlib.h>
#include <ts/ts.h>
#include <ts/register_tsi.h>
#include <ts/serialization.h>
#include <ts/system.h>
#include <atl/integer.h>
#include <atl/map.h>


/********************************************************************/
/**                            ChannelID                           **/
/********************************************************************/
// Copy of constants from timemark.c because of C programing language
#define WRONG_CHANNEL_ID   -2
#define UNIQUE_CHANNEL_ID  -1


/********************************************************************/
/**                        Auxiliary function                      **/
/********************************************************************/
static char* clone_string(const char* str)
{
int len;
char* res;

  if (str == NULL)
    return NULL;
  len = strlen(str);
  res = (char*)malloc(sizeof(char)*(len+1));
  assertion( res != NULL, "clone_string()#not enough memory" );

  memcpy(res,str,len+1);

  return res;
}

static char* clone_string_with_default(const char* str,const char* default_str)
{
  if (str != NULL)
    return clone_string( str );
  return clone_string( default_str );
}


/********************************************************************/
/**                          Interaction                           **/
/********************************************************************/
Interaction* createInteraction(
                 InteractionId   id,
                 bool            is_native,
                 const char*     name,
                 SpecificationID specID,
                 Object*         data,
                 TimeInterval    interval
                              )
{
Interaction* res;

  res = (Interaction*)malloc(sizeof(Interaction));
  assertion( res != NULL, "createInteraction()#not enough memory" );

  res->id                = id;
  res->is_stimulus       = !specID->is_reaction;
  res->is_native         = is_native;
  res->name              = clone_string_with_default( name, "<UnnamedInteraction>" );
  res->specificationDesc = specID;
  res->data              = (data!=NULL) ? clone_Object(data) : NULL;
  res->interval          = (getTSTimeModel() != NotUseTSTime) ? interval : overallTimeInterval;
  res->nextPtr           = NULL;
  res->prevPtr           = NULL;
  
  return res;
}

void destroyInteraction(Interaction* interaction)
{
  if (interaction == NULL)
    return;
  if (interaction->name != NULL)
    free(interaction->name);
  destroy(interaction->data);
  free(interaction);
}


/********************************************************************/
/**                       InteractionChannel                       **/
/********************************************************************/
InteractionChannel* createInteractionChannel(ChannelID chid)
{
InteractionChannel* res;

  res = (InteractionChannel*)malloc(sizeof(InteractionChannel));
  assertion( res != NULL, "createInteractionChannel()#not enough memory" );
  
  res->chid        = chid;
  res->first       = NULL;
  res->last        = NULL;
  res->maxMinMark  = minTimeMark.timemark;
  res->nextPtr     = NULL;
  
  return res;
}

void updateChannelsCache(Interactions* interactions,InteractionChannel* channel,Interaction* interaction)
{
  switch (getTSTimeModel())
   {case NotUseTSTime:
      break;
    case LinearTSTime:
      if (interaction->interval.minMark.timemark > channel->maxMinMark)
        channel->maxMinMark = interaction->interval.minMark.timemark;
      break;
    case DistributedTSTime:
      break;
    default:
      assertion( false, "updateChannelsCache#default" );
   }
}

static
int printChannel(char* buffer,ChannelID channel)
{
int res;

  if (channel == UniqueChannel)
    res = sprintf(buffer,"UniqueChannel");
  else
    res = sprintf(buffer,"%d",channel);
  return res;
}

void checkConsistencyOfAppend(Interactions* interactions,InteractionChannel* channel,Interaction* interaction)
{
  switch (getTSTimeModel())
   {case NotUseTSTime:
      break;
    case LinearTSTime:
      if (interaction->interval.maxMark.timemark < channel->maxMinMark)
       {int i;
        Interaction* other;
        const char* iname;
        const char* oname;
        
        
        // Find an interaction within the channel with 'channel->maxMinMark'
        for( other = channel->last; other != NULL; other = other->prevPtr )
         {
          if (other->interval.minMark.timemark == channel->maxMinMark)
            break;
         }
        assertion( other!=NULL, "checkConsistencyOfAppend: originator of channel->maxMinMark not found" );

        if (interactions->is_correct)
         {interactions->is_correct = false;
          iname = (interaction->name != NULL) ? interaction->name : interaction->specificationDesc->name;
          oname = (other->name != NULL) ? other->name : other->specificationDesc->name;
          interactions->defect_description = (char*)calloc(350+strlen(iname)+strlen(oname),sizeof(char));
          assertion(interactions->defect_description!=NULL,"checkConsistencyOfAppend()#not enough memory");
          
          i  = sprintf(interactions->defect_description,"The max time mark of the interaction {'%s',",iname);
          i += printLinearTimeInterval( interactions->defect_description+i, interaction->interval );
          i += sprintf(interactions->defect_description+i,"} is less than\n" );
          i += sprintf(interactions->defect_description+i,"the min time mark of the interaction {'%s',",oname);
          i += printLinearTimeInterval( interactions->defect_description+i, other->interval );
          i += sprintf(interactions->defect_description+i,"},\n" "which precedes the first one within the channel " );
          i += printChannel( interactions->defect_description+i, channel->chid );
         }
       }
      break;
    case DistributedTSTime:
      break;
    default:
      assertion( false, "checkConsistencyOfAppend#default" );
   }
}

void appendInteractionToChannel(Interactions* interactions,InteractionChannel* channel,Interaction* interaction)
{
  interaction->prevPtr = channel->last;
  if (channel->first == NULL)
   {channel->first = interaction;
    channel->last  = interaction;
   }
  else
   {channel->last->nextPtr = interaction;
    channel->last = interaction;
   }
  // update channels cache
  updateChannelsCache( interactions, channel, interaction );
  // check consistency
  checkConsistencyOfAppend( interactions, channel, interaction );
}

void destroyInteractionChannel(InteractionChannel* channel)
{
Interaction *tmp,*coffin;

  if (channel == NULL)
    return;
  for(tmp=channel->first;tmp!=NULL;)
   {coffin = tmp;
    tmp = tmp->nextPtr;
    destroyInteraction(coffin);
   }
  free(channel);
}


/********************************************************************/
/**                        InteractionList                         **/
/********************************************************************/
Interactions* createInteractions(void)
{
Interactions* res;

  res = (Interactions*)malloc(sizeof(Interactions));
  assertion( res != NULL, "createInteractions()#not enough memory" );
  
  res->first              = NULL;
  res->last               = NULL;
  res->numInteractions    = 0L;
  res->is_correct         = true;
  res->defect_description = NULL;
  
  return res;
}

InteractionChannel* getChannelByID(Interactions* interactions,ChannelID chid)
{
InteractionChannel* tmp;

  assertion( chid >= 0, "pre getChannelByID: chid should be user id" );

  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
    if (tmp->chid == chid)
      return tmp;
  return NULL;
}

void appendInteraction(
         Interactions*   interactions,
         ChannelID       chid,
         bool            is_native,
         const char*     name,
         SpecificationID specID,
         Object*         data,
         TimeInterval    interval
                      )
{
InteractionChannel* channel;
Interaction* interaction;

  assertion( chid >= -1, "pre appendInteraction: chid should be correct" );

  channel = NULL;
  if (chid != UniqueChannel)
    channel = getChannelByID(interactions,chid);
  if (channel == NULL)
   {channel = createInteractionChannel(chid);
    if (interactions->first == NULL)
      interactions->first = channel;
    else
      interactions->last->nextPtr = channel;
    interactions->last  = channel;
   }
  // create interaction
  interaction = createInteraction( interactions->numInteractions, is_native, name, specID, data, interval );
  if (is_native)
    traceModelOperationIdentifier(interaction->id);
  // physical appending of interaction
  appendInteractionToChannel( interactions, channel, interaction );
  interactions->numInteractions++;
}

ChannelID getChannelIDOfInteraction(Interactions* list,Interaction* interaction)
{
InteractionChannel* tmp;
Interaction* tmp2;

  for(tmp=list->first;tmp!=NULL;tmp=tmp->nextPtr)
    for(tmp2=tmp->first;tmp2!=NULL;tmp2=tmp2->nextPtr)
      if (tmp2 == interaction)
        return tmp->chid;
  return WrongChannel;
}

InteractionChannel* getChannelOfInteraction(Interactions* list,Interaction* interaction)
{
InteractionChannel* tmp;
Interaction* tmp2;

  for(tmp=list->first;tmp!=NULL;tmp=tmp->nextPtr)
    for(tmp2=tmp->first;tmp2!=NULL;tmp2=tmp2->nextPtr)
      if (tmp2 == interaction)
        return tmp;
  return NULL;
}

void destroyInteractions(Interactions* list)
{
InteractionChannel *tmp,*coffin;

  if (list == NULL)
    return;
  for(tmp=list->first;tmp!=NULL;)
   {coffin = tmp;
    tmp = tmp->nextPtr;
    destroyInteractionChannel(coffin);
   }
  if (list->defect_description != NULL)
    free(list->defect_description);
  free(list);
}


/********************************************************************/
/**                      Interactions Register                     **/
/********************************************************************/
Interactions* interactions = NULL;
TSSemaphore interactions_semaphore = NULL;

static
bool lock_interactions_register(void)
{
  if (interactions_semaphore == NULL)
    interactions_semaphore = ts_create_semaphore();
  return ts_close_semaphore( interactions_semaphore );
}

static
bool unlock_interactions_register(void)
{
  return ts_open_semaphore( interactions_semaphore );
}

static
bool isValidInteraction( 
         SpecificationID specID,
         Object*         data,
         TimeInterval    interval
                       )
{
  if (getTSTimeModel() != NotUseTSTime)
   {
    if (greaterTimeMarks(interval.minMark,interval.maxMark))
     {
      int i;
      char* defect_description = malloc( 256 + strlen(specID->name) );
      assertion( defect_description!=NULL, "isValidInteraction: not enough memory" );

      i  = sprintf( defect_description,   "The min time mark is greater than the max time mark " );
      i += sprintf( defect_description+i, "of the interaction {'%s',",specID->name);
      i += printLinearTimeInterval( defect_description+i, interval );
      i += sprintf( defect_description+i, "}" );

      registerWrongReaction( defect_description );
      free( defect_description );

      return false;
     }
   }
  // TODO: Consider possibility to check typeof(data) against expected type of specID
  return true;
}

static
void registerInteraction( 
         ChannelID       chid,
         bool            is_native,
         const char*     name,
         SpecificationID specID,
         Object*         data,
         TimeInterval    interval
                        )
{
  if (isValidInteraction( specID, data, interval ))
   {
    lock_interactions_register();
  
    if (interactions == NULL)
      interactions = createInteractions();
    appendInteraction( interactions, chid, is_native, name, specID, data, interval );

    unlock_interactions_register();
   }
}


/********************************************************************/
/**                        Register Stimulus                       **/
/********************************************************************/
static ChannelID stimulusChannelID = UNIQUE_CHANNEL_ID;

ChannelID setStimulusChannel(ChannelID chid)
{
ChannelID prevStimulusChannelID;
  
  prevStimulusChannelID = stimulusChannelID;
  stimulusChannelID = chid;
  return prevStimulusChannelID;
}

ChannelID getStimulusChannel(void)
{
  return stimulusChannelID;
}

void registerStimulus(
         const char*     name,
         SpecificationID stimulusID,
         Object*         data,
         TimeInterval    interval
                     )
{
  if (name == NULL)
    name = stimulusID->name;
  registerInteraction( stimulusChannelID, true, name, stimulusID, data, interval );
}

void registerStimulusWithTimeInterval(
         ChannelID       chid,
         const char*     name,
         SpecificationID stimulusID,
         TimeInterval    interval,
         ...
                                     )
{
va_list arg_list;
Object* data;

  assertion(!stimulusID->is_reaction,"Reaction should be registered with registerReaction functions");

  if (name == NULL)
    name = stimulusID->name;

  va_start( arg_list, interval );
  data = va_create( stimulusID->par_res_type, &arg_list );
  va_end( arg_list );

  registerInteraction( chid, false, name, stimulusID, data, interval );
}


/********************************************************************/
/**                        Register Reaction                       **/
/********************************************************************/
Object* create_reaction_par_res_type(const Type* type_desc,Object* data)
{
  return create( type_desc, data );
}

void registerReaction(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data
                     )
{
  registerReactionWithTimeInterval(chid,name,reactionID,data,createTimeInterval(minTimeMark,maxTimeMark));
}

void registerReactionWithTimeMark(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data,
         TimeMark        mark
                                 )
{
  registerReactionWithTimeInterval(chid,name,reactionID,data,createTimeInterval(mark,mark));
}

void registerReactionWithTimeInterval(
         ChannelID       chid,
         const char*     name,
         SpecificationID reactionID,
         Object*         data,
         TimeInterval    interval
                                     )
{
  assertion(reactionID->is_reaction,"Reaction should be marked by keyword 'reaction'");

  if (name == NULL)
    name = reactionID->name;

  registerInteraction( chid, false, name, reactionID, 
                       create_reaction_par_res_type( reactionID->par_res_type, data ),
                       interval
                     );
}


/********************************************************************/
/**                     Register Wrong Reaction                    **/
/********************************************************************/
void registerWrongReaction(const char* info)
{
char* buff;

  lock_interactions_register();

  if (interactions == NULL)
    interactions = createInteractions();
  if (interactions->is_correct)
   {// Put wrong reaction description
    interactions->is_correct = false;
    interactions->defect_description = clone_string_with_default( info, "Wrong reaction has been catched" );
   }
  else if (info != NULL)
   {// Append wrong reaction description to old ones
    buff = (char*)calloc(strlen(interactions->defect_description)+strlen(info)+16,sizeof(char));
    assertion( buff!=NULL, "registerWrongReaction#not enough memory" );
    sprintf(buff,"%s\n-----\n%s",interactions->defect_description,info);
    free(interactions->defect_description);
    interactions->defect_description = buff;
   }

  unlock_interactions_register();
}


/********************************************************************/
/**               Check Consistency of Interactions                **/
/********************************************************************/
static
void checkConsistencyDistributedInteractions(Interactions* interactions)
{
InteractionChannel *tmp,*tmp2;
Interaction *interaction,*interaction2;
char* dependencies;
long numInteractions;
int i,j,residual;
bool flag;
size_t size;

  if (!interactions->is_correct)
    return;
  numInteractions = interactions->numInteractions;
  size = numInteractions * numInteractions;
  dependencies = (char*)calloc(size,sizeof(char));
  assertion( dependencies != NULL, "checkConsistencyDistributedInteractions()#not enough memory" );
  memset(dependencies, '\0', size);

  // init graph of dependencies
  for(tmp=interactions->first;tmp!=NULL;tmp=tmp->nextPtr)
   {for(interaction=tmp->first;interaction!=NULL;interaction=interaction->nextPtr)
     {for(interaction2=interaction->nextPtr;interaction2!=NULL;interaction2=interaction2->nextPtr)
        dependencies[numInteractions*interaction->id+interaction2->id] = '\1';
     }
    for(tmp2=tmp->nextPtr;tmp2!=NULL;tmp2=tmp2->nextPtr)
     {for(interaction=tmp->first;interaction!=NULL;interaction=interaction->nextPtr)
       {for(interaction2=tmp2->first;interaction2!=NULL;interaction2=interaction2->nextPtr)
         {if (lessTimeMarks(interaction->interval.maxMark,interaction2->interval.minMark))
            dependencies[numInteractions*interaction->id+interaction2->id] = '\1';
          else if (greaterTimeMarks(interaction->interval.minMark,interaction2->interval.maxMark))
            dependencies[numInteractions*interaction2->id+interaction->id] = '\1';
         }
       }
     }
   }
  
  // search of cycles
  flag = true;
  residual = numInteractions;
  for(;flag;)
   {flag = false;
    for(i=0;i<numInteractions;i++)
     {if (dependencies[(numInteractions+1)*i] == '\0')
       {for(j=0;j<numInteractions;j++)
         {if (dependencies[(numInteractions+1)*j] == '\0')
           {if (dependencies[numInteractions*j+i] != '\0')
              break;
           }
         }
        if (j==numInteractions)
         {dependencies[(numInteractions+1)*i] = '\1';
          residual--;
          if (residual == 0)
            break;
          flag = true;
         }
       }
     }
   }
  // set defect description
  if (residual != 0)
   {interactions->is_correct = false;
    flag = true;
    for(tmp=interactions->first;(tmp!=NULL) && flag;tmp=tmp->nextPtr)
     {for(interaction=tmp->first;interaction!=NULL;interaction=interaction->nextPtr)
       {if (dependencies[(numInteractions+1)*interaction->id] == '\0')
         {const char* iname;
          iname = (interaction->name != NULL) ? interaction->name : interaction->specificationDesc->name;
          interactions->defect_description = (char*)calloc(256+strlen(iname),sizeof(char));
          assertion( interactions->defect_description!=NULL, "checkConsistencyInteractions#not enough memory" );
          strcpy(interactions->defect_description,"Cyclic dependencies between interactions.\n");
          strcat(interactions->defect_description,"One of members of that cycle is the following:\n");
          strcat(interactions->defect_description,"Channel=");
          i = strlen(interactions->defect_description);
          i += printChannel( interactions->defect_description+i, tmp->chid );
          i += sprintf(interactions->defect_description+i,"\nInteraction={'%s',",iname);
          i += printDistributedTimeInterval( interactions->defect_description+i, interaction->interval );
          i += sprintf(interactions->defect_description+i,"}\n");
          flag = false;
          break;
         }
       }
     }
   }

  free(dependencies);
}

static                                                                              
void checkConsistencyInteractions(Interactions* interactions)
{
  switch (getTSTimeModel())
   {case NotUseTSTime:
    case LinearTSTime:
      // method of interactions assembling guarantees consistency
      break;
    case DistributedTSTime:
      checkConsistencyDistributedInteractions(interactions);
      break;
    default:
      assertion( false, "checkConsistencyInteractions#default" );
   }
}


/********************************************************************/
/**                        Get Gathered Data                       **/
/********************************************************************/
Interactions* getGatheredInteractions()
{
Interactions* res;

  gatherReactions();
  
  lock_interactions_register();
  res = interactions;
  interactions = NULL;
  unlock_interactions_register();

  if (res == NULL)
    return createInteractions();
  checkConsistencyInteractions(res);
  return res;
}

// Clear the internal store.
void clearGatheredInteractions(void)
{
Interactions* coffin;

  lock_interactions_register();
  coffin = interactions;
  interactions = NULL;
  unlock_interactions_register();

  destroyInteractions(coffin);
}


/********************************************************************/
/**                         Catcher Cell                           **/
/********************************************************************/
typedef struct CatcherCell CatcherCell;

struct CatcherCell
{
  ReactionCatcherFuncType catcher;
  void*                   parameter;
  CatcherCell*            nextPtr;
  CatcherCell*            prevPtr;
};

CatcherCell* createCatcherCell(ReactionCatcherFuncType catcher,void* parameter)
{
CatcherCell* res;

  res = (CatcherCell*)malloc(sizeof(CatcherCell));
  assertion( res != NULL, "createCatcherCell()#not enough memory" );
  
  res->catcher   = catcher;
  res->parameter = parameter;
  res->nextPtr   = NULL;
  res->prevPtr   = NULL;
  
  return res;
}

void destroyCatcherCell(CatcherCell* cell)
{
  if (cell == NULL)
    return;
  free(cell);
}


/********************************************************************/
/**                       Catchers Register                        **/
/********************************************************************/
static 
struct CatcherRegister
{
  CatcherCell* first;
  CatcherCell* last;
}
registeredCatchers = { NULL, NULL };


/********************************************************************/
/**                            Catchers                            **/
/********************************************************************/
void registerReactionCatcher(ReactionCatcherFuncType catcher,void* par)
{
CatcherCell* cell;

  assertion( catcher!=NULL, "registerReactionCatcher: catcher should be nonnull" );

  cell = createCatcherCell(catcher,par);
  cell->prevPtr = registeredCatchers.last;
  if (registeredCatchers.first == NULL)
    registeredCatchers.first = cell;
  else
    registeredCatchers.last->nextPtr = cell;
  registeredCatchers.last = cell;
}

bool unregisterReactionCatcher(ReactionCatcherFuncType catcher,void* par)
{
CatcherCell* tmp;

  for(tmp=registeredCatchers.first;tmp!=NULL;tmp=tmp->nextPtr)
    if ((tmp->catcher == catcher) && (tmp->parameter == par))
     {if (tmp->prevPtr == NULL)
        registeredCatchers.first = tmp->nextPtr;
      else
        tmp->prevPtr->nextPtr = tmp->nextPtr;
      if (tmp->nextPtr == NULL)
        registeredCatchers.last = tmp->prevPtr;
      else
        tmp->nextPtr->prevPtr = tmp->prevPtr;
      destroyCatcherCell(tmp);
      return true;
     }
  return false;
}

bool unregisterReactionCatchers(ReactionCatcherFuncType catcher)
{
CatcherCell *tmp,*coffin;
bool verdict = false;

  for(tmp=registeredCatchers.first;tmp!=NULL;)
   {if (tmp->catcher == catcher)
     {if (tmp->prevPtr == NULL)
        registeredCatchers.first = tmp->nextPtr;
      else
        tmp->prevPtr->nextPtr = tmp->nextPtr;
      if (tmp->nextPtr == NULL)
        registeredCatchers.last = tmp->prevPtr;
      else
        tmp->nextPtr->prevPtr = tmp->prevPtr;
      coffin = tmp;
      tmp = tmp->nextPtr;
      destroyCatcherCell(coffin);
      verdict = true;
     }
    else
     {
      tmp = tmp->nextPtr;
     }
   }
  return verdict;
}

bool gatherReactions()
{
CatcherCell *tmp,*next;

  for(tmp=registeredCatchers.first;tmp!=NULL;)
   {
    next = tmp->nextPtr;
    if (!tmp->catcher(tmp->parameter))// bool returned value replace on registerIncorrectReaction
      assertion( false, "A catcher returns bad verdict" );
    tmp = next;
   }
  return true;
}


/********************************************************************/
/**                    Trace Alien Interactions                    **/
/********************************************************************/
void traceAlienInteractions(Interactions* interactions)
{
InteractionChannel* channel;
Interaction*        interaction;

  for(channel=interactions->first;channel!=NULL;channel=channel->nextPtr)
   {for(interaction=channel->first;interaction!=NULL;interaction=interaction->nextPtr)
     {if (!interaction->is_native)
       {interaction->specificationDesc->trace_model_operation( interaction->id, 
                                                               r(interaction->data),
                                                               channel->chid,
                                                               interaction->interval
                                                             );
       }
     }
   }
}


/********************************************************************/
/**                          Serialization                         **/
/********************************************************************/
struct Serialization
{
  SerializationManager* manager;
};


Serialization* createSerialization(Interactions* interactions)
{
  switch (getTSTimeModel())
   {case NotUseTSTime:
      return createLinearSerialization(interactions);
    case LinearTSTime:
      return createLinearSerialization(interactions);
    case DistributedTSTime:
      return createDistributedSerialization(interactions);
    default:
      assertion( false, "createSerialization#default" );
      return NULL;
   }
}

bool fixPrefix(Serialization* serialization,Interaction* interaction,bool ignore)
{
  return serialization->manager->fixPrefix(serialization,interaction,ignore);
}

bool nextSeries(Serialization* serialization)
{
  return serialization->manager->nextSeries(serialization);
}

void restartSerialization(Serialization* serialization)
{
  serialization->manager->restartSerialization(serialization);
}

Interaction* getNextInteraction(Serialization* serialization)
{
  return serialization->manager->getNextInteraction(serialization);
}

void destroySerialization(Serialization* serialization)
{
  serialization->manager->destroySerialization(serialization);
}

