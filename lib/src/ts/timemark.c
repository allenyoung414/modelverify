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
 * FILE:        timemark.c
 * AUTHOR:      Alexey V. Khoroshilov
 * CREATION:    1 Jul 2003
 * DESCRIPTION: Implementation of functions that
 *              needs for work with timemarks.
 */

#include <ts/timemark.h>
#include <atl/map.h>
#include <atl/string.h>
#include <atl/integer.h>
#include <utils/assertion.h>
#include <time.h>


/********************************************************************/
/**                         INT64 Formatting                       **/
/********************************************************************/
/* CTesK Source Start */
#ifdef _MSC_VER
  #define I64FORMAT  "%I64d"
  #define UI64FORMAT "%I64u"
#else
/* CTesK Source End */
  #define I64FORMAT  "%lld"
  #define UI64FORMAT "%llu"
/* CTesK Source Start */
#endif
/* CTesK Source End */


/********************************************************************/
/**                            ChannelID                           **/
/********************************************************************/
#define WRONG_CHANNEL_ID   -2
#define UNIQUE_CHANNEL_ID  -1

const ChannelID WrongChannel  = WRONG_CHANNEL_ID;
const ChannelID UniqueChannel = UNIQUE_CHANNEL_ID;


ChannelID nextChannelID = 0;

ChannelID getChannelID()
{
  return nextChannelID++;
}

void releaseChannelID(ChannelID chid)
{
}


/********************************************************************/
/**                     Time Frame of Reference                    **/
/********************************************************************/
const TimeFrameOfReferenceID infiniteTimeFrameOfReferenceID = -1;
const TimeFrameOfReferenceID systemTimeFrameOfReferenceID = 0;

/* String -m-> Integer */
static Map* timeFrameOfReferenceMap = NULL;
static int timeFrameOfReferenceCounter = 0;

TimeFrameOfReferenceID getTimeFrameOfReferenceID(const char* name)
{
  if (name == NULL)
   {timeFrameOfReferenceCounter++;
    assertion( timeFrameOfReferenceCounter>0, "getTimeFrameOfReferenceID: number of time frames is exceeded" );
    return timeFrameOfReferenceCounter;
   }

  if (timeFrameOfReferenceMap == NULL)
    timeFrameOfReferenceMap = create_Map(&type_String,&type_Integer);
  if (containsKey_Map( r(timeFrameOfReferenceMap), create_String(name) ))
    return value_Integer( get_Map( r(timeFrameOfReferenceMap), create_String(name) ) );
  timeFrameOfReferenceCounter++;
  assertion( timeFrameOfReferenceCounter>0, "getTimeFrameOfReferenceID: number of time frames is exceeded" );
  destroy( put_Map( r(timeFrameOfReferenceMap),
                    create_String(name),
                    create_Integer(timeFrameOfReferenceCounter)
                  ) );
  return timeFrameOfReferenceCounter;
}

bool setSystemTimeFrameOfReferenceName(const char* name)
{
String* old_name;

  assertion( name!=NULL, "getTimeFrameOfReferenceID: name is NULL" );

  if (timeFrameOfReferenceMap == NULL)
    timeFrameOfReferenceMap = create_Map(&type_String,&type_Integer);
  old_name = (String*)put_Map( r(timeFrameOfReferenceMap), 
                               create_String(name),
                               create_Integer(systemTimeFrameOfReferenceID) 
                             );
  if (old_name != NULL)
   {destroy( old_name );
    return false;
   }
  return true;
}


/********************************************************************/
/**                            Time Mark                           **/
/********************************************************************/
/* -1 is infiniteTimeFrameOfReferenceID */
#define MIN_TIME_MARK 0x0000000000000000
#define MAX_TIME_MARK 0xffffffffffffffff

const TimeMark minTimeMark = { -1, MIN_TIME_MARK };
const TimeMark maxTimeMark = { -1, MAX_TIME_MARK };


TimeMark createTimeMark(LinearTimeMark timemark)
{
TimeMark res;
 
  res.frame = systemTimeFrameOfReferenceID;
  res.timemark = timemark;
  return res;
}

TimeMark createDistributedTimeMark(TimeFrameOfReferenceID frame,LinearTimeMark timemark)
{
TimeMark res;
 
  res.frame = frame;
  res.timemark = timemark;
  return res;
}


/********************************************************************/
/**                     Set Time Dependencies                      **/
/********************************************************************/
void setTimeMarkDependence(TimeMark former,TimeMark latter)
{
  /** Not yet implemented **/
  /** Should register the dependence between timemarks **/
  /** It is required to define an arbitrary partial order upon interactions **/
}


/********************************************************************/
/**                       Compare Time Marks                       **/
/********************************************************************/
CompareResult compareTimeMarks(TimeMark tm1,TimeMark tm2)
{
  if ((tm1.frame == infiniteTimeFrameOfReferenceID) || (tm2.frame == infiniteTimeFrameOfReferenceID))
   {if ((tm1.frame == infiniteTimeFrameOfReferenceID) && (tm2.frame == infiniteTimeFrameOfReferenceID))
     {if (tm1.timemark == tm2.timemark)
        return EqualCR;
      return (tm1.timemark == minTimeMark.timemark) ? LessCR : GreaterCR;
     }
    if (tm1.frame == infiniteTimeFrameOfReferenceID)
      return (tm1.timemark == minTimeMark.timemark) ? LessCR : GreaterCR;
    return (tm2.timemark == minTimeMark.timemark) ? GreaterCR : LessCR;
   }
  if (tm1.frame != tm2.frame)
    return NonComparableCR;
  if (tm1.timemark == tm2.timemark)
    return EqualCR;
  return (tm1.timemark < tm2.timemark) ? LessCR : GreaterCR;
}

bool equalTimeMarks(TimeMark tm1,TimeMark tm2)
{
  return (compareTimeMarks(tm1,tm2) == EqualCR);
}

bool lessTimeMarks(TimeMark tm1,TimeMark tm2)
{
  return (compareTimeMarks(tm1,tm2) == LessCR);
}

bool lessOrEqualTimeMarks(TimeMark tm1,TimeMark tm2)
{
CompareResult res2;
  
  res2  = compareTimeMarks(tm1,tm2);
  return (res2 == LessCR) || (res2 == EqualCR);
}

bool greaterTimeMarks(TimeMark tm1,TimeMark tm2)
{
  return (compareTimeMarks(tm1,tm2) == GreaterCR);
}

bool greaterOrEqualTimeMarks(TimeMark tm1,TimeMark tm2)
{
CompareResult res2;
  
  res2  = compareTimeMarks(tm1,tm2);
  return (res2 == GreaterCR) || (res2 == EqualCR);
}

bool noncomparableTimeMarks(TimeMark tm1,TimeMark tm2)
{
  return (compareTimeMarks(tm1,tm2) == NonComparableCR);
}


/********************************************************************/
/**                     Time Mark Arithmetic                       **/
/********************************************************************/
TimeUnit diffTimeMarks(TimeMark tm1,TimeMark tm2)
{
  assertion( tm1.frame != infiniteTimeFrameOfReferenceID, "diffTimeMarks: time marks should be finite" );
  assertion( tm2.frame != infiniteTimeFrameOfReferenceID, "diffTimeMarks: time marks should be finite" );
  assertion( tm1.frame == tm2.frame, "diffTimeMarks: time marks should be comparable" );

  return (tm2.timemark - tm1.timemark);
}

TimeMark addTimeMarks(TimeMark tm,TimeUnit delta)
{
TimeMark res;

  if (tm.frame == infiniteTimeFrameOfReferenceID)
    return tm;
  res.frame = tm.frame;
  res.timemark = tm.timemark + delta;
  // May assertion on overflow
  return res;
}

TimeMark subtractTimeMarks(TimeMark tm,TimeUnit delta)
{
TimeMark res;

  if (tm.frame == infiniteTimeFrameOfReferenceID)
    return tm;
  res.frame = tm.frame;
  res.timemark = tm.timemark - delta;
  // May assertion on overflow
  return res;
}


/********************************************************************/
/**                         Time Interval                          **/
/********************************************************************/
/* { minTimeMark, maxTimeMark } */
/* -1 is infiniteTimeFrameOfReferenceID */
const TimeInterval overallTimeInterval = { { -1, MIN_TIME_MARK }, { -1, MAX_TIME_MARK } };

TimeInterval createTimeInterval(TimeMark minMark,TimeMark maxMark)
{
TimeInterval res;

  // If min and max are incomparable an user should call setTimeMarkDependence before.
  // The only problem that we cann't set (min <= max) dependence. May be check min < max+1.
  assertion( lessOrEqualTimeMarks(minMark,maxMark),
             "createTimeInterval: minMark should be less than or equal to maxMark"
           );

  res.minMark = minMark;
  res.maxMark = maxMark;
  return res;
}

bool areIntersectedTimeIntervals(TimeInterval ti1,TimeInterval ti2)
{
TimeFrameOfReferenceID frame;

  if (equalTimeMarks(ti1.minMark,minTimeMark) && equalTimeMarks(ti2.minMark,minTimeMark))
    return true;
  if (equalTimeMarks(ti1.maxMark,maxTimeMark) && equalTimeMarks(ti2.maxMark,maxTimeMark))
    return true;

  frame = infiniteTimeFrameOfReferenceID;
  if (ti1.minMark.frame != infiniteTimeFrameOfReferenceID)
    frame = ti1.minMark.frame;
  if (ti1.maxMark.frame != infiniteTimeFrameOfReferenceID)
   {if (frame != infiniteTimeFrameOfReferenceID)
      assertion( frame==ti1.maxMark.frame, "areIntersectedTimeIntervals: intervals should be comparable" );
    frame = ti1.maxMark.frame;
   }
  if (ti2.minMark.frame != infiniteTimeFrameOfReferenceID)
   {if (frame != infiniteTimeFrameOfReferenceID)
      assertion( frame==ti2.minMark.frame, "areIntersectedTimeIntervals: intervals should be comparable" );
    frame = ti2.minMark.frame;
   }
  if (ti2.maxMark.frame != infiniteTimeFrameOfReferenceID)
   {if (frame != infiniteTimeFrameOfReferenceID)
      assertion( frame==ti2.maxMark.frame, "areIntersectedTimeIntervals: intervals should be comparable" );
    frame = ti2.maxMark.frame;
   }
  if (frame == infiniteTimeFrameOfReferenceID)
    return false;

  if (lessTimeMarks(ti1.maxMark,ti2.minMark))
    return false;
  if (lessTimeMarks(ti2.maxMark,ti1.minMark))
    return false;
  return true;
}

TimeInterval addTimeIntervals(TimeInterval ti,TimeUnit delta)
{
TimeInterval res;

  res.minMark = addTimeMarks(ti.minMark,delta);
  res.maxMark = addTimeMarks(ti.maxMark,delta);
  return res;
}

TimeInterval subtractTimeIntervals(TimeInterval ti,TimeUnit delta)
{
TimeInterval res;

  res.minMark = subtractTimeMarks(ti.minMark,delta);
  res.maxMark = subtractTimeMarks(ti.maxMark,delta);
  return res;
}


/********************************************************************/
/**                Auxiliary Time Interval Functions               **/
/********************************************************************/
static
int printLinearTimeMark(char* buffer,TimeMark timemark)
{
int res;

  if (timemark.frame == minTimeMark.frame)
   {
    if (equalTimeMarks(timemark,minTimeMark))
      res = sprintf( buffer, "-INFINITY" );
    else if (equalTimeMarks(timemark,maxTimeMark))
      res = sprintf( buffer, "+INFINITY" );
    else
      res = sprintf( buffer, "INVALID_TIMEMARK" );
   }
  else
   {
    res = sprintf( buffer, UI64FORMAT, timemark.timemark );
   }
  return res;
}

int printLinearTimeInterval(char* buffer,TimeInterval interval)
{
int res;

  if (interval.minMark.timemark == interval.maxMark.timemark)
    res = printLinearTimeMark( buffer, interval.minMark );
  else
   {
    res  = sprintf( buffer, "[" );
    res += printLinearTimeMark( buffer+res, interval.minMark );
    res += sprintf( buffer+res, "," );
    res += printLinearTimeMark( buffer+res, interval.maxMark );
    res += sprintf( buffer+res, "]" );
   }
  return res;
}

int printDistributedTimeInterval(char* buffer,TimeInterval interval)
{
int res;

  if (equalTimeMarks(interval.minMark,interval.maxMark))
   {
    if (interval.minMark.frame == minTimeMark.frame)
      res = printLinearTimeMark( buffer, interval.minMark );
    else if (interval.minMark.frame == systemTimeFrameOfReferenceID)
      res = sprintf( buffer, "(SystemFrame:" UI64FORMAT ")", interval.minMark.timemark );
    else
      res = sprintf( buffer, "(%d:" UI64FORMAT ")", interval.minMark.frame, interval.minMark.timemark );
   }
  else
   {res = sprintf( buffer, "[" );
    if (interval.minMark.frame == minTimeMark.frame)
      res += printLinearTimeMark( buffer+res, interval.minMark );
    else if (interval.minMark.frame == systemTimeFrameOfReferenceID)
      res += sprintf( buffer+res, "(SystemFrame:" UI64FORMAT ")", interval.minMark.timemark );
    else
      res += sprintf( buffer+res, "(%d:" UI64FORMAT ")", interval.minMark.frame, interval.minMark.timemark );
    res += sprintf( buffer+res, "," );
    if (interval.maxMark.frame == minTimeMark.frame)
      res += printLinearTimeMark( buffer+res, interval.maxMark );
    else if (interval.minMark.frame == systemTimeFrameOfReferenceID)
      res += sprintf( buffer+res, "(SystemFrame:" UI64FORMAT ")", interval.maxMark.timemark );
    else
      res += sprintf( buffer+res, "(%d:" UI64FORMAT ")", interval.maxMark.frame, interval.maxMark.timemark );
    res += sprintf( buffer+res, "]" );
   }
  return res;
}


/********************************************************************/
/**                          Current Time                          **/
/********************************************************************/
static bool automaticTimeMarksMode = false;

bool setAutomaticTimeMarksMode(bool enable)
{
bool oldAutomaticTimeMarksMode = automaticTimeMarksMode;
  
  automaticTimeMarksMode = enable;
  return oldAutomaticTimeMarksMode;
}

bool areAutomaticTimeMarksEnabled(void)
{
  return automaticTimeMarksMode;
}

static TimeMark getCurrentTimeMarkDefault(void)
{
TimeMark res;

  res.frame = systemTimeFrameOfReferenceID;
  res.timemark = time(NULL);
  return res;
}

static GetCurrentTimeMarkFuncType getCurrentTimeMarkFunction = getCurrentTimeMarkDefault;

TimeMark getCurrentTimeMark(void)
{
  return getCurrentTimeMarkFunction();
}


GetCurrentTimeMarkFuncType setDefaultCurrentTimeMarkFunction(GetCurrentTimeMarkFuncType new_func)
{
GetCurrentTimeMarkFuncType old_func;

  assertion( new_func!=NULL, "setDefaultCurrentTimeMarkFunction: new_func should not be NULL" );
  assertion( new_func!=getCurrentTimeMark,
             "setDefaultCurrentTimeMarkFunction: new_func should not be getCurrentTimeMark"
           );

  old_func = getCurrentTimeMarkFunction;
  getCurrentTimeMarkFunction = new_func;
  return old_func;
}

void setAutomaticTimeMark(TimeMark* timemark)
{
  if (automaticTimeMarksMode)
   {
    assertion( timemark != NULL, "setAutomaticTimeMark: timemark should not be NULL" );
    *timemark = getCurrentTimeMarkFunction();
   }
}


/********************************************************************/
/**                      Time Model property                       **/
/********************************************************************/
static TSTimeModel ts_time_model = NotUseTSTime;

TSTimeModel setTSTimeModel(TSTimeModel time_model)
{
TSTimeModel old_time_model;

  old_time_model = ts_time_model;
  ts_time_model = time_model;
  return old_time_model;
}

TSTimeModel getTSTimeModel()
{
  return ts_time_model;
}
