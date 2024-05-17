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
 * File: boolean.h
 * Description: 
 *       This file contains boolean type definition.
 */

#ifndef BOOLEAN_H
#define BOOLEAN_H

#ifndef __cplusplus
  #ifdef bool
  #undef bool
  #endif
  
  typedef char bool;

  #ifdef true
  #undef true
  #endif
  #define true  1

  #ifdef false
  #undef false
  #endif
  #define false 0

#endif /* __cplusplus */

#endif /* BOOLEAN_H */
