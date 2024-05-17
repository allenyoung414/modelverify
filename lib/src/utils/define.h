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
 * File: define.h
 * Description: 
 *       This file defines macroses that used for access from SEC code
 *       to identifiers equaled to SEC keywords.
 */

#ifdef __SEC__
#define call $call
#define coverage $coverage
#define invariant $invariant
#define iterate $iterate
#define iterator $iterator
#define mediator $mediator
#define pre $pre
#define post $post
#define reaction $reaction
#define reads $reads
#define scenario $scenario
#define specification $specification
#define stable $stable
#define state $state
#define updates $updates
#define writes $writes
#endif
