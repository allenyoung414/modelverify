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
 * File: TSEnvironment.h 
 * Description: 
 *       This is a header file of TSEnvironment.
 */

#ifndef __TRACER_TS_ENVIRONMENT_H__
#define __TRACER_TS_ENVIRONMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************/
/**                     CTesK Version Info                         **/
/********************************************************************/
extern char* ts_get_ctesk_version(void);
extern char* ts_get_ctesk_build(void);


/********************************************************************/
/**                         TS Environment                         **/
/********************************************************************/
struct _TSEnvironment
{
  char * (*getOSInfo)();
  char * (*getHostInfo)();
};
typedef struct _TSEnvironment TSEnvironment;


/********************************************************************/
/**                    TS Environment Factory                      **/
/********************************************************************/
TSEnvironment* TSEnvironment_createTSEnvironment();


#ifdef __cplusplus
}
#endif

#endif /* __TRACER_TS_ENVIRONMENT_H__ */
