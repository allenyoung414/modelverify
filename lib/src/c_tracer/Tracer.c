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
 * File: Tracer.c
 * Description:
 *       This file contains an implementation of Tracer.
 */

#define TRACER_DEBUG_LEVEL_NO       0
#define TRACER_DEBUG_LEVEL_DEFAULT  1
#define TRACER_DEBUG_LEVEL_FULL     2


#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "TraceBufferWriter.h"
#include "TraceConsoleWriter.h"
#include "TraceXMLFormatter.h"
#include "TSEnvironment.h"
#include "Tracer.h"

#if defined(PTHREAD) || (defined(WIN32) && defined(_MT))
  #define MULTITHREADED_TRACING
#endif

static TraceWriteController* tracer_getDefaultWriter();
static TraceFormatter* tracer_getDefaultFormatter();
static TraceConstrainer* tracer_getDefaultConstrainer();


int tracer_debug_level = 0;

/*
 * Default tracer
 */
class_Tracer* c_Tracer_default_tracer = NULL;

/*
 * Environment field names
 */
#define TRACER_ENV_HOST              "Host"
#define TRACER_ENV_OS                "Operating System"
#define TRACER_ENV_PRODUCT_NAME      "Product Name"
#define TRACER_ENV_PRODUCT_VERSION   "Product Version"
#define TRACER_ENV_PRODUCT_BUILD     "Product Build"



static TraceBool tracer_ready(class_Tracer* tracer)
{
  if((NULL != tracer) &&
     (NULL != tracer->constrainer) &&
     (NULL != tracer->writer) &&
     (NULL != tracer->formatter) &&
     (NULL != tracer->transitionBuffer) &&
     (NULL != tracer->transitionBuffer_wr) &&
     (NULL != tracer->keepBuffer) &&
     (NULL != tracer->keepBuffer_wr))
    return tr_true;

  return tr_false;
}

/*****************************/

static TraceBool tracer_valid( class_Tracer* tracer )
{
  if( !(tracer_debug_level) ) return tr_true;

  if( tr_true == tracer_ready( tracer ) )
  {
    if( tracer_debug_level >= TRACER_DEBUG_LEVEL_FULL )
    {
      if(    (    NULL == tracer->constrainer->valid
               || (*tracer->constrainer->valid)( tracer->constrainer ) == tr_true
             )
          && (    NULL == tracer->writer->valid
               || (*tracer->writer->valid)( tracer->writer ) == tr_true
             )
          && (    NULL == tracer->formatter->valid
               || (*tracer->formatter->valid)( tracer->formatter ) == tr_true
             )
        )
        return tr_true;
    }
    else
      return tr_true;
  }

  return tr_false;
}

/*****************************/

static void deletelist_tracer_environment( TraceList* list )
{
  TraceListitem *item, *tmp;

  for(item = list->head; item != NULL; ) {
    if(item->data) {
      TracePair_free((TracePair *)item->data);
    }
    tmp = item->next;
    TraceList_remove(list, item);
    item = tmp;
  }
  TraceList_clear(list);
}

/*****************************/

static void deletelist_tracer_transitions (TraceList* list)
{
  TraceList_clear(list);
}

/*****************************/

static void deletelist_tracer_model_operations (TraceList* list)
{
  TraceListitem *item, *tmp;

  for(item = list->head; item != NULL; ) {
    if(item->data) free(item->data);
    tmp = item->next;
    TraceList_remove(list, item);
    item = tmp;
  }
  TraceList_clear(list);
}

/*****************************/

static void deletelist_tracer_exception_values (TraceList* list)
{
  TraceListitem *item, *tmp;

  for(item = list->head; item != NULL; )
  {
    TracePair_free((TracePair *)item->data);
    tmp = item->next;
    TraceList_remove(list, item);
    item = tmp;
  }
  TraceList_clear(list);
}

/*****************************/

static void deletelist_tracer_exception_infos (TraceList* list)
{
  TraceListitem *item, *tmp;

  for(item = list->head; item != NULL; )
  {
    if(item->data) free(item->data);
    tmp = item->next;
    TraceList_remove(list, item);
    item = tmp;
  }
  TraceList_clear(list);
}

/*****************************/

/* Write string to writer */
static void write_string(TraceWriteController* writer, String *str)
{
	(*writer->puts)( writer->state, str );
}

/*****************************/

static void TracerExitTrigger(void)
{
  tracer_setDefaultTracer( NULL );
}



/*
 * Function for setting default server
 */

void tracer_setDefaultTracer(class_Tracer* tracer)
{
  if(NULL != c_Tracer_default_tracer)
	  tracer_deleteTracer(c_Tracer_default_tracer);
  if(NULL != tracer)
	  atexit(TracerExitTrigger);
  c_Tracer_default_tracer = tracer;
}

/*
 * Global tracer counter
 */
static int traceIdSeq = 1;


/*
 * Duplicate list
 */
/*
static TraceBool duplicate_func(TraceList *list, TraceListitem *item, const void *arg1, void *arg2)
{
	TracePair *srcpair = (TracePair *)item->data;
	TracePair *dstpair = TracePair_create_clone( srcpair->first, srcpair->second );
	TraceList *dst = (TraceList *)arg1;
	TraceList_add_back(dst, dstpair);
	return tr_false;
}

static void env_duplicate(TraceList *dst, TraceList *src)
{
	if (src == NULL || dst == NULL)	return;
	TraceList_itr(src, duplicate_func, dst, NULL);
}
*/

/*
 * Function for acquiring default servers
 */
class_Tracer* tracer_createTracer()
{
class_Tracer* newtracer;
TSEnvironment* tsEnv;
TracePair *tmppair;

  newtracer = malloc(sizeof(*newtracer));
  if(NULL != newtracer) {

#ifdef MULTITHREADED_TRACING
	newtracer->semaphore = ts_create_semaphore();
#else
	newtracer->semaphore = NULL;
#endif

    newtracer->ENV_HOST            = TRACER_ENV_HOST;
    newtracer->ENV_OS              = TRACER_ENV_OS;
    newtracer->ENV_PRODUCT_NAME    = "ModelVerify";
    newtracer->ENV_PRODUCT_VERSION = TRACER_ENV_PRODUCT_VERSION;
    newtracer->ENV_PRODUCT_BUILD   = TRACER_ENV_PRODUCT_BUILD;

    memset(newtracer->encoding,0,ENCODING_STR_LEN + 1);
    // newtracer->traceIdSeq = 1;
    newtracer->transitionIdSeq = 1;
    newtracer->modelOperationIdSeq = 1;
    newtracer->traceId = -1;

    // All accidental transitions does not traced by default
    newtracer->traceAccidentalTransitions = tr_false;

    TraceList_clear(&(newtracer->environment));
    TraceList_clear(&(newtracer->transitions));
    TraceList_clear(&(newtracer->model_operations));
    TraceList_clear(&(newtracer->exception_values));
    TraceList_clear(&(newtracer->exception_infos));

    // Set user environment
	// env_duplicate(&newtracer->environment, &user_environment);

    // Set system environment
    tsEnv = TSEnvironment_createTSEnvironment();
    tmppair = TracePair_create( tracer_cloneString(newtracer->ENV_HOST)
                              , (*tsEnv->getHostInfo)()	// will not be free()'d
                              );
    TraceList_add_back(&(newtracer->environment), tmppair);

    tmppair = TracePair_create( tracer_cloneString(newtracer->ENV_OS)
                              , (*tsEnv->getOSInfo)()	// will not be free()'d
                              );
    TraceList_add_back(&(newtracer->environment), tmppair);

    tmppair = TracePair_create_clone( newtracer->ENV_PRODUCT_NAME
                                    , "ModelVerify"
                                    );
    TraceList_add_back(&(newtracer->environment), tmppair);

    tmppair = TracePair_create_clone( newtracer->ENV_PRODUCT_VERSION
                                    , ts_get_ctesk_version()
                                    );
    TraceList_add_back(&(newtracer->environment), tmppair);

    tmppair = TracePair_create_clone( newtracer->ENV_PRODUCT_BUILD
                                    , ts_get_ctesk_build()
                                    );

    TraceList_add_back( &(newtracer->environment), tmppair );

    newtracer->transitionBuffer = create_StringBuffer();
    newtracer->transitionBuffer_wr
            = TraceBufferWriter_create( newtracer->transitionBuffer );

    if( newtracer->transitionBuffer_wr )
      newtracer->transitionBuffer_wr->refCnt = 1;

    newtracer->keepBuffer = create_StringBuffer();
    newtracer->keepBuffer_wr = TraceBufferWriter_create( newtracer->keepBuffer );
    if( newtracer->keepBuffer_wr )
      newtracer->keepBuffer_wr->refCnt = 1;

    newtracer->writer      = tracer_getDefaultWriter();
    newtracer->formatter   = tracer_getDefaultFormatter();
    newtracer->constrainer = tracer_getDefaultConstrainer();

    if( tracer_ready( newtracer ) == tr_false )
    {
      tracer_deleteTracer( newtracer );
      newtracer = NULL;
    }
  }
  return newtracer;
}

/*****************************/

void tracer_lock(class_Tracer * tracer)
{
#ifdef MULTITHREADED_TRACING
	ts_close_semaphore( tracer->semaphore );
#endif
}

void tracer_unlock(class_Tracer * tracer)
{
#ifdef MULTITHREADED_TRACING
	ts_open_semaphore( tracer->semaphore );
#endif
}

/*****************************/

/*
 * Clear exception_values
 */
static void do_clear_exception_values( class_Tracer* tracer )
{
    deletelist_tracer_exception_values(&(tracer->exception_values));
    deletelist_tracer_exception_infos(&(tracer->exception_infos));
}

void clear_exception_values( class_Tracer* tracer )
{
	tracer_lock( tracer );
	do_clear_exception_values( tracer );
	tracer_unlock( tracer );
}

void tracer_deleteTracer( class_Tracer* tracer )
{
  if( tracer != NULL )
  {
    TraceWriteController_delete( tracer->writer );
    TraceFormatter_delete( tracer->formatter );
    TraceConstrainer_delete( tracer->constrainer );
    TraceWriter_delete( tracer->transitionBuffer_wr );
    TraceWriter_delete( tracer->keepBuffer_wr );
    //TraceList_itr()
    deletelist_tracer_environment( &(tracer->environment) );
    deletelist_tracer_transitions( &(tracer->transitions) );
    deletelist_tracer_model_operations( &(tracer->model_operations) );

    do_clear_exception_values( tracer );

#ifdef MULTITHREADED_TRACING
    ts_destroy_semaphore( tracer->semaphore );
#endif
    free( tracer );
  }
}

/*****************************/

class_Tracer* tracer_getDefaultTracer()
{
  if( NULL == c_Tracer_default_tracer )
  {
    c_Tracer_default_tracer = tracer_createTracer();
    if( NULL != c_Tracer_default_tracer )
    {
        atexit( TracerExitTrigger );
    }
  }
  return c_Tracer_default_tracer;
}

/*****************************/

static TraceWriteController* tracer_getDefaultWriter()
{
  return TraceMultiWriter_create();
}

static TraceFormatter* tracer_getDefaultFormatter()
{
  return TraceXMLFormatter_default_create();
}

static TraceConstrainer* tracer_getDefaultConstrainer()
{
  return TraceConstrainer_TraceConstrainer();
}

/*****************************/
// tl - tracer->model_operations
static unsigned int tracer_getModelOperationReference( TraceList tl, long refId )
{
  TraceListitem *item;
  // std::map<long,unsigned>::const_iterator pair;
  struct _pair
  {
    long first;
    unsigned second;
  } *pair;

  if(refId != -1)
  {
    for( item = tl.head; item != NULL; item = item->next )
    {
      pair = (struct _pair *)item->data;
      if (pair != NULL && pair->first == refId ) {
        return pair->second;
	  }
    }
  }

  return 0;
}


/*****************************/

/*
 * Methods for getting current servers
 */
TraceWriteController * tracer_getWriter( class_Tracer* tracer )
{
  return (tracer != NULL ? tracer->writer : NULL);
}

TraceFormatter * tracer_getFormatter( class_Tracer* tracer )
{
  return (tracer != NULL ? tracer->formatter : NULL);
}

TraceConstrainer * tracer_getConstrainer( class_Tracer* tracer )
{
  return (tracer != NULL ? tracer->constrainer : NULL);
}

/*****************************/

/*
 * Methods for setting user environment
 */
void tracer_setUserEnv(class_Tracer *tracer, const char *name, const char *value)
{
TracePair *tmppair;
	if (tracer == NULL)	return;

    tracer_lock( tracer );

    tmppair = TracePair_create_clone(name, value);
    TraceList_add_back(&tracer->environment, tmppair);

	tracer_unlock( tracer );
}


/*
 * Methods for getting and setting encoding
 */
void tracer_setEncoding(class_Tracer* tracer, const char* encoding )
{
    if( tracer && encoding ) {
	  tracer_lock( tracer );
      memset( tracer->encoding, 0, ENCODING_STR_LEN );
      strncpy( tracer->encoding, encoding, ENCODING_STR_LEN );
	  tracer_unlock( tracer );
    }
}



/*
 * Methods for setting current servers
 */
TraceBool tracer_addWriter(class_Tracer* tracer, const char* name, TraceWriter* writer)
{
  if( !(tracer_valid( tracer ))) return tr_false;

  if( (*tracer->constrainer->isInScenario)( tracer->constrainer ) )
  {
    TraceWriter_delete( writer );
    return tr_false;
  }

  tracer_lock( tracer );

  if( (*tracer->constrainer->isInTrace)( tracer->constrainer ) )
  {
    if( writer != NULL && writer->puts != NULL )
    {
	  (*writer->puts)
          ( writer->state
          , (*tracer->formatter->startTrace)( tracer->formatter->state, tracer->encoding )
          );
    }
  }

  (*tracer->writer->addWriter)( tracer->writer->state, name, writer );

  tracer_unlock( tracer );
  return tr_true;
}

/*****************************/

TraceBool tracer_removeWriter( class_Tracer * tracer, const char* name )
{
TraceBool res;

  if(    !tracer_valid( tracer )
      || (*tracer->constrainer->isInScenario)( tracer->constrainer )
    )
    return tr_false;

  tracer_lock( tracer );

  if( (*tracer->constrainer->isInTrace)( tracer->constrainer ) )
  {
    res = (*tracer->writer->removeWriter)
               ( tracer->writer->state
               , name
               , (*tracer->formatter->endTrace)( tracer->formatter->state, 0)
               );
  } else {
    res = (*tracer->writer->removeWriter)( tracer->writer->state, name, create_String("") );
  }

  tracer_unlock( tracer );
  return res;
}

/*****************************/

TraceBool tracer_hasWriter( class_Tracer* tracer, const char* name )
{
TraceBool res;

  if( !tracer_valid( tracer ) ) return tr_false;

  tracer_lock( tracer );
  res = (*tracer->writer->hasWriter)( tracer->writer->state, name );
  tracer_unlock( tracer );
  return res;
}

/*****************************/

TraceBool tracer_isTraceEnabled( class_Tracer* tracer )
{
TraceBool res;

  if( !tracer_valid( tracer ) ) return tr_false;

  tracer_lock( tracer );
  res = (*tracer->writer->isTraceEnabled)( tracer->writer->state );
  tracer_unlock( tracer );
  return res;
}

/*****************************/

TraceFormatter* tracer_setFormatter( class_Tracer* tracer, TraceFormatter* formatter )
{
  TraceFormatter* old;

  if( NULL == tracer ) return NULL;

  tracer_lock( tracer );

  old = tracer->formatter;
  tracer->formatter = formatter;

  tracer_unlock( tracer );
  return old;
}

/*****************************/

TraceConstrainer* tracer_setConstrainer( class_Tracer* tracer
                                       , TraceConstrainer* constrainer
                                       )
{
  TraceConstrainer* old;

  if( NULL == tracer ) return NULL;

  tracer_lock( tracer );

  old = tracer->constrainer;
  tracer->constrainer = constrainer;

  tracer_unlock( tracer );
  return old;
}



/*
 * Method for getting trace id
 */
int tracer_getTraceId(class_Tracer* tracer)
{
  if(    !tracer_valid( tracer )
      || !(*tracer->constrainer->isInTrace)( tracer->constrainer )
    )
  {
    return -1;
  }
  return tracer->traceId;
}



/*
 * Enable/disable tracing of accidental transitions (disabled by default)
 */
TraceBool tracer_setTraceAccidental(class_Tracer* tracer, TraceBool enable)
{
TraceBool wasTraceAccidentalTransitions;

  if(NULL == tracer) return tr_false;

  tracer_lock( tracer );

  wasTraceAccidentalTransitions = tracer->traceAccidentalTransitions;
  tracer->traceAccidentalTransitions = enable;
  tracer->needTrace = tr_true;
  tracer->needTraceState = tr_true;

  tracer_unlock( tracer );
  return wasTraceAccidentalTransitions;
}



/*
 * Trace level methods
 */
void tracer_startTrace(class_Tracer* tracer)
{
  if( tracer_valid( tracer ) )
  {
    tracer_lock( tracer );

    if( (*tracer->constrainer->startTrace)( tracer->constrainer ) )
    {
      // Set trace identifier
      tracer->traceId = traceIdSeq ++; // tracer->traceIdSeq++;

      do_clear_exception_values( tracer );

      // All scenario values preceded the first transition should be traced
      tracer->needTrace = tr_true;
      tracer->needTraceState = tr_true;
      // Write message
	  write_string( tracer->writer
			 , (*tracer->formatter->startTrace)
				   (  tracer->formatter->state
					, tracer->encoding
				   )
             );
    }

	tracer_unlock( tracer );
  }
}

/*****************************/
void tracer_endTrace( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->endTrace)( tracer->constrainer )
    )
  {
    write_string( tracer->writer
			, (*tracer->formatter->endTrace)( tracer->formatter->state, 1 )
        );
  }

  tracer_unlock( tracer );
}



/*
 * Scenario level methods
 */
void tracer_traceScenarioStart( class_Tracer* tracer, const char* name )
{
char *host, *os;

  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceScenarioStart)( tracer->constrainer, name )
    )
  {
    TSEnvironment* tsEnv = TSEnvironment_createTSEnvironment();
    time_t now = time(NULL);

    write_string(  tracer->writer
			, (*tracer->formatter->traceEnvironmentProperties)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , &(tracer->environment)
				  )
	);

    write_string(  tracer->writer
			, (*tracer->formatter->traceScenarioStart)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , name
				  , now
				  , host = (*tsEnv->getHostInfo)()
				  , os = (*tsEnv->getOSInfo)()
				  )
	);

	free(host);
	free(os);
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceScenarioParameters( class_Tracer* tracer
                                   , int argc
                                   , const char** argv
                                   )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceScenarioParameters)( tracer->constrainer, argc, argv )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->traceScenarioParameters)
				( tracer->formatter->state
				, tracer->traceId
				, argc
				, argv
				)
		   );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceTestEngineProperty( class_Tracer* tracer
                                   , const char* name
                                   , const char* value
                                   )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceTestEngineProperty)( tracer->constrainer, name, value )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->traceTestEngineProperty)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , name
				  , value
				  )
           );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceScenarioEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceScenarioEnd)( tracer->constrainer )
    )
  {
    time_t now = time( NULL );
	write_string(  tracer->writer
			, (*tracer->formatter->traceScenarioEnd)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , now
				  )
		   );
  }

  tracer_unlock( tracer );
}



/*
 * Transition level methods
 */
void tracer_traceScenarioValue( class_Tracer* tracer
                              , const char* kind
                              , const char* type
                              , const char* name
                              , const char* value
                              )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceScenarioValue)
              ( tracer->constrainer
              , kind
              , type
              , name
              , value
              )
    )
  {
    if(    (*tracer->constrainer->isInTransition)( tracer->constrainer )
        || tracer->traceAccidentalTransitions
        || tracer->needTraceState
      )
    {
      // skip state message if transition wasn't traced
	  write_string(  tracer->writer
			  , (*tracer->formatter->traceScenarioValue)
					( tracer->formatter->state
					, tracer->traceId
					, kind
					, type
					, name
					, value
					)
			  );
    }
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceState( class_Tracer* tracer, const char* id )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceState)( tracer->constrainer, id )
    )
  {
    if(    (*tracer->constrainer->isInTransition)( tracer->constrainer )
        || tracer->traceAccidentalTransitions
        || tracer->needTraceState
      )
    {
      // skip state message if transition wasn't traced
	  write_string(  tracer->writer
			  , (*tracer->formatter->traceState)
					( tracer->formatter->state
					, tracer->traceId
					, id
					)
			  );
    }
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceTransitionStart( class_Tracer* tracer, const char* id )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceTransitionStart)( tracer->constrainer, id )
    )
  {
    if( !tracer->traceAccidentalTransitions )
    {
      clear_StringBuffer( r(tracer->transitionBuffer) );

      (*tracer->writer->pushWriters)( tracer->writer->state );

      (*tracer->writer->addWriter)
          ( tracer->writer->state, "$transition", tracer->transitionBuffer_wr );

      tracer->needTrace = tr_false;
      tracer->needTraceState = tr_false;
    }

	write_string(  tracer->writer
			, (*tracer->formatter->traceTransitionStart)
				  ( tracer->formatter->state , tracer->traceId ,id )
		   );

    // Initialize transition local variables
    tracer->modelOperationIdSeq = 1;
    deletelist_tracer_model_operations( &(tracer->model_operations) );

    tracer->wasSeries = tr_false;

    clear_StringBuffer( r(tracer->keepBuffer) );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceTransitionEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceTransitionEnd)( tracer->constrainer )
    )
  {
    // Write keeping trace
    if(    !tracer->wasSeries
        && size_StringBuffer(r(tracer->keepBuffer)) > 0
      )
    {
	  write_string(  tracer->writer
			  , (*tracer->formatter->traceSeriesStart)
					( tracer->formatter->state , tracer->traceId )
          );

	  write_string( tracer->writer, toString(r(tracer->keepBuffer)) );

	  write_string(  tracer->writer
			  , (*tracer->formatter->traceSeriesEnd)
					( tracer->formatter->state, tracer->traceId )
			  );

      clear_StringBuffer( r(tracer->keepBuffer) );
    }

    // Write 'TransitionEnd' message
	write_string(  tracer->writer
			, (*tracer->formatter->traceTransitionEnd)
				  ( tracer->formatter->state, tracer->traceId )
			);

    // Process accidental transitions filter
    if( !tracer->traceAccidentalTransitions )
    {
      (*tracer->writer->popWriters)( tracer->writer->state );

      if( tracer->needTrace ) {
		write_string( tracer->writer, toString(r(tracer->transitionBuffer)) );
      }

      tracer->needTraceState = tracer->needTrace;
    }
  }

  tracer_unlock( tracer );
}



/*
 * Oracle level methods
 */
void tracer_traceModelOperationStart( class_Tracer* tracer
                                    , const char* kind
                                    , const char* subsystem
                                    , const char* signature
                                    )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceModelOperationStart)
              ( tracer->constrainer, kind, subsystem, signature )
    )
  {
    write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationStart)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , kind
				  , subsystem
				  , signature
				  , tracer->modelOperationIdSeq++
				  )
        );

    tracer->needTrace = tr_true;
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationArgument( class_Tracer* tracer
                                       , const char* type
                                       , const char* name
                                       , const char* value
                                       )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceModelOperationArgument)
              ( tracer->constrainer, type, name, value )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationArgument)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , type
				  , name
				  , value
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationResult( class_Tracer* tracer
                                     , const char* type
                                     , const char* value
                                     )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceModelOperationResult)
            ( tracer->constrainer, type, value )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationResult)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , type
				  , value
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationIdentifier( class_Tracer* tracer, long id )
{
  TraceListitem *item;
  struct _pair
  {
    long first;
    unsigned second;
  } *pair;

  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceModelOperationIdentifier)
              ( tracer->constrainer, id )
    )
  {
    for( item = tracer->model_operations.head; item != NULL; item = item->next ) {
      pair = (struct _pair *)item->data;
      if( pair!=NULL && pair->first==id )
        break;
    }

    if( item == NULL ) {
      pair = (struct _pair *)malloc( sizeof( *pair ) );

      if( pair != NULL ) {
        pair->first = id;
        pair->second = tracer->modelOperationIdSeq - 1;
        TraceList_add_back( &(tracer->model_operations), pair );
      }
    }
    else
      pair->second = tracer->modelOperationIdSeq - 1;
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationChannel( class_Tracer* tracer, long chid )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceModelOperationChannel)
              ( tracer->constrainer, chid )
    )
  {
    char ss[ 10 ];

    if( chid == -1 )
      strcpy( ss,"unique" );
    else
      sprintf( ss, "%ld\0", chid );

    write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationChannel)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , ss
				  )
        );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationTimestamp( class_Tracer* tracer, const char* timestamp )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceModelOperationTimestamp)
              ( tracer->constrainer, timestamp )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationTimestamp)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , timestamp
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceOracleStart( class_Tracer* tracer
                            , const char* subsystem
                            , const char* signature
                            , long refId
                            )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && ( (*tracer->constrainer->traceOracleStart)
               ( tracer->constrainer
               , subsystem
               , signature
               , refId
               )
         )
    )
  {
    write_string(  tracer->writer
			, (*tracer->formatter->traceOracleStart)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , subsystem
				  , signature
				  , tracer_getModelOperationReference( tracer->model_operations, refId )
				  )
        );
  }
  else
  if( (*tracer->constrainer->keepOracleStart)
          ( tracer->constrainer
          , subsystem
          , signature
          , refId
          )
    )
  {
    (*tracer->writer->pushWriters)( tracer->writer->state );
    (*tracer->writer->addWriter)( tracer->writer->state
                                , "$keep"
                                , tracer->keepBuffer_wr
                                );

    write_string(  tracer->writer
			, (*tracer->formatter->traceOracleStart)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , subsystem
				  , signature
				  , tracer_getModelOperationReference( tracer->model_operations, refId )
				  )
        );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_tracePreconditionEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->tracePreconditionEnd)( tracer->constrainer )
    )
    write_string(  tracer->writer
			, (*tracer->formatter->tracePreconditionEnd)
				  ( tracer->formatter->state, tracer->traceId )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceOracleEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if( tracer_valid( tracer ) )
  {
    if( (*tracer->constrainer->traceOracleEnd)( tracer->constrainer ) )
    {
	  write_string(  tracer->writer
			  , (*tracer->formatter->traceOracleEnd)
					( tracer->formatter->state, tracer->traceId )
             );

      do_clear_exception_values( tracer );

  } else if( (*tracer->constrainer->keepOracleEnd)( tracer->constrainer ) )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceOracleEnd)
				  ( tracer->formatter->state , tracer->traceId )
           );

    (*tracer->writer->popWriters)( tracer->writer->state );

    do_clear_exception_values( tracer );
  }
 }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceModelOperationEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceModelOperationEnd)( tracer->constrainer )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceModelOperationEnd)
				  ( tracer->formatter->state, tracer->traceId )
        );

    do_clear_exception_values( tracer );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceSeriesStart(class_Tracer* tracer)
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceSeriesStart)( tracer->constrainer )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceSeriesStart)( tracer->formatter->state
													, tracer->traceId
													)
        );

    if( size_StringBuffer(r(tracer->keepBuffer))> 0 )
    {
	  write_string( tracer->writer, toString(r(tracer->keepBuffer)) );
      tracer->wasSeries = tr_true;
    }
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceSeriesEnd(class_Tracer* tracer)
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceSeriesEnd)( tracer->constrainer )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceSeriesEnd)
				( tracer->formatter->state, tracer->traceId )
        );
  }

  tracer_unlock( tracer );
}



/*
 * Coverage level methods
 */
void tracer_tracePrimeFormula( class_Tracer* tracer, int id, TraceBool value )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->tracePrimeFormula)
            (tracer->constrainer, id, value )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->tracePrimeFormula)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , id
				  , value
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceCoverageElement( class_Tracer* tracer
                                , const char *structureId
                                , const char* coverageId
                                , int branchId
                                )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceCoverageElement)
            ( tracer->constrainer
            , structureId
            , coverageId
            , branchId
            )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceCoverageElement)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , structureId
				  , coverageId
				  , branchId
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceMark( class_Tracer* tracer, const char* mark )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceMark)( tracer->constrainer, mark )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceMark)
				  ( tracer->formatter->state, tracer->traceId ,mark )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceCoverageStructureStart( class_Tracer* tracer, const char* name )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceCoverageStructureStart)
              ( tracer->constrainer, name )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceCoverageStructureStart)
				  ( tracer->formatter->state, tracer->traceId, name )
        );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceFormulaeStart( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceFormulaeStart)( tracer->constrainer )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceFormulaeStart)( tracer->formatter->state )
           );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceFormula( class_Tracer* tracer, int id, const char* text )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceFormula)( tracer->constrainer, id, text )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceFormula)
				  ( tracer->formatter->state
				  , id
				  , text
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceFormulaeEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceFormulaeEnd)( tracer->constrainer )
    )
  {
	write_string(  tracer->writer
	        , (*tracer->formatter->traceFormulaeEnd)( tracer->formatter->state )
        );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceCoverageStart( class_Tracer* tracer, const char* id )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceCoverageStart)( tracer->constrainer, id)
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceCoverageStart)( tracer->formatter->state, id )
           );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceElement( class_Tracer* tracer, int id, const char* name )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceElement)( tracer->constrainer, id, name )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceElement)
				  ( tracer->formatter->state
				  , id
				  , name
				  )
        );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceCoverageEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceCoverageEnd)( tracer->constrainer )
    )
	write_string(  tracer->writer
			, (*tracer->formatter->traceCoverageEnd)( tracer->formatter->state )
           );

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceCoverageStructureEnd( class_Tracer* tracer )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceCoverageStructureEnd)( tracer->constrainer )
    )
	write_string(  tracer->writer
	        , (*tracer->formatter->traceCoverageStructureEnd)( tracer->formatter->state )
           );

  tracer_unlock( tracer );
}


/*
 * Other messages
 */
void tracer_traceException( class_Tracer* tracer, const char* kind )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceException)( tracer->constrainer, kind )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceException)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , kind
				  , &(tracer->exception_values)
				  , &(tracer->exception_infos)
				  )
        );

    if( !(tracer->traceAccidentalTransitions) )
      tracer->needTrace = tr_true;

    do_clear_exception_values( tracer );
  }

  tracer_unlock( tracer );
}

/*****************************/
void tracer_traceInterimException( class_Tracer* tracer , const char* kind )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceInterimException)( tracer->constrainer, kind )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceInterimException)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , kind
				  , &(tracer->exception_values)
				  , &(tracer->exception_infos)
				  )
        );

    if( !(tracer->traceAccidentalTransitions) )
      tracer->needTrace = tr_true;

    do_clear_exception_values( tracer );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceExceptionValue( class_Tracer* tracer
                               , const char* name
                               , const char* value
                               )
{
  TracePair *pair;

  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceExceptionValue)
              (tracer->constrainer, name, value )
    )
  {
    pair = TracePair_create_clone( name, value );
    if( pair != NULL )
      TraceList_add_back( &(tracer->exception_values), pair );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceExceptionInfo( class_Tracer* tracer, const char* info )
{
  char* str;

  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceExceptionInfo)( tracer->constrainer, info )
    )
  {
    str = tracer_cloneString( info );
    if( str )
      TraceList_add_back( &(tracer->exception_infos), str );
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceInternalError( class_Tracer* tracer, const char* info )
{
  tracer_lock( tracer );

  if(    tracer_valid( tracer )
      && (*tracer->constrainer->traceInternalError)( tracer->constrainer, info )
    )
  {
	write_string(  tracer->writer
		, (*tracer->formatter->traceInternalError)
			( tracer->formatter->state, tracer->traceId,info )
		);

    if( !(tracer->traceAccidentalTransitions) )
      tracer->needTrace = tr_true;
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceSystemInfo( class_Tracer* tracer, const char* info )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceSystemInfo)( tracer->constrainer, info )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceSystemInfo)
				  ( tracer->formatter->state, tracer->traceId,info )
        );

    if ( !(tracer->traceAccidentalTransitions) )
      tracer->needTrace = tr_true;
  }

  tracer_unlock( tracer );
}

/*****************************/

void tracer_traceUserInfo( class_Tracer* tracer, const char* info )
{
  tracer_lock( tracer );

  if(    tracer_valid(tracer)
      && (*tracer->constrainer->traceUserInfo)( tracer->constrainer, info )
    )
  {
	write_string(  tracer->writer
			, (*tracer->formatter->traceUserInfo)
				  ( tracer->formatter->state
				  , tracer->traceId
				  , info
				  )
        );

    if( !(tracer->traceAccidentalTransitions) )
      tracer->needTrace = tr_true;
  }

  tracer_unlock( tracer );
}

