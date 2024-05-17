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
 * File: undef.h
 * Description: 
 *       This file undefines macroses that used for access from SEC code
 *       to identifiers equaled to SEC keywords.
 */

#ifdef __SEC__
#undef call
#undef coverage
#undef invariant
#undef iterate
#undef iterator
#undef mediator
#undef pre
#undef post
#undef reaction
#undef reads
#undef scenario
#undef specification
#undef stable
#undef state
#undef updates
#undef writes
#endif
