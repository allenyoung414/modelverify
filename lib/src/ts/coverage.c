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
#include <stdarg.h>
#include <ts/coverage.h>
#include <utils/boolean.h>
#include <atl/stringbuffer.h>
#include <atl/string.h>
#include <c_tracer/c_tracer.h>

int _CE_OUT_OF_COVERAGE = -1;

static int bad( int size, int index ) { return index < 0 || size <= index; }

const CoverageDesc* cov_component( const CoverageDesc* cov_desc, int cn )
{
    if(    cov_desc->base_num == 0
        || bad( cov_desc->base_num, cn )
      )
        return 0;

    return (CoverageDesc*)cov_desc->data[ cn ];
}

int ce_coord_intern( const CoverageDesc* cov_desc
                   , int* p_element
                   , int* p_axis
                   , CoverageDesc ** p_prim_cov_desc
                   );

/*
 * returns the element that composes the given element.
 * resulting element belongs to particular primary base coverage of the
 * given coverage. the number of that primary coverage is given by 'axis'.
 * also this method returns that primary coverage via 'p_prim_cov_desc' pointer.
 */
CoverageElement ce_coord( const CoverageDesc* cov_desc
                        , CoverageElement element
                        , int axis
                        , CoverageDesc** p_prim_cov_desc
                        )
{
    if( element == _CE_OUT_OF_COVERAGE )
        return _CE_OUT_OF_COVERAGE;

    if( bad( cov_desc->size, element ) )
        return _CE_OUT_OF_COVERAGE;

    if( bad( cov_desc->order, axis ) )
        return _CE_OUT_OF_COVERAGE;

    if( cov_desc->kind == PRIMARY_COVERAGE )
    {
        if( p_prim_cov_desc != 0 )
            (*p_prim_cov_desc) = (CoverageDesc*)cov_desc;

        return element;
    } else {
        int bc;

        int res = _CE_OUT_OF_COVERAGE;
        int r2l_distance = cov_desc->order - axis - 1;

        for( bc = cov_desc->base_num - 1; bc >= 0; bc-- )
        {
            res = ce_coord_intern( cov_component( cov_desc, bc )
                                 , &element
                                 , &r2l_distance
                                 , p_prim_cov_desc
                                 );

            if( res != _CE_OUT_OF_COVERAGE )
                return res;
        }
    }
    return _CE_OUT_OF_COVERAGE;
}

int ce_coord_intern( const CoverageDesc* cov_desc
                   , int* p_element
                   , int* p_r2l_distance
                   , CoverageDesc ** p_prim_cov_desc
                   )
{
    int res = _CE_OUT_OF_COVERAGE;

    if( (*p_r2l_distance) >= cov_desc->order )
    {
        (*p_r2l_distance) = (*p_r2l_distance) - cov_desc->order;
        (*p_element) = (*p_element) / cov_desc->size;
        return res;

    } else {

        int bc;

        (*p_element) = (*p_element) % cov_desc->size;

        if(    (*p_r2l_distance) == 0
            && cov_desc->kind == PRIMARY_COVERAGE
          )
        {
            if( p_prim_cov_desc != 0 )
                (*p_prim_cov_desc) = (CoverageDesc*)cov_desc;

            return (*p_element);
        }

        for( bc = cov_desc->base_num - 1; bc >= 0; bc -- )
        {
            res = ce_coord_intern( cov_component( cov_desc, bc )
                                 , p_element
                                 , p_r2l_distance
                                 , p_prim_cov_desc
                                 );
            if( res != _CE_OUT_OF_COVERAGE )
                return res;
        }
    }

    return _CE_OUT_OF_COVERAGE;
}

/*
 * '...' parameter is a va_list of elements of direct base coverages of 'cov_desc'
 */
CoverageElement ce_product( const CoverageDesc* cov_desc, ... )
{
    CoverageElement res = 0;

    va_list coordinates;
    int coord;

    va_start( coordinates, cov_desc );

    if( cov_desc->base_num <= 1 )
    {
        res = va_arg( coordinates, int );
    } else {
        int bc;

        for( bc = 0; bc < cov_desc->base_num; bc++ )
        {
            coord = va_arg( coordinates, int );

            if( coord == _CE_OUT_OF_COVERAGE )
                return _CE_OUT_OF_COVERAGE;

            res += coord;

            if( bc < ( cov_desc->base_num - 1 ) )
                res *= cov_component( cov_desc, bc + 1 )->size;
        }
    }
    va_end( coordinates );
    return res;
}

CoverageText ce_txt( const CoverageDesc* cov_desc, CoverageElement elem )
{
    if( elem == _CE_OUT_OF_COVERAGE ) return 0;

    switch( cov_desc->kind )
    {
        case PRIMARY_COVERAGE:
            return ((CoverageText*)(cov_desc->data))[ elem ];
        case DOMAIN_VARIATION_COVERAGE:
            return ce_txt( (CoverageDesc*)cov_desc->data[ 0 ], elem );
        default:
            return 0;
    }
}

bool isElementInCoverage( const CoverageDesc* cov_desc, int element )
{
    if( element == _CE_OUT_OF_COVERAGE )
        return 0;

    if( cov_desc->filter != 0 )
    {
        return cov_desc->filter( element );
    } else
        return 1;
}

void traceReachedCoverageElement( CoverageDesc* cov_desc, int element )
{
    traceCoverageStructure( cov_desc );

    if( isElementInCoverage( cov_desc, element ) )
    {
        if( cov_desc->is_global )
        {
          traceCoverageElement( cov_desc->name, cov_desc->name, element );
        } else {
          traceCoverageElement( "", cov_desc->name, element );
        }
    }
}

void tracePrimaryCoverageElements( const CoverageDesc* cov_desc );

void traceCoverageStructure( CoverageDesc* cov_desc )
{
    if( cov_desc->last_trace_id >= getTraceId() )
        return;

    cov_desc->last_trace_id = getTraceId();

    if( cov_desc->is_global )
    {
      traceCoverageStructureStart( cov_desc->name );
    }

    traceCoverageStart( cov_desc->name );

    if( cov_desc->kind == PRIMARY_COVERAGE )
    {
        tracePrimaryCoverageElements( cov_desc );
    } else {
        int el_c;
        for( el_c = 0; el_c < cov_desc->size; el_c ++ )
        {
            StringBuffer* sb;
            int prim_el_num;
            int axis;
            CoverageDesc * p_prim_cov_desc;
            String * elem_str;

            if( !(isElementInCoverage( cov_desc, el_c )) )
                continue;

            sb = create_StringBuffer();
            append_StringBuffer( r( sb ), "( " );
            for( axis = 0; axis < cov_desc->order; axis ++ )
            {
                prim_el_num = ce_coord( cov_desc, el_c, axis, &p_prim_cov_desc );
                if( axis != 0 )
                {
                    append_StringBuffer( r( sb ), ", " );
                }

                append_StringBuffer( r( sb )
                                   , (char*)p_prim_cov_desc->data[ prim_el_num ]
                                   );
            }
            append_StringBuffer( r( sb ), " )" );

            elem_str = toString( r( sb ) );

            traceElement( el_c, toCharArray_String( r( elem_str ) ) );
            destroy_Object( elem_str );
            destroy_Object( sb );
        }
    }

    traceCoverageEnd();

    if( cov_desc->is_global )
    {
      traceCoverageStructureEnd();
    }
}

void tracePrimaryCoverageElements( const CoverageDesc* cov_desc )
{
    int i;
    for( i = 0; i < cov_desc->size; i ++ )
        traceElement( i, cov_desc->data[ i ] );
}

