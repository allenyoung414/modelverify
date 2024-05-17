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
 * FILE:        ndfsm.c
 * AUTHOR:      Alexey V. Khoroshilov (hed)
 * CREATION:    11 Apr 2005
 * DESCRIPTION: Implementation of the test engine
 *              for testing a group of functions
 *              by traversing a nondetermenictic
 *              graph contained mutually connected
 *              deterministic spanning subgraph.
 */

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <ts/engine_tsi.h>
#include <ts/ndfsm.h>
#include <ts/register_tsi.h>
#include <ts/system.h>
#include <atl/arraylist.h>
#include <atl/int2ptrhash.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>

/********************************************************************/
/**                     NDFSM internal parameters                  **/
/********************************************************************/
#define NDFSM_NODES_ALLOC_BY        1024
#define NDFSM_PATH_ALLOC_BY         512
#define NDFSM_ARCS_ALLOC_BY         32
#define NDFSM_PATH_FINDER_HASH_SIZE 1024

/********************************************************************/
/**             Declarations of NDFSM Node and NDFSM Arc           **/
/********************************************************************/
typedef struct NDFSM_Node NDFSM_Node;
typedef struct NDFSM_Arc  NDFSM_Arc;


/********************************************************************/
/**               Operations for work with NDFSM Arc               **/
/********************************************************************/
struct NDFSM_Arc
{
  int         id;         // unique transition identifier
  int         start_node; // index of the start node of the transition
  ArcSymbol   symbol;     // symbol marking the arc
  NDFSM_Node* end_node;   // end node of the transition
};


/*
 * init_NDFSM_Arc does not decrement reference counter of par.
 */
static
void init_NDFSM_Arc( NDFSM_Arc* arc, int id, int start_node, int kind, Object* value, NDFSM_Node* end_node )
{
  assertion( (arc != NULL), "init_NDFSM_Arc: arc is NULL" );

  arc->id           = id;
  arc->start_node   = start_node;
  arc->symbol.kind  = kind;
  arc->symbol.value = value;
  arc->end_node     = end_node;
}

static void destroy_NDFSM_Arc( NDFSM_Arc* arc )
{
  assertion( (arc != NULL), "destroy_NDFSM_Arc: arc is NULL" );

  destroy_ArcSymbol( &arc->symbol );
}


/********************************************************************/
/**               Operations for work with NDFSM Node              **/
/********************************************************************/
struct NDFSM_Node
{
  int        id;             // unique node identifier
  char       id_string[16];  // string representation of the unique node identifier
  Object*    node_state;     // generalized state of the node
  String*    node_string;    // string representation of the generalized state of the node
  bool       is_finished;    //
  ArcSymbol  next_symbol;    // the next symbol
  ArrayList  arcs;           // deterministic transitions started in the node
};


/*
 * init_NDFSM_Node does not decrement reference counter of node_state
 * because stores it in node->node_state.
 */
static void init_NDFSM_Node( NDFSM_Node* node, int id, Object* node_state )
{
  assertion( (node != NULL), "init_NDFSM_Node: node is NULL" );

  node->id = id;
  sprintf( node->id_string, "%d", id );
  node->node_state = node_state;
  node->node_string = toString( r(node_state) );
  node->is_finished = false;
  init_ArcSymbol( &node->next_symbol );
  init_ArrayList( &node->arcs, sizeof(NDFSM_Arc), NDFSM_ARCS_ALLOC_BY, (Destructor)destroy_NDFSM_Arc );
}

static void destroy_NDFSM_Node( NDFSM_Node* node )
{
  assertion( (node != NULL), "destroy_NDFSM_Node: node is NULL" );

  destroy( node->node_state );
  destroy( node->node_string );
  destroy_ArcSymbol( &node->next_symbol );
  destroy_ArrayList( &node->arcs );
}


/********************************************************************/
/**         Operations for work with NDFSM Test Engine Data        **/
/********************************************************************/
typedef struct
{
  ndfsm*            test;                      // ndfsm test scenario data
  ArcSymbolExecutor executor;                  // executor of arc symbols
  ArrayList         nodes;                     // nodes of the FSM
  int               arc_number;                // number of arcs in the FSM
  int               nonfinished_nodes_number;  // number of nonfinished nodes
  ArrayList         selected_path;             // path to selected nonfinished node (in reverse order)
  int               current_arc;               // index of current arc in the selected path

  bool              fatal_failure;             // flag for fatal failure
  int               failure_number;            // number of failure found by the test scenario
} NDFSM_TestEngineData;

static void init_NDFSM_TestEngineData( NDFSM_TestEngineData* ndfsm_state, ndfsm* test )
{
  assertion( (ndfsm_state != NULL), "init_NDFSM_TestEngineData: ndfsm_state is NULL" );
  assertion( (test != NULL), "init_NDFSM_TestEngineData: test is NULL" );

  ndfsm_state->test = test;
  ndfsm_state->executor.actions = test->actions;
  ndfsm_state->executor.model_state.saveModelState    = test->saveModelState;
  ndfsm_state->executor.model_state.restoreModelState = test->restoreModelState;
  ndfsm_state->executor.model_state.isStationaryState = test->isStationaryState;
  ndfsm_state->executor.model_state.observeState      = test->observeState;
  init_ArrayList( &ndfsm_state->nodes, sizeof(NDFSM_Node), NDFSM_NODES_ALLOC_BY, (Destructor)destroy_NDFSM_Node );
  ndfsm_state->arc_number = 0;
  ndfsm_state->nonfinished_nodes_number = 0;
  init_ArrayList( &ndfsm_state->selected_path, sizeof(NDFSM_Arc*), NDFSM_PATH_ALLOC_BY, NULL );
  ndfsm_state->current_arc = -1;
  ndfsm_state->fatal_failure = false;
  ndfsm_state->failure_number = 0;

  // normalize test data
  if (test->getState == NULL)
    scenario_set_default_get_state(test);
  // init DR mode and Time model
  if ((test->isStationaryState != NULL) &&
      (test->saveModelState != NULL)    &&
      (test->restoreModelState != NULL)
     )
   {setDeferredReactionsMode(true);
    setTSTimeModel(LinearTSTime);
   }
  else
   {setDeferredReactionsMode(false);
    setTSTimeModel(NotUseTSTime);
   }
}

static void destroy_NDFSM_TestEngineData( NDFSM_TestEngineData* ndfsm_state )
{
  assertion( (ndfsm_state != NULL), "destroy_NDFSM_TestEngineData: ndfsm_state is NULL" );

  destroy_ArrayList( &ndfsm_state->nodes );
  destroy_ArrayList( &ndfsm_state->selected_path );
}

static bool node_finder( void* element, void* key )
{
NDFSM_Node* node = (NDFSM_Node*)element;
Object* node_state = (Object*)key;

  return equals( r(node->node_state), r(node_state) );
}

static
NDFSM_Node* registerNode_NDFSM( NDFSM_TestEngineData* ndfsm_state, Object* node_state )
{
NDFSM_Node* node = (NDFSM_Node*)find_ArrayList( &ndfsm_state->nodes, node_finder, node_state );

  if (node != NULL)
   {
    destroy(node_state);
    return node;
   }
  node = (NDFSM_Node*)append_ArrayList( &ndfsm_state->nodes );
  init_NDFSM_Node( node, ndfsm_state->nodes.size-1, node_state );
  initArcSymbol( &ndfsm_state->executor, &node->next_symbol );
  if (!isNull_ArcSymbol( &node->next_symbol ))
   {
    ndfsm_state->nonfinished_nodes_number++;
   }
  else
   {
    node->is_finished = true;
   }
  return node;
}

static
NDFSM_Arc* registerArc_NDFSM( NDFSM_TestEngineData* ndfsm_state,
                              int         arcid,
                              NDFSM_Node* start_node,
                              ArcSymbol*  symbol,
                              NDFSM_Node* end_node
                            )
{
NDFSM_Arc* arc = NULL;

  arc = (NDFSM_Arc*)append_ArrayList( &start_node->arcs );
  init_NDFSM_Arc( arc, arcid, start_node->id, symbol->kind, symbol->value, end_node );
  return arc;
}

static bool nonfinished_node_finder( void* element, void* key )
{
NDFSM_Node* node = (NDFSM_Node*)element;

  if (!node->is_finished)
    return true;
  return false;
}

static
NDFSM_Node* findNonfinishedNode_NDFSM( NDFSM_TestEngineData* ndfsm_state )
{
  return (NDFSM_Node*)find_ArrayList( &ndfsm_state->nodes, nonfinished_node_finder, NULL );
}


/********************************************************************/
/**                         Trace functions                        **/
/********************************************************************/
static void trace_current_node( NDFSM_Node* current_node )
{
  traceState( current_node->id_string );
  traceScenarioValue( "state", "", "", toCharArray_String( r(current_node->node_string) ) );
}


/********************************************************************/
/*                          NDFSM Algorithm                         */
/********************************************************************/
#define NULL_NDFSM_ARC ((NDFSM_Arc*)-1)

typedef struct PathFinderData
{
  ArrayList      nodes_list;
  Int2PtrHashMap found_nodes;
} PathFinderData;

static void init_PathFinderData( PathFinderData* data, NDFSM_Node* current_node )
{
NDFSM_Node** node_holder;

  // init nodes list
  init_ArrayList( &data->nodes_list, sizeof(NDFSM_Node*), NDFSM_NODES_ALLOC_BY, NULL );
  node_holder = (NDFSM_Node**)append_ArrayList( &data->nodes_list );
  *node_holder = current_node;
  // init hash set
  init_Int2PtrHashMap( &data->found_nodes, NDFSM_PATH_FINDER_HASH_SIZE, NDFSM_PATH_FINDER_HASH_SIZE );
  put_Int2PtrHashMap( &data->found_nodes, current_node->id, NULL_NDFSM_ARC );
}

static void destroy_PathFinderData( PathFinderData* data )
{
  destroy_ArrayList( &data->nodes_list );
  destroy_Int2PtrHashMap( &data->found_nodes );
}

static bool nonfinished_state_finder( void* element, void* key )
{
NDFSM_Arc* arc = (NDFSM_Arc*)element;
PathFinderData* data = (PathFinderData*)key;
NDFSM_Node** node_holder;

  if (!arc->end_node->is_finished)
    return true;
  if (put_Int2PtrHashMap( &data->found_nodes, arc->end_node->id, arc ) == arc)
   {
    node_holder = (NDFSM_Node**)append_ArrayList( &data->nodes_list );
    *node_holder = arc->end_node;
   }
  return false;
}

static bool find_nonfinished_state( NDFSM_TestEngineData* ndfsm_state, NDFSM_Node* current_node )
{
NDFSM_Arc** arc_holder;
NDFSM_Node* node;
NDFSM_Arc* arc;
bool verdict;
int i;
PathFinderData* data = calloc( 1, sizeof(PathFinderData) );
  assertion( (data != NULL), "find_nonfinished_state: 'data' allocation error" );

  init_PathFinderData( data, current_node );
  clear_ArrayList( &ndfsm_state->selected_path );
  verdict = false;

  for( i = 0; i < data->nodes_list.size; i++ )
   {
    node = *(NDFSM_Node**)get_ArrayList( &(data->nodes_list), i );
    arc = (NDFSM_Arc*)find_ArrayList( &node->arcs, nonfinished_state_finder, data );
    if (arc != NULL)
     {
      for(;;)
       {
        arc_holder = (NDFSM_Arc**)append_ArrayList( &ndfsm_state->selected_path );
        *arc_holder = arc;

        arc = (NDFSM_Arc*)get_Int2PtrHashMap( &(data->found_nodes), arc->start_node );
        if (arc == NULL_NDFSM_ARC)
          break;
       }
      ndfsm_state->current_arc = ndfsm_state->selected_path.size - 1;
      verdict = true;
      break;
     }
   }

  destroy_PathFinderData( data );
  free( data );

  return verdict;
}

void executeArcSymbol_NDFSM( NDFSM_TestEngineData* ndfsm_state,
                             int arcid,
                             ArcSymbol* symbol,
                             ArcSymbol* next_symbol
                           )
{
bool verdict;

  verdict = executeArcSymbol( &ndfsm_state->executor, arcid, symbol, next_symbol );
  if (!verdict)
   {
    if (!areDeferredReactionsEnabled())
     {
      ndfsm_state->failure_number++;
     }
    else
     {
      ndfsm_state->fatal_failure = true;
     }
   }
}

NDFSM_Node* walk_new_arc_NDFSM( NDFSM_TestEngineData* ndfsm_state, NDFSM_Node* current_node )
{
int arcid;
NDFSM_Node* end_node;
ArcSymbol next_symbol;

  arcid = ndfsm_state->arc_number++;
  executeArcSymbol_NDFSM( ndfsm_state, arcid, &current_node->next_symbol, &next_symbol );
  end_node = registerNode_NDFSM( ndfsm_state, scenario_get_state(ndfsm_state->test) );
  if (current_node != end_node)
   {
    registerArc_NDFSM( ndfsm_state, arcid, current_node, &current_node->next_symbol, end_node );
    /* current_node->next_symbol moved to the newly created arc */
   }
  else
   {
    destroy_ArcSymbol( &current_node->next_symbol );
   }
  current_node->next_symbol.kind  = next_symbol.kind;
  current_node->next_symbol.value = next_symbol.value;
  if (isNull_ArcSymbol(&next_symbol))
   {
    current_node->is_finished = true;
    ndfsm_state->nonfinished_nodes_number--;
   }
  return end_node;
}

NDFSM_Node* walk_old_arc_NDFSM( NDFSM_TestEngineData* ndfsm_state, NDFSM_Node* current_node )
{
NDFSM_Arc** arc_holder;
NDFSM_Arc*  arc;
NDFSM_Node* end_node;
c_bool      old_value;

  arc_holder = (NDFSM_Arc**)get_ArrayList( &ndfsm_state->selected_path, ndfsm_state->current_arc );
  arc = *arc_holder;

  assertion( arc->start_node == current_node->id, "walk_old_arc_NDFSM: invalid selected path" );

  old_value = setTraceAccidental(true);
  executeArcSymbol_NDFSM( ndfsm_state, arc->id, &arc->symbol, NULL );
  setTraceAccidental(old_value);
  end_node = registerNode_NDFSM( ndfsm_state, scenario_get_state(ndfsm_state->test) );
  if (end_node == arc->end_node)
   {
    ndfsm_state->current_arc--;
   }
  else
   {
    // Nondeterministic arc has been found
    ndfsm_state->current_arc = -1;
    remove_ArrayList( &current_node->arcs, arc );
   }
  return end_node;
}

void traceProperties_NDFSM( NDFSM_TestEngineData* ndfsm_state )
{
char buff[64];

  // Deferred Reactions Enabled
  traceTestEngineProperty( "Deferred Reactions Enabled", areDeferredReactionsEnabled() ? "true" : "false" );
  if (areDeferredReactionsEnabled())
   {// WTime
    if (getWTimeMSec() % 1000 == 0)
      sprintf(buff,"%d secs",getWTime());
    else
      sprintf(buff,"%d millisecs",getWTimeMSec());
    traceTestEngineProperty( "WTime", buff );
	// Is Find First Series Only
	traceTestEngineProperty( "Find First Series Only", isFindFirstSeriesOnly() ? "true" : "false" );
   }
  // Time Model
  switch (getTSTimeModel())
   {case NotUseTSTime :
	  traceTestEngineProperty( "Time Model", "not used" );
	  break;
    case LinearTSTime :
	  traceTestEngineProperty( "Time Model", "linear" );
	  break;
	case DistributedTSTime :
	  traceTestEngineProperty( "Time Model", "distributed" );
	  break;
	default :
	  assertion( false, "Unexpected Time Model" );
   }
  // Finish Mode
  switch (getFinishMode())
   {case UNTIL_ERROR :
      if (tsNumberOfErrorsToFinish == 1)
        traceTestEngineProperty( "Finish Mode", "until an error" );
      else
       {
        sprintf( buff, "until %d errors", tsNumberOfErrorsToFinish );
        traceTestEngineProperty( "Finish Mode", buff );
       }
      break;
    case UNTIL_END :
      traceTestEngineProperty( "Finish Mode", "until end" );
      break;
    default :
      assertion( false, "Unexpected Finish Mode" );
   }
}

void traceDeterministicPathNotFound( NDFSM_TestEngineData* ndfsm_state, NDFSM_Node* current_node )
{
NDFSM_Node* untested_node;
String* buf;

  traceExceptionInfo("Deterministic path to the incompletely tested states not found. The ndfsm test engine can be used only for scenarios with state transition graph, "
                  "which contains mutually connected deterministic frame subgraph"
                 );
  buf = format_String("Current state is: %s", toCharArray_String(r(current_node->node_string)) );
  traceExceptionInfo(toCharArray_String(r(buf)));
  destroy(buf);
  traceExceptionInfo("All the states deterministically reachable from it are completely tested");
  //
  untested_node = findNonfinishedNode_NDFSM( ndfsm_state );
  if (untested_node != NULL)
  {
	StringBuffer *str_buf = create_StringBuffer();
	append_StringBuffer(       r(str_buf), "State '" );
	appendString_StringBuffer( r(str_buf), r(untested_node->node_string) );
    append_StringBuffer(       r(str_buf), "' may have untested transitions, but it is unreachable" );
	buf = toString(str_buf);
    traceExceptionInfo(toCharArray_String(r(buf)));
    destroy(buf);
  }
  traceException(UNCONNECTED_GRAPH_MESSAGE);
}

void traverse_NDFSM( NDFSM_TestEngineData* ndfsm_state )
{
NDFSM_Node* current_node;

  current_node = registerNode_NDFSM( ndfsm_state, scenario_get_state(ndfsm_state->test) );
  for(;;)
   {
    trace_current_node( current_node );

    // perform a transition
    if (!current_node->is_finished)
     {// current_node is not finished
      current_node = walk_new_arc_NDFSM( ndfsm_state, current_node );
     }
    else if (ndfsm_state->current_arc == -1)
     {// search for the nearest nonfinished state and start way to it
      if (find_nonfinished_state( ndfsm_state, current_node ))
       {
        current_node = walk_old_arc_NDFSM( ndfsm_state, current_node );
       }
      else
       {// deterministic path not found
        traceDeterministicPathNotFound( ndfsm_state, current_node );
        ndfsm_state->fatal_failure = true;
        // tracing of the last node has been already done
        return;
       }
     }
    else
     {// continue way to the nearest nonfinished state
      current_node = walk_old_arc_NDFSM( ndfsm_state, current_node );
     }

    // check for failure
    if (    ndfsm_state->fatal_failure
        || (tsNumberOfErrorsToFinish > 0) && (ndfsm_state->failure_number >= tsNumberOfErrorsToFinish)
       )
     {// break by failure
      break;
     }

    // check for success
    if (ndfsm_state->nonfinished_nodes_number == 0)
     {// all nodes are finished
      traceSystemInfo( "Test is complete, all available transitions are tested" );
      // break by success
      break;
     }
   }
  trace_current_node( current_node );
  return;
}


/********************************************************************/
/**                 Invariant on ndfsm engine state                **/
/********************************************************************/
static
bool check_consistency_NDFSM( ndfsm* test )
{
  if (areDeferredReactionsEnabled())
   {if (test->isStationaryState == NULL)
     {traceException( "is_stationary field should be initialized if deferred reactions are enabled" );
      return false;
     }
    if (test->saveModelState == NULL)
     {traceException( "save_model_state field should be initialized if deferred reactions are enabled" );
      return false;
     }
    if (test->restoreModelState == NULL)
     {traceException( "restore_model_state field should be initialized if deferred reactions are enabled" );
      return false;
     }
   }
  return true;
}


/********************************************************************/
/**                  Start NDFSM engine function                   **/
/********************************************************************/
bool start_ndfsm(int argc, char** argv, ndfsm* test)
{
NDFSM_TestEngineData ndfsm_state;
bool tracing_was_started_here = false;
bool was_accidental_transition_traced;
AssertionExit prev_assert = NULL;
TestSystemParameters* pars;
bool verdict;
bool verdict_init;

  // Initialize test engine data
  init_NDFSM_TestEngineData( &ndfsm_state, test );

  // Process standard parameters
  pars = processStandardParameters(test->name,"",test->actions,argc,argv);
  assertion( pars != NULL, "start_ndfsm: standard parameters processing fails");

  // Start tracing if it has not been started yet
  if (!isInTrace())
   {
    tracing_was_started_here = true;
    prev_assert = ts_setup_assert_function();
    startTrace();
   }

  START_EXCEPTION_PROTECTED_SECTION(ts_default_exception_handler);

  setCurrentTestScenarioName(test->name);
  traceScenarioStart(test->name);
  traceScenarioParameters(pars->argc,(const char**)pars->argv);
  // Deferred Reactions Enabled
  traceTestEngineProperty( "Deferred Reactions Enabled", areDeferredReactionsEnabled() ? "true" : "false" );

  // Scenario initialization
  verdict = true;
  if (test->init != NULL)
   {
    // Trace pseudo transition start
    was_accidental_transition_traced = setTraceAccidental(true);
    traceState("start");
    traceScenarioValue ("state", "", "", "start");
    traceTransitionStart("initialize");
    traceScenarioValue ("scenario method", "", "", "initialize");

    // Call 'init' function
    clearOracleVerdict();
    verdict_init = scenario_init(test, pars->argc, pars->argv);
    verdict = verdict_init && getOracleVerdict();

    // Frame end
    verdict = onFrameEnd( &ndfsm_state.executor.model_state, verdict );

    // Trace initialization failed message
    if (!verdict_init)
     {
      traceException(SCENARIO_INITIALIZATION_FAILED_MESSAGE);
      testVerdict = TD_faulted;
     }

    // Trace pseudo transition end
    traceTransitionEnd();
    setTraceAccidental(was_accidental_transition_traced);
   }

  // FSM traversing
  if (verdict)
   {if (check_consistency_NDFSM(test))
     {
      traceProperties_NDFSM( &ndfsm_state );
      traverse_NDFSM( &ndfsm_state );
      // Evaluate historical test verdict
      if (ndfsm_state.fatal_failure)
       {
        verdict = false;
        testVerdict = TD_faulted;
       }
      else if (ndfsm_state.failure_number > 0)
       {
        verdict = false;
        testVerdict = TD_bad_verdict;
       }
      else
        testVerdict = TD_ok;
     }
    if (test->finish != NULL)
     {
      // Trace pseudo transition start
      was_accidental_transition_traced = setTraceAccidental(true);
      traceTransitionStart("finalize");
      traceScenarioValue ("scenario method", "", "", "finalize");

      // Call 'finish' function
      clearOracleVerdict();
	  scenario_finish(test);
      verdict = verdict && getOracleVerdict();

      // Frame end
      verdict = onFrameEnd( &ndfsm_state.executor.model_state, verdict );

      // Trace pseudo transition end
      traceTransitionEnd();
      traceState("end");
      traceScenarioValue ("state", "", "", "end");
      setTraceAccidental(was_accidental_transition_traced);
     }
   }

  traceScenarioEnd();
  setCurrentTestScenarioName(NULL);

  // Deallocate resources
  destroy_NDFSM_TestEngineData( &ndfsm_state );

  END_EXCEPTION_PROTECTED_SECTION();

  // Stop tracing if it was started here
  if (tracing_was_started_here)
   {
    endTrace();
    ts_restore_assert_function(prev_assert);
   }

  // Disable standard parameters
  disableStandardParameters(pars);

  return verdict;
}

