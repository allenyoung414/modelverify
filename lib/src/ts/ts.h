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
 * FILE:        ts.h
 * AUTHOR:      Vitaliy Omelchenko (vitaliy)
 * CREATION:    2 Aug 2001
 * DESCRIPTION: Contains data types and function
 *              declarations needed for all
 *              CTesK tests.
 * SCOPE:       It is an interface header.
 * VERSION AND LIST OF CHANGES:
 *  1 Jul 2003  (hed)
 *   Support of deferred reactions has been added
 */

#ifndef TS_H
#define TS_H

#ifdef __SEC__
#pragma SEC force_use_start
#endif

#include <utils/sec.h>

#include <string.h>

#include <utils/boolean.h>
#include <utils/assertion.h>
#include <ts/generated.h>

#include <ts/register.h>
#include <ts/engine.h>
#include <ts/dfsm.h>
#include <ts/ndfsm.h>

#include <atl/object.h>
#include <atl/unit.h>
#include <atl/string.h>

#include <c_tracer/c_tracer.h>

#include <ts/coverage.h>

/********************************************************************/
/**                     CTesK Version Info                         **/
/********************************************************************/

// Run 'ant subst' to generate ts/version.hh
#include <ts/version.hh>

#ifdef __cplusplus
extern "C" {
#endif

const char* ts_get_ctesk_version(void);
const char* ts_get_ctesk_build(void);


/********************************************************************/
/**                        Set Bad Verdict                         **/
/********************************************************************/
void setBadVerdict(const char *msg);
bool isBadVerdict(void);
void clearBadVerdict(void);

/********************************************************************/
/**                        Verdict Event Handling                  **/
/********************************************************************/

enum TS_Verdict
{
  TS_VERDICT_PRE = 1,
  TS_VERDICT_POST = 2,
  TS_VERDICT_MEDIATOR = 4,
  TS_VERDICT_SCENARIO = 8,
  TS_VERDICT_ALL = 15
};

typedef void (*VerdictEventHandler)( bool verdict );

void ts_set_verdict_event_handler( enum TS_Verdict ts_verdict_flags, VerdictEventHandler handler );
void ts_fire_verdict_event( enum TS_Verdict ts_verdict_flag, bool verdict );

#ifdef __SEC__
#pragma SEC force_use_end
#endif

#ifdef __cplusplus
}
#endif

#endif

