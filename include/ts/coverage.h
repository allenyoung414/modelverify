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
#ifndef __COVERAGE_H__
#define __COVERAGE_H__

#include "utils/boolean.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* CoverageText;

typedef int CoverageElement;

typedef enum
{ PRIMARY_COVERAGE
, DOMAIN_VARIATION_COVERAGE
, MULTIPLICATION_COVERAGE
}
COVERAGE_KIND;

#ifndef __cplusplus // Pure C

typedef struct _CoverageDesc
{
    COVERAGE_KIND kind;
    // the total number of elements in this coverage
    // irrespective of possible presence of filter expression.
    // for primary coverages this field denotes the length
    // of the 'data' array
    int size;
    // the number of direct base coverages for this coverage
    // for derived coverages this field denotes the length
    // of the 'data' array.
    // for primary coverages this field should be 0.
    // for coverages of 'DOMAIN_VARIATION' kind this field should be 1.
    int base_num;
    int order;
    // this flag is true if the coverage is global coverage, i.e. is defined
    // outside any specification function.
    bool is_global;
    int last_trace_id;
    CoverageText name;
    CoverageElement (*calc)(  );
    bool (*filter)( CoverageElement );
    const void* data[];
}
CoverageDesc;
#else 
// C++
typedef struct _CoverageDesc
{
    COVERAGE_KIND kind;
    // the total number of elements in this coverage
    // irrespective of possible presence of filter expression.
    // for primary coverages this field denotes the length
    // of the 'data' array
    int size;
    // the number of direct base coverages for this coverage
    // for derived coverages this field denotes the length
    // of the 'data' array.
    // for primary coverages this field should be 0.
    // for coverages of 'DOMAIN_VARIATION' kind this field should be 1.
    int base_num;
    int order;
    // this flag is true if the coverage is global coverage, i.e. is defined
    // outside any specification function.
    bool is_global;
    int last_trace_id;
    CoverageText name;
    CoverageElement (*calc)(  );
    bool (*filter)( CoverageElement );
}
// Fllowed by an array of elements
CoverageDesc;
#endif
extern int _CE_OUT_OF_COVERAGE;

const CoverageDesc* cov_component( const CoverageDesc* cov_desc, int a );

CoverageElement ce_product( const CoverageDesc* cov_desc, ... );

CoverageElement ce_coord( const CoverageDesc* cov_desc
                        , CoverageElement element
                        , int axis
                        , CoverageDesc** p_prim_cov_desc
                        );

CoverageText ce_txt( const CoverageDesc* cov_desc, CoverageElement elem );

bool isElementInCoverage( const CoverageDesc* cov_desc, int element );

void traceReachedCoverageElement( CoverageDesc* cov_desc, int element );

void traceCoverageStructure( CoverageDesc* cov_desc );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // __COVERAGE_H__
