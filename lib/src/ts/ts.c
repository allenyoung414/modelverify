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
 * FILE:        ts.c
 * AUTHOR:      Vitaliy Omelchenko (vitaliy)
 * CREATION:    2 Aug 2001
 * DESCRIPTION: Implementation of functions
 *              that needs for all CTesK tests.
 * VERSION AND LIST OF CHANGES:
 *  1 Jul 2003  (hed)
 *   Support of deferred reactions has been added.
 */

#include <ts/ts.h>
#include <c_tracer/c_tracer.h>


/********************************************************************/
/**                     CTesK Version Info                         **/
/********************************************************************/

// Run 'ant subst' to generate ts/version.ch
#include "ts/version.ch"

const char* ts_get_ctesk_version(void)
{
  return CTesK_version;
}

const char* ts_get_ctesk_build(void)
{
  return CTesK_build;
}


/********************************************************************/
/**                        Set Bad Verdict                         **/
/********************************************************************/
bool _ts_mediator_verdict = true;

void setBadVerdict(const char *msg)
{
  ts_fire_verdict_event( TS_VERDICT_MEDIATOR, false );

  if (_ts_mediator_verdict) {
    if (msg != NULL) {
      traceExceptionInfo(msg);
	}
    ts_trace_bad_mediator_verdict();
  } else {
    if (msg != NULL) {
      traceSystemInfo(msg);
	}
  }
  _ts_mediator_verdict = false;
}

bool isBadVerdict(void)
{
  return !_ts_mediator_verdict;
}

void clearBadVerdict(void)
{
  _ts_mediator_verdict = true;
}

/********************************************************************/
/**                        Verdict Event Handling                  **/
/********************************************************************/

static VerdictEventHandler verdictEventHandlers[4];

void ts_set_verdict_event_handler( enum TS_Verdict ts_verdict_flags
                                 , VerdictEventHandler handler
                                 )
{
    if( ts_verdict_flags & TS_VERDICT_PRE      ) verdictEventHandlers[0] = handler;
    if( ts_verdict_flags & TS_VERDICT_POST     ) verdictEventHandlers[1] = handler;
    if( ts_verdict_flags & TS_VERDICT_MEDIATOR ) verdictEventHandlers[2] = handler;
    if( ts_verdict_flags & TS_VERDICT_SCENARIO ) verdictEventHandlers[3] = handler;
}

void ts_fire_verdict_event( enum TS_Verdict ts_verdict_flag, bool verdict )
{
    switch( ts_verdict_flag )
    {
    case TS_VERDICT_PRE:
        if( verdictEventHandlers[0] != NULL ) verdictEventHandlers[0]( verdict );
        break;
    case TS_VERDICT_POST:
        if( verdictEventHandlers[1] != NULL ) verdictEventHandlers[1]( verdict );
        break;
    case TS_VERDICT_MEDIATOR:
        if( verdictEventHandlers[2] != NULL ) verdictEventHandlers[2]( verdict );
        break;
    case TS_VERDICT_SCENARIO:
        if( verdictEventHandlers[3] != NULL ) verdictEventHandlers[3]( verdict );
        break;
    default:
        assertion( false, "Unexpected TS_VERDICT_FLAG" );
    }
}
