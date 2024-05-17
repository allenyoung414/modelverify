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
 * FILE:        dfsm.c
 * AUTHOR:      Vitaliy Omelchenko (vitaliy)
 * CREATION:    2 Aug 2001
 * DESCRIPTION: Implementation of testing group
 *              of functions by browsing
 *              determenictic mutually connected
 *              graph
 * VERSION AND LIST OF CHANGES:
 *  1 Jul 2003  (hed)
 *   Support of deferred reactions has been added
 *
 * 12 Nov 2010 Nikolay Pakulin (npak)
 *   Added wrappers to run DFSM specified as a C++ class
 */

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <ts/engine_tsi.h>
#include <ts/cpp_iface.h>
#include <ts/dfsm.h>
#include <ts/register_tsi.h>
#include <ts/system.h>
#include <atl/string.h>
#include <atl/stringbuffer.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>

/********************************************************************/
/*  GD - pointer to a dfsm structure describing the group           */
/********************************************************************/
static dfsm* GD;
static ModelStateManager model_state_manager;

#include "scenario.h"

/********************************************************************/
/**                      Auxiliary functions                       **/
/********************************************************************/
static unsigned is_node_new(void* new_node);


/********************************************************************/
/**                          Type of FSM arcs                      **/
/********************************************************************/
typedef struct
{
  Object *par;  /* pointer to the object type containing test model parameters */
  int     fnum; /* number of function (kind of stimuli) in tested group */
  int     refs; /* reference counter */
  int     id;   /* unique transition identifier */
} FSM_Arc;

static int arc_id;

/********************************************************************/
/**                Operations for work with FSM arcs               **/
/********************************************************************/
/*
typedef struct
{
  Object *par;  // pointer to the object type containing test model parameters
  int     fnum; // number of function (kind of stimuli) in tested group
  int     refs; // reference counter
  int     id;   // unique transition identifier
} FSM_Arc;
*/

#ifdef _DEBUG_REFCNT
static void addRef(FSM_Arc *x) {
  if (x) {
    x->refs++;
    printf("inc FSM_Arc %p %d\n",x,x->refs+1);
  }
}
#else
#define addRef(x)   if ((x)) (x)->refs++
#endif

static FSM_Arc* newFSM_Arc(int n)
{
  FSM_Arc*  a = (FSM_Arc*)malloc(sizeof(FSM_Arc));

  assertion(a != 0, "No memory for new fsm arc");

  a->fnum = n;
  a->par = (n < 0) ? (void *)0 : action_init(GD->actions[n]);
  a->refs = 0;
  a->id = arc_id++;

#ifdef _DEBUG_REFCNT
  printf("create FSM_Arc %p\n",a);
#endif
  return a;
}

static void deleteFSM_Arc(FSM_Arc*  a)
{
    if (a && --a->refs < 0)
    {
#ifdef _DEBUG_REFCNT
      printf("destroy FSM_Arc %p\n",a);
#endif
      destroy(a->par); /* par must be object pointer !!!!!! */
      free(a);
    }
#ifdef _DEBUG_REFCNT
    if (a && a->refs >= 0)
      printf("dec FSM_Arc %p %d\n",a,a->refs+1);
#endif
}

static FSM_Arc* cloneFSM_Arc(FSM_Arc* a)
{
FSM_Arc* na = NULL;

  if (a)
    {
      na = (FSM_Arc *)malloc (sizeof (FSM_Arc));
      assertion(na != 0, "No memory to copy fsm arc");
      na->fnum = a->fnum;
      na->par  = clone_Object (r(a->par));
      na->id = a->id;
      na->refs = 0;
#ifdef _DEBUG_REFCNT
  printf("clone FSM_Arc %p\n",na);
#endif
    }

  return na;
}


/********************************************************************/
/**                         Trace functions                        **/
/********************************************************************/

/* The flag to suppress double tracing of the last state */
static bool no_last_transition = false;

static void trace_current_state (void)
{
  static char buf[100];
  Object *new_node;
  String *str, *type_str;
  const char *type;

#ifdef _DEBUG_REFCNT
  printf("*** start: trace_current_state()\n");
#endif
  new_node = scenario_get_state(GD);
  str = toTrace(r(new_node));

  sprintf (buf, "%d", is_node_new(new_node));
  traceState (buf);

  if (new_node == NULL) {
	type = "";
	type_str = NULL;
  } else {
	StringBuffer *str_buf = create_StringBuffer();
	append_StringBuffer( r(str_buf), TYPE(new_node)->name );
	append_StringBuffer( r(str_buf), " *" );
	type_str = toString(str_buf);
	type = toCharArray_String(r(type_str));
  }

  traceScenarioValue ("state", type, "", toCharArray_String(r(str)));
  destroy(str);
  destroy(type_str);
  destroy(new_node);
#ifdef _DEBUG_REFCNT
  printf("*** end:   trace_current_state()\n");
#endif
}

static void trace_transition_start (FSM_Arc *arc)
{
  static char buf[100];

  /* transition start */
  sprintf (buf, "%d", arc->id);
  traceTransitionStart (buf);

  /* scenario_value: scenario method*/
  traceScenarioValue ("scenario method"/*"scenario operation"*/, "", "", action_name(GD->actions[arc->fnum]));
}


/*===================================================================*/
/*  Functions iterating test scenario functions and their parameters */
/*===================================================================*/

/* Defines next scenario function (kind of stimulus) in a test scenario, */
/* if there are not scenario functions in the test scenario returns 0    */
static FSM_Arc* get_next_function(FSM_Arc* arc)
{
  FSM_Arc* cur_arc = arc;

  if (!GD->actions[0])
   {/* there are not scenario functions in the test scenario */
    return 0;
   }
  if (!cur_arc)
   {/* returns the first scenario function */
    return newFSM_Arc(0);
   }
  if (!GD->actions[cur_arc->fnum + 1])
   {/* the last scenario function, returns 0 */
    return 0;
   }

  /* returns the next scenario function */
  cur_arc = newFSM_Arc(cur_arc->fnum + 1);

  return cur_arc;
}

/* Return next arc and update coverage info for    */
/* current function and node calling manually      */
/* defined next_par, if there no arcs for current  */
/* function get next function, define its initial  */
/* arc and return it, if there are not functions   */
/* return 0                                        */
static FSM_Arc* get_next_arc(FSM_Arc* in_arc)
{
	char has_next = (in_arc)? action_has_next(GD->actions[in_arc->fnum], r(in_arc->par)) : false;
  if (!in_arc || !has_next) {
    return get_next_function(in_arc);
  }
  in_arc->id = arc_id++;
  return in_arc;
}


/***********************************************************/

/*===========================================================*/
/* Global variables, types and functions for traversed nodes */
/*===========================================================*/

/* Tested node */
typedef struct
{
  void*    node; /** node state **/
  FSM_Arc* barc; /** back-arc **/
/** the way along back-arcs finishes at PATH **/
  int      bnum; /** index of back-arc end in TESTED-list **/
  int      pnum; /** if the node is not at PATH then -1   **/
                 /** if the node is at PATH then its      **/
                 /** index in PATH-list                   **/
} tnode;

tnode*  new_tnode(void* n, FSM_Arc* b, int bn, int pn)
{
  tnode*  tn;

  tn = (tnode *) malloc (sizeof(tnode));

  assertion(tn != 0, "No memory for new tnode.");

  tn->node = n;
  tn->barc = b;
  addRef(b);
  tn->bnum = bn;
  tn->pnum = pn;

  return tn;
}

void delete_tnode(tnode* tn)
{
  destroy(tn->node); /* it must be object type !!!*/
  deleteFSM_Arc(tn->barc);
  free(tn);
}

/* The node at PATH, it can have untested arcs */
typedef struct
{
  int         tnum;      /** node index in TESTED-list **/
  FSM_Arc*    parc;      /** path-arc **/
  FSM_Arc*    larc;      /** last-called arc **/
  int         nback;     /** back-node index in PATH-list **/
                         /** back-node is the end of the  **/
                         /** way along back-arcs          **/
} pnode;

pnode* new_pnode(int tn, FSM_Arc* p, int nb)
{
  pnode* pn;

  pn = (pnode*) malloc (sizeof(pnode));

  assertion(pn != 0, "No memory for new pnode");

  pn->tnum  = tn;
  pn->parc  = 0;
  pn->larc  = p;
  addRef(p);

  pn->nback = nb;

  return pn;
}

void delete_pnode(pnode* pn)
{
  deleteFSM_Arc(pn->parc);
  deleteFSM_Arc(pn->larc);
  free(pn);
}
/*****************************************************/

/*===============================================================*/
/* Types and functions for lists of nodes and their global lists */
/*===============================================================*/
#define INIT_LIST_SIZE  100

typedef struct
{
  void**   list;
  unsigned len;
  unsigned bound;
} list;

/* creates a new list */
static
list* newList()
{
unsigned i;
list*    l;

  l = (list*) malloc(sizeof(list));

  assertion(l != 0, "No memory for new list");

  l->list = (void**)calloc(INIT_LIST_SIZE, sizeof(void*));

  assertion(l->list != 0, "No memory for new list");

  l->len   = 0;
  l->bound = INIT_LIST_SIZE;

  for (i = 0; i < l->bound; i++)
    l->list[i] = 0;

  return l;
}

/* adds the element given to the list */
static
void addToList(list* l, void* e)
{
unsigned i;

  if (l->len == l->bound)
   {
    l->list = (void**)realloc(l->list, (2*l->bound)*sizeof(void*) );

    assertion(l->list != 0, "No memory to realloc list");

    l->bound *= 2;
    for (i = l->len + 1; i < l->bound; i++)
      l->list[i] = 0;
   }

  l->list[l->len++] = e;
}

/** LIST OF TESTED STATES OF OBJECT **/
static list*  tested = 0;

/** PATH IN GRAPH - LIST OF STATES OF OBJECT UNDER TESTING **/
static list*  path = 0;

#define TESTED(i)       ((tnode*)(tested->list[(i)]))
#define PATH(i)         ((pnode*)(path->list[(i)]))

void deleteTestedList()
{
  if (tested)
  {unsigned i;
   for (i = 0; i < tested->len; i++)
     delete_tnode(TESTED(i));
   free(tested->list); // !!!
   free(tested);
   tested = 0;
  }
}

void deleteLastPathNode()
{
  if (path)
   {delete_pnode(PATH(--path->len));
    path->list[path->len] = 0;
   }
}

void deletePathList()
{
  if (path)
   {unsigned i;
    for (i = 0; i < path->len; i++)
      delete_pnode(PATH(i));
    free(path->list); // !!!
    free(path);
    path = 0;
   }
}
/*****************************************************/

/********************************************************************/
/**                        Call Function                           **/
/********************************************************************/
static
void execute_FSM_Arc(FSM_Arc* cur_arc, int second)
{
// PtrActionCall call;
c_bool old_value;
bool verdict;

  /* state */
  trace_current_state();

  assertion(cur_arc && (0 <= cur_arc->fnum) && (GD->actions[cur_arc->fnum] != NULL), "Bad arc");

//  call = GD->actions[cur_arc->fnum]->call;

  if (second)
   {
    old_value = setTraceAccidental(true);
   }

  trace_transition_start(cur_arc);

  // call stimulus
  {
	  Object * arg = (second) ? clone_Object (r(cur_arc->par)) : r(cur_arc->par);
	verdict = action_invoke(GD->actions[cur_arc->fnum], arg);
  }

  // frame end
  verdict = onFrameEnd( &model_state_manager, verdict );

  // transition end
  traceTransitionEnd();

  if (!verdict)
    testVerdict = TD_bad_verdict;

  if (second)
   {
    setTraceAccidental(old_value);
   }
}

/********************************************************************/

/*============================================================================*/
/* GO ALONG BACK OR PATH ARC CHECK WETHER NEW NODE IS EQUAL TO OLD ONE OR NOT */
/*============================================================================*/
void
go_to_known_node (void*    tag_node, /* node to which we should go */
                  FSM_Arc* arc       /* arc along which we go */
                 )
{
	Object * state;
#ifdef _DEBUG_REFCNT
  printf("*** start: go_to_known_node()\n");
#endif

  /* GO ALONG BACK OR PATH ARC */
  execute_FSM_Arc(arc, 1);

  /** IS THE RESULT OF PERFORMING BACK/PATH-OPERATION RIGHT? **/
  state = scenario_get_state(GD);
  if (compare_Object(state, r(tag_node)))
  {

	String* current_state,*buf, *tag_node_name;
	  state = scenario_get_state(GD);
	current_state = toString(state);
    tag_node_name = toString( r(tag_node) );

    traceExceptionInfo("The dfsm test engine can be used for deterministic scenarios only");
    buf = format_String("The last transition (id=%d) previously has led to: %s",
			arc->id, toCharArray_String(r(tag_node_name)) );
    traceExceptionInfo(toCharArray_String(r(buf)));
    destroy(tag_node_name);
    destroy(buf);

    buf = format_String("Now it leads to: %s", toCharArray_String(r(current_state)) );
    traceExceptionInfo(toCharArray_String(r(buf)));
    destroy(buf);
    destroy(current_state);

    traceException(NONDETERMINISTIC_GRAPH_MESSAGE);
    testVerdict = TD_faulted;
   }

#ifdef _DEBUG_REFCNT
  printf("*** end:   go_to_known_node()\n");
#endif

  return;
}
/****************************************************/

/*=================================================================*/
/** IS THE ARRIVED NODE NEW? TO KNOW THAT BROWSE ALL TESTED NODES **/
/*=================================================================*/
static
unsigned is_node_new(void* new_node)
{
unsigned i;

#ifdef _DEBUG_REFCNT
  printf("*** start: is_node_new()\n");
#endif
  for (i = 0;i < tested->len && compare(r(new_node),r(TESTED(i)->node));i++);
#ifdef _DEBUG_REFCNT
  printf("*** end:   is_node_new()\n");
#endif
  return i;
}
/****************************************************/

/*==============================================================*/
/**                GO ALONG BACK-ARCS TO PATH                  **/
/*==============================================================*/
int go_along_back_arcs(int tested_id)
{
int new_tested_id;

#ifdef _DEBUG_REFCNT
  printf("*** start: go_along_back_arc()\n");
#endif
  while (0 > TESTED(tested_id)->pnum) {
    if (0 > TESTED(tested_id)->bnum) {
      /** BACK-ARC IS NOT EXIST **/
		Object * state = scenario_get_state(GD);
      String* current_state = toString(state);
      String* buf;
	  StringBuffer *str_buf;
      void* untested_node;

      traceExceptionInfo("The dfsm test engine can be used only for scenarios with mutually connected state transition graph");
	  str_buf = create_StringBuffer();
	  append_StringBuffer(       r(str_buf), "Current state is: ");
	  appendString_StringBuffer( r(str_buf), current_state); // current_state destroyed
	  buf = toString(str_buf);
      traceExceptionInfo(toCharArray_String(r(buf)));
      destroy(buf);

      traceExceptionInfo("All the states reachable from it are completely tested");
      //
      untested_node = TESTED(PATH(0)->tnum)->node;
	  str_buf = create_StringBuffer();
      append_StringBuffer(       r(str_buf), "State '" );
	  appendString_StringBuffer( r(str_buf), toString(r(untested_node)) );
	  append_StringBuffer(       r(str_buf), "' may have untested transitions, but it is unreachable" );
	  buf = toString(str_buf);
      traceExceptionInfo(toCharArray_String(r(buf)));
      destroy(buf);

      traceException(UNCONNECTED_GRAPH_MESSAGE);

      no_last_transition = true;
      testVerdict = TD_faulted;
      return -1;
    }
    else { /** BACK-ARC EXISTS **/
      /** MOVEMENT ALONG BACK-ARC **/
      new_tested_id = TESTED(tested_id)->bnum;
      /** IS THE RESULT OF MOVEMENT ALONG BACK-ARC RIGHT? **/
      go_to_known_node(TESTED(new_tested_id)->node,
                       TESTED(tested_id)->barc
                      );
      if (  testVerdict == TD_faulted
         || (  tsFinishMode != UNTIL_END
            && (  testVerdict == ORACLE_bad_verdict
               || testVerdict == TD_bad_verdict
               )
            )
         ) {
        /** THE RESULT OF MOVEMENT ALONG BACK-ARC IS NOT RIGHT **/
        // trace_message = non_deterministic_message;
        testVerdict = TD_faulted;
        return -1;
      }
      else
        /** THE RESULT OF MOVEMENT ALONG BACK-ARC IS RIGHT **/
        tested_id = new_tested_id;
    } /** end if - back-arc exists **/
  } /** end while - go along back-arcs to path **/
#ifdef _DEBUG_REFCNT
  printf("*** end:   go_along_back_arc()\n");
#endif
  return tested_id;
}
/****************************************************/

/*=========================================================*/
/**            CORRECT BACK ARCS IF IT NEEDS              **/
/*=========================================================*/
void correct_back_arcs (int path_id, unsigned last_arc_end_tested_id)
{
  /** SOME NODE AT PATH WAS ARRIVED TO  **/
  /** THIS NODE INDEX IN PATH-LIST IS path_id **/
  /** new_back - FOR BACK-NODE OF path_id NODE **/
  /** ITS INDEX IN PATH LIST **/
  int i;
  int new_back = PATH(path_id)->nback;

  /** IS NEW_BACK LESS THAN INDEX OF BACK-NODE OF LAST NODE OF PATH? **/
  if (new_back < PATH(path->len - 1)->nback) {
    /** WE CORRECT BACK-ARCS AND BACK-NODE INDEXES FOR EACH **/
    /** NODE AT PATH-LIST WHERE POINT OF RETURNING TO PATH IS **/
    /** GREATER THEN new_back FROM END OF PATH TO path_id+1 **/
    for (i = path->len - 2; i > path_id && new_back < PATH(i)->nback; i-- ) {
      /** UPDATE nback IN pnode IN path(i) **/
      PATH(i)->nback = new_back;
      /** UPDATE tnode IN tested(path(i)->tnum) **/
      deleteFSM_Arc(TESTED(PATH(i)->tnum)->barc);
        TESTED(PATH(i)->tnum)->barc
      = PATH(i)->parc;
        addRef(TESTED(PATH(i)->tnum)->barc);
        TESTED(PATH(i)->tnum)->bnum
      = PATH(i + 1)->tnum;
    } /**end for - correct back arcs**/
    i = path->len - 1;
    /** UPDATE nback IN pnode IN path(i) **/
    PATH(i)->nback = new_back;
    /** UPDATE tnode IN tested(path(i)->tnum) **/
    deleteFSM_Arc(TESTED(PATH(i)->tnum)->barc);
      TESTED(PATH(i)->tnum)->barc
    = PATH(i)->parc;
      addRef(TESTED(PATH(i)->tnum)->barc);
      TESTED(PATH(i)->tnum)->bnum
    = last_arc_end_tested_id;
  }
}
/****************************************************/

/**-----------------------------------------------------------------------**/
/**                                                                       **/
/**                            ENGINE OF BROWSING                         **/
/**                                                                       **/
/**                      DETERMINICTIC MUTUALLY CONNECTED GRAPH           **/
/**                                                                       **/
/**-----------------------------------------------------------------------**/
void traverse()
{
  bool        need_back;              /** BACK IS true IF IT NEEDS TO GO BACK **/
  int         tested_id;              /** TESTED INDEX **/
  unsigned    last_arc_end_tested_id; /** TESTED INDEX OF THE LAST ARC END **/
  int         path_id;                /** PATH INDEX **/
  pnode*      last_pnode;             /** LAST NODE IN PATH LIST **/
  tnode*      last_tnode;             /** TESTED NODE CORRESPONDING TO LAST PATH NODE **/
  FSM_Arc*    new_arc;                /** NEW ARC **/
  void*       new_node;               /** NEW NODE **/

#ifdef _DEBUG_REFCNT
  printf("*** start: traverse()\n");
#endif

  /** INITIALIZE LIST OF TESTED STATES OF OBJECT **/
  tested = newList();
  new_node = scenario_get_state(GD);
  addToList(tested, new_tnode(new_node, 0, -1, 0));
  /** INITIALIZE LIST OF STATES OF OBJECT UNDER TESTING **/
  path = newList();
  addToList(path, new_pnode(0, 0, 0));
  /** MAIN CYCLE **/
  do
   {
    need_back  = false;
    last_pnode = PATH(path->len - 1);
    last_tnode = TESTED(last_pnode->tnum);
    new_arc    = get_next_arc (last_pnode->larc);
    if (new_arc)
	 {/** DOES UNTESTED ARC EXIST AT THE PATH END? **/
      /** YES - UNTESTED ARC EXISTS AT THE PATH END STORE THIS ARC **/

      deleteFSM_Arc (last_pnode->parc);
      last_pnode->parc = cloneFSM_Arc (new_arc);
      /** GO ALONG UNTESTED ARC **/
#ifdef _DEBUG_REFCNT
  printf("*** start: call(%d)\n",new_arc->id);
#endif
      execute_FSM_Arc(new_arc, 0);
#ifdef _DEBUG_REFCNT
  printf("*** end:   call(%d)\n",new_arc->id);
#endif

      if (new_arc != last_pnode->larc)
	   {deleteFSM_Arc (last_pnode->larc);
	    last_pnode->larc = new_arc;
	   }
      if (  testVerdict == TD_faulted
         || (  tsFinishMode != UNTIL_END
            && (   testVerdict == ORACLE_bad_verdict
               || testVerdict == TD_bad_verdict
               )
            )
         )
        goto finish;
      new_node = scenario_get_state(GD);
      if (compare(r(last_tnode->node), r(new_node)))
	   { /** THE ARC IS NOT A LOOP. IS THE ARRIVED NODE NEW? **/
        tested_id = last_arc_end_tested_id = is_node_new(new_node);
        if ( last_arc_end_tested_id < tested->len)
		 { /** THE ARRIVED NODE IS NOT NEW **/
          destroy(new_node);
          need_back = true;
         }
        else
	     { /** THE NODE IS NEW (UNTESTED). ADD IT TO PATH **/
          addToList(tested, new_tnode(new_node, 0, -1, path->len));
          addToList(path,
                    new_pnode(tested->len - 1,
                              0,
                              path->len
                             )
                   );
         } /** if - is the arrived node new? **/
       }
	  else
	   { /** THE ARC IS A LOOP. DESTROY new_node **/
        destroy(new_node);
	   } /** if - is the arc a loop?  **/
     }
    else
     { /** NO - UNTESTED ARC DOES NOT EXIST AT THE PATH END **/
       /** DELETE LAST NODE OF PATH **/
      tested_id = last_pnode->tnum;
      last_tnode->pnum = -1;
      deleteLastPathNode();
      need_back = true;
     }
    if (need_back && path->len > 0)
     { /** WE NEED COME BACK **/
       /** GO ALONG BACK-ARCS TO PATH **/
      tested_id = go_along_back_arcs(tested_id);
      if(tested_id < 0)
        goto finish;
      path_id = TESTED(tested_id)->pnum;
      /** CORRECT BACK-ARCS AND BACK-NODE INDEXES IF IT NEEDS **/
      correct_back_arcs(path_id, last_arc_end_tested_id);
      /** GO ALONG PATH FROM path_id TO THE END OF PATH **/
      for (; (unsigned)path_id < path->len - 1; path_id++)
	   { /** IS THE RESULT OF MOVEMENT ALONG PATH-ARC RIGHT? **/
        go_to_known_node(TESTED(PATH(path_id+1)->tnum)->node,
                         PATH(path_id)->parc
                        );
        if (  testVerdict == TD_faulted
           || (  tsFinishMode != UNTIL_END
              && (  testVerdict == ORACLE_bad_verdict
                 || testVerdict == TD_bad_verdict
                 )
              )
           ) /** THE RESULT OF MOVEMENT ALONG PATH-ARC IS NOT RIGHT **/
          goto finish;
       }
     } /** end if - need_back **/
   } while (path->len != 0);  /** MAIN CYCLE END - ALL NODES ARE TESTED **/
finish:
  if (!no_last_transition)
   {
    trace_current_state();
   }
  deleteTestedList();
  deletePathList();

#ifdef _DEBUG_REFCNT
  printf("*** end:   traverse()\n");
#endif

  return;
}

/****************************************************/
/*             initialization of engine             */
static void init_engine(void);
/*             invariant on engine state            */
static bool check_consistency(void);
/*             tracing engine properties            */
static void trace_dfsm_properties(void);


/*=============================================================*/
/*  Interface function. It initializes global variables by     */
/*  default values, parses standart parameters, calls          */
/*  manually developed function of initalization, calls        */
/*  function traverse() and finishes the test.                 */
/*=============================================================*/
bool start_dfsm (int argc, char** argv, dfsm* td)
{
bool tracing_was_started_here = false;
bool was_accidental_transition_traced;
AssertionExit prev_assert = NULL;
TestSystemParameters* pars;
bool verdict;
bool verdict_init;

#ifdef _DEBUG_REFCNT
  printf("*** start: start_dfsm()\n");
#endif

  assertion((GD = td) != 0, "NULL scenario data");

  // Initialize test engine data
  init_engine();

  // Process standard parameters
  pars = processStandardParameters(GD->name,"",GD->actions,argc,argv);
  assertion( pars != NULL, "start_ndfsm: standard parameters processing fails");

  // Start tracing if it has not been started yet
  if (!isInTrace())
   {
    tracing_was_started_here = true;
    prev_assert = ts_setup_assert_function();
    startTrace();
   }

  START_EXCEPTION_PROTECTED_SECTION(ts_default_exception_handler);

  setCurrentTestScenarioName(GD->name);
  traceScenarioStart(GD->name);
  traceScenarioParameters(pars->argc,(const char**)pars->argv);
  // Deferred Reactions Enabled
  traceTestEngineProperty( "Deferred Reactions Enabled", areDeferredReactionsEnabled() ? "true" : "false" );

  // Scenario initialization
  verdict = true;
  if (GD->init != NULL)
   {
    // Trace pseudo transition start
    was_accidental_transition_traced = setTraceAccidental(true);
    traceState("start");
    traceScenarioValue ("state", "", "", "start");
    traceTransitionStart("initialize");
    traceScenarioValue ("scenario method", "", "", "initialize");

    // Call 'init' function
    clearOracleVerdict();
    verdict_init = scenario_init(GD, pars->argc, pars->argv) ;
    verdict = verdict_init && getOracleVerdict();

    // Frame end
    verdict = onFrameEnd( &model_state_manager, verdict );

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
   {if (check_consistency())
     {
       trace_dfsm_properties();
       traverse();
     }
    if (GD->finish)
     {
      // Trace pseudo transition start
      was_accidental_transition_traced = setTraceAccidental(true);
      traceTransitionStart("finalize");
      traceScenarioValue ("scenario method", "", "", "finalize");

      // Call 'finish' function
      clearOracleVerdict();
      scenario_finish(GD) ;
      verdict = (testVerdict == TD_ok) && getOracleVerdict();

      // Frame end
      verdict = onFrameEnd( &model_state_manager, verdict );

      if (!verdict && (testVerdict == TD_ok))
        testVerdict = TD_bad_verdict;

      // Trace pseudo transition end
      traceTransitionEnd();
      traceState("end");
      traceScenarioValue ("state", "", "", "end");
      setTraceAccidental(was_accidental_transition_traced);
     }
   }

  traceScenarioEnd();
  setCurrentTestScenarioName(NULL);

  END_EXCEPTION_PROTECTED_SECTION();

  // Stop tracing if it was started here
  if (tracing_was_started_here)
   {
    endTrace();
    ts_restore_assert_function(prev_assert);
   }

  // Disable standard parameters
  disableStandardParameters(pars);

#ifdef _DEBUG_REFCNT
  printf("*** end:   start_dfsm()\n");
#endif

  return (testVerdict == TD_ok);
}


/********************************************************************/
/**                  initialization of engine                      **/
/********************************************************************/
static void init_engine(void)
{
  // init model_state_manager
  model_state_manager.saveModelState    = GD->saveModelState;
  model_state_manager.restoreModelState = GD->restoreModelState;
  model_state_manager.isStationaryState = GD->isStationaryState;
  model_state_manager.observeState      = GD->observeState;

  // init test verdict
  testVerdict = TD_ok;

  // Engine properties initialization
  // init finish mode
  setFinishMode(UNTIL_ERROR);

  // init transition id
  arc_id = 0;

  // init internal data
  no_last_transition = false;

  // init uninitialized fields of dfsm structure
  // init getState field
  if (GD->getState == NULL)
    GD->getState = singletonState;

  // init DR mode and Time model
  if ((GD->isStationaryState != NULL) &&
      (GD->saveModelState != NULL)    &&
      (GD->restoreModelState != NULL)
     )
   {setDeferredReactionsMode(true);
    setTSTimeModel(LinearTSTime);
   }
  else
   {setDeferredReactionsMode(false);
    setTSTimeModel(NotUseTSTime);
   }

  // init WTime
  setWTime(0);

  // init Find First Series Only
  setFindFirstSeriesOnly(false);
}


/********************************************************************/
/**                  invariant on engine state                     **/
/********************************************************************/
static bool check_consistency(void)
{
  if (areDeferredReactionsEnabled())
   {if (GD->isStationaryState == NULL)
     {traceException( "is_stationary field should be initialized if deferred reactions are enabled" );
      return false;
     }
    if (GD->saveModelState == NULL)
     {traceException( "save_model_state field should be initialized if deferred reactions are enabled" );
      return false;
     }
    if (GD->restoreModelState == NULL)
     {traceException( "restore_model_state field should be initialized if deferred reactions are enabled" );
      return false;
     }
   }
  return true;
}

/********************************************************************/
/**                  tracing engine properties                     **/
/********************************************************************/
static void trace_dfsm_properties(void)
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
      traceTestEngineProperty( "Finish Mode", "until error" );
      break;
    case UNTIL_END :
      traceTestEngineProperty( "Finish Mode", "until end" );
      break;
    default :
      assertion( false, "Unexpected Finish Mode" );
   }
}
