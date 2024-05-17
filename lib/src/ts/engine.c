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
 * FILE:        engine.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    18 April 2005
 * DESCRIPTION: Common properties of test engines.
 */

#include <ts/scenario.h>
#include <ts/engine_tsi.h>
#include <ts/serialization.h>
#include <ts/system.h>
#include <c_tracer/c_tracer.h>
#include <utils/assertion.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <ts/cpp_iface.h>

/* CTesK Source Start */
#if defined(WIN32)
  #include <io.h>
#endif
/* CTesK Source End */


/********************************************************************/
/**                         Test Verdict                           **/
/********************************************************************/
/*==============================================================*/
/*  Global variable defining test mode:                         */
/*  TD_ok - test succesfull finished;                           */
/*  TD_bad_parameters - bad parameters are got to the test,     */
/*                      they may be standart parameters or      */
/*                      defined by test developer;              */
/*  TD_not_enough_resourses - not enough resources for the test */
/*                            (malloc or calloc returned NULL); */
/*  TD_bad_verdict - after stimulus call and waiting for the    */
/*                   specified time stationary state was not    */
/*                   achieved;                                  */
/*  TD_faulted - during serialization repeated reaction oracle  */
/*               call return bad verdict, or                    */
/*               empty arc was got in stimulus call, or         */
/*               non-deterministic behaviour was found when     */
/*               calling back arc (i.e. calling back arc was    */
/*               result in the test state different from one    */
/*               after calling the same arc from the same test  */
/*               state), or                                     */
/*               when trying go along back arcs to path there   */
/*               is not back arc in tested node;                */
/*  ORACLE_bad_verdict - oracle of some stimulus returned bad   */
/*                       verdict or serialization of reactions  */
/*                       not found;                             */
/*==============================================================*/
TestVerdict testVerdict = TD_ok;


/********************************************************************/
/**                     Finish Mode property                       **/
/********************************************************************/
FinishMode tsFinishMode = UNTIL_ERROR;
int tsNumberOfErrorsToFinish = 20;

FinishMode setFinishMode(FinishMode finish_mode)
{
FinishMode old_finish_mode;

  old_finish_mode = tsFinishMode;
  tsFinishMode = finish_mode;
  if (finish_mode == UNTIL_ERROR)
    tsNumberOfErrorsToFinish = 1;
  else
    tsNumberOfErrorsToFinish = 0;
  return old_finish_mode;
}

FinishMode getFinishMode()
{
  return tsFinishMode;
}

int setNumberOfErrorsToFinish(int error_number)
{
int old_error_number;

  old_error_number = tsNumberOfErrorsToFinish;
  if (error_number <= 0)
   {
    tsNumberOfErrorsToFinish = 0;
    tsFinishMode = UNTIL_END;
   }
  else
   {
    tsNumberOfErrorsToFinish = error_number;
    tsFinishMode = UNTIL_ERROR;
   }
  return old_error_number;
}


/********************************************************************/
/**                       Test System Mode                         **/
/********************************************************************/
static TSMode tsMode = SequentialTSMode;

TSMode getTSMode()
{
  return tsMode;
}

void setTSMode(TSMode mode)
{
  tsMode = mode;
}


/********************************************************************/
/**                   Deferred Reactions property                  **/
/********************************************************************/
bool setDeferredReactionsMode(bool enable)
{
bool prev_value;

  prev_value = areDeferredReactionsEnabled();
  if (enable)
  {
    setTSMode(ParallelTSMode);
  } else {
    setTSMode(SequentialTSMode);
  }
  return prev_value;
}

bool areDeferredReactionsEnabled()
{
  return (tsMode == ParallelTSMode);
}


/********************************************************************/
/**                        WTime property                          **/
/********************************************************************/
static time_t WTime = 0;

time_t setWTime(time_t secs)
{
time_t oldWTime;

  oldWTime = WTime;
  WTime = 1000*secs;
  return oldWTime/1000;
}

time_t setWTimeMSec(time_t secs)
{
time_t oldWTime;

  oldWTime = WTime;
  WTime = secs;
  return oldWTime;
}

time_t getWTime()
{
  return WTime/1000;
}

time_t getWTimeMSec()
{
  return WTime;
}


/********************************************************************/
/**               Find First Series Only property                  **/
/********************************************************************/
//static bool build_one_series_only = false;
static bool build_one_series_only_bound = 0;

bool setFindFirstSeriesOnly(bool new_value)
{
bool old_value;

  old_value = (build_one_series_only_bound > 0);
  build_one_series_only_bound = new_value ? 1 : 0;
  return old_value;
}

bool isFindFirstSeriesOnly(void)
{
  return (build_one_series_only_bound > 0);
}

int setFindFirstSeriesOnlyBound(int bound)
{
bool old_value;

  old_value = build_one_series_only_bound;
  build_one_series_only_bound = (bound > 0) ? bound : 0;
  return old_value;
}

int getFindFirstSeriesOnlyBound(void)
{
  return build_one_series_only_bound;
}

/********************************************************************/
/**                   Disabled Actions property                    **/
/********************************************************************/
static char** disabled_actions= NULL;

static
void resetDisabledActions(void)
{
  int i;

  if (disabled_actions != NULL)
  {
    for (i=0; disabled_actions[i]!=NULL; i++)
    {
      free(disabled_actions[i]);
    }
    free(disabled_actions);
  }
  disabled_actions = NULL;
}

static
char* readDisabledAction(FILE * file)
{
#define MAX_SPEC_NAME_LENGTH 256
  static int buff_length = MAX_SPEC_NAME_LENGTH + 2; // 1 for \n, one for NULL
  static char buffer[MAX_SPEC_NAME_LENGTH + 2];
  char * res;
  int name_length;
  memset( buffer, 0, buff_length );
  res = fgets( buffer, buff_length, file );
  if( res == NULL )
    return NULL;
  name_length = strlen( buffer );
  buffer[ name_length - 1 ] = '\0';
  name_length = strlen( buffer );
  printf( "disabled action name length : %d\n", name_length );
  res = malloc( sizeof( char ) * name_length );
  memcpy( res, buffer, name_length );
  printf( "disabled actions read: %s\n", res );
  return res;
#undef MAX_SPEC_NAME_LENGTH
}

/*
// hed's implementation
static
char* readDisabledAction(int handle)
{
#define BUFFER_SIZE 1024
  static char buffer[BUFFER_SIZE+1];
  static int pointer = 0;
  static int size = 0;
  int i;
  char* res= NULL;
  int res2;

  if ((size == 0) || (pointer >= size))
  {
    size = read(handle, buffer, BUFFER_SIZE);
    if (size <= 0)
    {
      size = 0;
      return NULL;
    }
    pointer = 0;
  }
  for (i=pointer; i<size; i++)
  {
    if ((buffer[i]==0x0A) || (buffer[i]==0x0D))
    {
      // skip EOLNs
      if (i == pointer)
      {
        pointer++;
        continue;
      }
      buffer[i] = 0;
      res = strdup( &(buffer[pointer]));
      pointer = i + 1;
      return res;
    }
  }
  if (size < BUFFER_SIZE)
  {
    if (i == pointer)
    {
      size = 0;
      return NULL;
    }
    buffer[size] = 0;
    res = strdup( &(buffer[pointer]));
    size = 0;
    pointer = 0;
    return res;
  }
  if (pointer == 0)
  {
    // too long name
    size = 0;
    return NULL;
  }
  memmove(buffer, buffer+pointer, BUFFER_SIZE-pointer);
  res2 = read(handle, buffer+BUFFER_SIZE-pointer, pointer);
  if (res2 < 0)
  {
    buffer[BUFFER_SIZE-pointer] = 0;
    res = strdup(buffer);
    size = 0;
    pointer = 0;
    return res;
  }
  i = BUFFER_SIZE - pointer;
  size = BUFFER_SIZE - pointer + res2;
  pointer = 0;
  for (; i<size; i++)
  {
    // skip EOLNs
    if (i == pointer)
    {
      pointer++;
      continue;
    }
    if ((buffer[i]==0x0A) || (buffer[i]==0x0D))
    {
      buffer[i] = 0;
      res = strdup( &(buffer[pointer]));
      pointer = i + 1;
      return res;
    }
  }
  if ((i != pointer) && (size < BUFFER_SIZE))
  {
    buffer[size] = 0;
    size = 0;  name_length = strlen( buffer );

    return strdup( &(buffer[pointer]));
  }
  // too long name
  size = 0;
  return NULL;
#undef BUFFER_SIZE
}*/

static
bool isActionDisabled(const char* action)
{
  int i;

  if (disabled_actions == NULL)
    return false;
  for (i=0; disabled_actions[i]!=NULL; i++)
  {
    if (strcmp(disabled_actions[i], action) == 0)
      return true;
  }
  return false;
}

static
void setupDisabledActions(const char* filename, ScenarioFunctionID * scenario_actions )
{
  FILE *file;
  char* action;
  int disabled_actions_size;
  int i;

  // Reset state
  resetDisabledActions();
  // Open file
  file = fopen(filename, "r");
  if (file == NULL)
    return;
  // Allocate memory
  disabled_actions_size = 2569;
  disabled_actions = malloc(disabled_actions_size);
  if (disabled_actions == NULL)
    return;
  // Read from file
  for (i=0;; i++)
  {
    action = readDisabledAction(file);
    //printf("disabled action[%d,%d]: %s\n",i,disabled_actions_size,action!=NULL?action:"<NULL>");
    if (i >= disabled_actions_size)
    {
      disabled_actions_size = disabled_actions_size + 256;
      disabled_actions = realloc(disabled_actions, disabled_actions_size);
      if (disabled_actions == NULL)
        return;
    }
    disabled_actions[i] = action;
    if (action == NULL)
    {
      break;
    }
  }
  // Close file
  fclose(file);

  // Filter disabled actions
  {
    int i, j;
    for (i=0; scenario_actions[i]!=NULL;)
    {
      if (!isActionDisabled(scenario_actions[i]->name))
      {
        i++;
        continue;
      }
      printf( "action disabled: %s\n", action_name(scenario_actions[i]) );
      for (j=i; scenario_actions[j]!=NULL; j++)
      {
        scenario_actions[j] = scenario_actions[j+1];
      }
    }
  }
}

/********************************************************************/
/**               Test System Assertion Exit Function              **/
/********************************************************************/
void ts_assert_function(const char* format, va_list arg_list)
{
String *buff = vformat_String( format, arg_list );

  traceInternalError( toCharArray_String(r(buff)) );
//  destroy( buff );
  closeTrace();
  exit(1);
}

AssertionExit ts_setup_assert_function(void)
{
  if (isCustomAssertionExit())
   {
    return NULL;
   }
  return atAssertion(ts_assert_function);
}

void ts_restore_assert_function(AssertionExit prev_assert)
{
  if (prev_assert != NULL)
   {
    atAssertion(prev_assert);
   }
}


/********************************************************************/
/**                      Calculate TT filename                     **/
/********************************************************************/
/*                      YYYY-MM-DD--HH-MM-SS.NNN */
#define MAX_TIME_STRLEN 5    +3 +2+2+3 +3 +3 +3 +1

char* format_time(void)
{
static char buf[MAX_TIME_STRLEN];
  time_t tt;
  struct tm *tm;
  time(&tt);
  tm = localtime(&tt);
  strftime(buf, MAX_TIME_STRLEN, "%Y-%m-%d_%H-%M-%S", tm);
  return buf;
}

// TODO: Improve implementation
bool file_exists(const char* name)
{
int handle;

  handle = open(name,O_EXCL | O_RDONLY);
  if (handle == -1)
    return false;
  close(handle);
  return true;
}

char* getTTFileName(const char *basename, const char *prefix, const char *extension)
{
int j;
char* timemark = (char*)format_time();
char* ttfilename;

  ttfilename = malloc(strlen(basename) + strlen(prefix) + MAX_TIME_STRLEN + 16);
  assertion(ttfilename != NULL, "generateTTFileName: not enough memory");

  sprintf(ttfilename, "%s_%s_%s.%s", basename, prefix, timemark, extension);

  for(j = 1; j < 1000; j++)
  {
    if(!file_exists(ttfilename))
      { break; }

    sprintf(ttfilename, "%s_%s_%s.%03d.%s", basename, prefix, timemark, j, extension);
  }

  return ttfilename;
}

/********************************************************************/
/**                   Process Standard Parameters                  **/
/********************************************************************/
TestSystemParameters* processStandardParameters( const char* name
                                               , const char* prefix
                                               , ScenarioFunctionID* scenario_actions
                                               , int argc
                                               , char** argv
                                               )
{
TestSystemParameters* res;
int i,numFiles;
char* ttfilename = NULL;

  res = malloc( sizeof(TestSystemParameters) );
  assertion( res != NULL, "processStandardParameters: not enough memory" );

  res->no_trace = false;
  res->console = false;
  res->argv = calloc( argc+1, sizeof(char*) );
  assertion( res->argv != NULL, "processStandardParameters: not enough memory" );

  res->argc = 0;
  if (argc > 0)
   {res->argv[0] = argv[0];
    res->argc++;
   }

  numFiles = 0;
  res->files   = calloc( argc+1, sizeof(char*) );
  assertion( res->files != NULL, "processStandardParameters: not enough memory" );

  for(i = 1; i < argc; i++)
   {
    if(strcmp("-t", argv[i]) == 0)
    {
      assertion( !res->no_trace, "Bad parameters: -t and -nt options are mutually exclusive");
      assertion(++i < argc, "Bad parameter: after -t should be destination of trace log");
      assertion(addTraceToFile(argv[i]), "Can't trace to file %s", argv[i]);
      res->files[numFiles] = strdup(argv[i]);
      assertion( res->files[numFiles] != NULL, "processStandardParameters: not enough memory" );
      numFiles++;
    }
    else if(strcmp("-tc", argv[i]) == 0)
    {
      assertion( !res->no_trace, "Bad parameters: -tc and -nt options are mutually exclusive");
      if (!res->console)
       {
        addTraceToConsole();
       }
      res->console = true;
    }
    else if(strcmp("-tt", argv[i]) == 0)
    {
      assertion( !res->no_trace, "Bad parameters: -tt and -nt options are mutually exclusive");
      if ((name != NULL) && (ttfilename == NULL))
       {
        ttfilename = getTTFileName(name, prefix, "utt");
        assertion(addTraceToFile(ttfilename), "Can't trace to file %s", ttfilename);
		if (res->argc <= numFiles) {
			res->files = realloc(res->files, (numFiles+2)*sizeof(res->files[0]));
		}
        res->files[numFiles] = ttfilename;
        numFiles++;
		res->files[numFiles] = NULL;
       }
    }
    else if ((strcmp("-nt", argv[i]) == 0) || (strcmp("--no-trace", argv[i]) == 0))
    {
      res->no_trace = true;
      assertion( !res->console, "Bad parameters: -tc and -nt options are mutually exclusive");
      assertion( ttfilename == NULL, "Bad parameters: -tt and -nt options are mutually exclusive");
      assertion( numFiles == 0, "Bad parameters: -t and -nt options are mutually exclusive");
    }
    else if(strcmp("-uerr", argv[i]) == 0)
    {
      setFinishMode(UNTIL_ERROR);
    }
    else if(strcmp("-uend", argv[i]) == 0)
	{
      setFinishMode(UNTIL_END);
    }
    else if(strncmp("-uerr=", argv[i], 6) == 0)
	{
      int value = atoi( argv[i] + 6 );
      assertion( value > 0, "Bad parameter [%s]: -uerr=<positive_number> expected", argv[i] );
      setNumberOfErrorsToFinish(value);
    }
    else if(strcmp("--trace-accidental", argv[i]) == 0)
    {
      setTraceAccidental(true);
    }
    else if ((strcmp("-ffso", argv[i]) == 0) || (strcmp("--find-first-series-only", argv[i]) == 0))
    {
      setFindFirstSeriesOnly(true);
    }
    else if (strncmp("--disabled-actions=", argv[i], 19) == 0)
    {
      if( scenario_actions != NULL )
       setupDisabledActions( argv[i] + 19, scenario_actions );
	}
	else if (strncmp("--trace-format=", argv[i], 15) == 0)
	{
	  if (strcmp("xml", argv[i]+15) == 0)
        setTraceDataFormatXML();
	  else if (strcmp("string", argv[i]+15) == 0)
		setTraceDataFormatString();
	  else assertion( false, "Bad parameter [%s]: --trace-format=<xml|string> expected", argv[i] );
	}
    else
     {// the rest arguments are transmitted to user
      for(; i < argc; i++)
       {res->argv[res->argc] = argv[i];
        res->argc++;
       }
      break;
     }
   }
  res->argv[res->argc] = NULL;

  // Default trace write: according '-tt' option
  if (!res->no_trace && !isTraceEnabled() && (name != NULL))
   {
    ttfilename = getTTFileName(name, prefix, "utt");
    assertion(addTraceToFile(ttfilename), "Can't trace to file %s", ttfilename);
	if (res->argc <= numFiles) res->files = realloc(res->files, (numFiles+2)*sizeof(res->files[0]));
    res->files[numFiles] = ttfilename;
    numFiles++;
	res->files[numFiles] = 0;
   }

  return res;
}

void disableStandardParameters(TestSystemParameters* pars)
{
int i;

  assertion( pars!=NULL, "disableStandardParameters: pars is NULL" );

  if (pars->console)
    removeTraceToConsole();
  for( i=0; pars->files[i]!=NULL; i++ )
   {
    removeTraceToFile(pars->files[i]);
    free(pars->files[i]);
   }
  free(pars->files);
  free(pars->argv);
  free(pars);
  resetDisabledActions();
}


/********************************************************************/
/**                   Start and Stop Test System                   **/
/********************************************************************/
static TestSystemParameters* ts_instance = NULL;
static AssertionExit prev_assert = NULL;

bool startTestSystem(int argc, char** argv)
{
TestSystemParameters* pars;

  if (ts_instance != NULL)
   {
    traceException("Double invocation of startTestSystem detected");
    return false;
   }
  pars = processStandardParameters(NULL,NULL,NULL,argc,argv);
  if (ts_instance != NULL)
   {
    traceException("Double invocation of startTestSystem detected");
    disableStandardParameters(pars);
    return false;
   }
  ts_instance = pars;
  prev_assert = ts_setup_assert_function();
  startTrace();
  return true;
}

void stopTestSystem(void)
{
TestSystemParameters* pars;

  if (!isInScenario())
   {
    pars = ts_instance;
    ts_instance = NULL;
    if (pars != NULL)
     {
      disableStandardParameters(pars);
      endTrace();
      ts_restore_assert_function(prev_assert);
     }
   }
}


/********************************************************************/
/**                     Current Test Scenario                      **/
/********************************************************************/
static char* ts_current_scenario_name = NULL;

void setCurrentTestScenarioName(const char* name)
{
  if (ts_current_scenario_name != NULL)
   {
    free(ts_current_scenario_name);
   }
  if (name == NULL)
   {
   	ts_current_scenario_name = NULL;
   }
  else
   {
    ts_current_scenario_name = strdup(name);
    assertion( ts_current_scenario_name!=NULL, "setCurrentTestScenarioName: not enough memory" );
   }
}

const char* getCurrentTestScenarioName(void)
{
  return ts_current_scenario_name;
}


/********************************************************************/
/**                      Model State Syncronizer                   **/
/********************************************************************/
bool onFrameEnd( ModelStateManager* model_state, bool verdict )
{
  if (!areDeferredReactionsEnabled())
    return verdict;
  if (!verdict)
   {// scenario function or mediator failed
    clearGatheredInteractions();
    return false;
   }
  return syncronize_ModelState( model_state );
}

bool syncronize_ModelState( ModelStateManager* model_state )
{
bool verdict;
Interactions* interactions;

  // Sleep to WTime
  sleep_msec(getWTimeMSec());

  // Observe state on the assumption that the system under test is in a stationary state
  if (model_state->observeState != NULL)
    model_state_observe(model_state);

  // Gather reactions
  interactions = getGatheredInteractions();
  traceAlienInteractions(interactions);
  if (!interactions->is_correct)
   {
    // Incorrect set of interactions
    traceExceptionInfo(interactions->defect_description);
    traceException(INCORRECT_SET_OF_INTERACTIONS_MESSAGE);
    verdict = false;
   }
  else
   {
    // Serialize
    verdict = serialize_interactions( interactions, model_state );
   }

  // Deallocate resources
  destroyInteractions( interactions );

  return verdict;
}


/********************************************************************/
/**                           Arc Symbol                           **/
/********************************************************************/
void init_ArcSymbol( ArcSymbol* symbol )
{
  symbol->kind  = -1;
  symbol->value = NULL;
}

void destroy_ArcSymbol( ArcSymbol* symbol )
{
  destroy( symbol->value );
}

bool isNull_ArcSymbol( ArcSymbol* symbol )
{
  return (symbol->kind == -1);
}


/********************************************************************/
/**                        Arc Symbol Executor                     **/
/********************************************************************/
void initArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* symbol )
{
  if (executor->actions[0] == NULL)
   {/* there are no actions */
    symbol->kind  = -1;
    symbol->value = NULL;
    return;
   }
  symbol->kind  = 0;
  symbol->value = action_init(executor->actions[0]);
}

void nextArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* next_symbol, int kind, Object* value )
{
  if (action_has_next(executor->actions[kind], r(value)))
   {
    next_symbol->kind  = kind;
    next_symbol->value = value;
   }
  else
   {
    destroy(value);
    kind++;
    if (executor->actions[kind] != NULL)
     {
      next_symbol->kind  = kind;
      next_symbol->value = action_init(executor->actions[kind]);
     }
    else
     {
      next_symbol->kind  = -1;
      next_symbol->value = NULL;
     }
   }
}

/**
// Commented because has_next() can be called after call() only
// Unconditional initialization is used now
void initArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* symbol )
{
int     kind;
Object* value;

  for( kind = 0; executor->actions[kind] != NULL; kind++ )
   {
    value = executor->actions[kind]->init();
    if (executor->actions[kind]->has_next(r(value)))
      break;
    destroy(value);
   }

  if (executor->actions[kind] == NULL)
   { // there are no actions
    symbol->kind  = -1;
    symbol->value = NULL;
    return;
   }
  symbol->kind  = kind;
  symbol->value = value;
}

void nextArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* next_symbol, int kind, Object* value )
{
  if (executor->actions[kind]->has_next(r(value)))
   {
    next_symbol->kind  = kind;
    next_symbol->value = value;
   }
  else
   {
    destroy(value);
    for( ; executor->actions[kind] != NULL; kind++ )
     {
      value = executor->actions[kind]->init();
      if (executor->actions[kind]->has_next(r(value)))
        break;
      destroy(value);
     }
    if (executor->actions[kind] != NULL)
     {
      next_symbol->kind  = kind;
      next_symbol->value = value;
     }
    else
     {
      next_symbol->kind  = -1;
      next_symbol->value = NULL;
     }
   }
}
**/

bool executeArcSymbol( ArcSymbolExecutor* executor, int arcid, ArcSymbol* symbol, ArcSymbol* next_symbol )
{
Object* value;
bool verdict;
char buff[16];

  if (isNull_ArcSymbol(symbol))
   {
    return true;
   }

  // Trace transition start
  sprintf( buff, "%d", arcid );
  traceTransitionStart( buff );

  traceScenarioValue( "scenario method", "", "", action_name(executor->actions[symbol->kind]) );

  // Perform call
  value = clone_Object(r(symbol->value));
  verdict = action_invoke(executor->actions[symbol->kind], (next_symbol != NULL) ? r(value) : value );

  // Frame end
  verdict = onFrameEnd( &executor->model_state, verdict );

  // Trace transition end
  traceTransitionEnd();

  // Evaluate the next symbol
  if (next_symbol != NULL)
   {
    nextArcSymbol( executor, next_symbol, symbol->kind, value );
   }

  return verdict;
}

