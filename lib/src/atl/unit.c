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
 * File: unit.c
 * Description:
 *       This file defines the specification type containing the only value.
 *       It is a part of CTesK Abstract Type Library.
 */

#include <atl/unit.h>

#include <utils/boolean.h>

static int compare_Unit( Unit *left, Unit *right ) {
    return 0;
}

static String *to_string_Unit( Unit *unit ) {
    return create_String( "<unit>" );
}

static String *to_XML_Unit( Unit *unit ) {
	return to_XML_spec("Unit", to_string_Unit(unit));
}

const Type type_Unit
    = { "Unit"
      , 0
      , &type_Object
      , (Init)init_Default
      , (Copy)copy_Default
      , (Compare)compare_Unit
      , (ToString)to_string_Unit
      , (ToXML)to_XML_Unit
      , (CheckInvariant)check_invariant_Default
      , (Enumerate)enumerate_Default
      , (Destroy)destroy_Default
      };

Unit *create_Unit() {
    return create(&type_Unit);
}
