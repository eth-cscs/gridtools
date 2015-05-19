/**
 * This code was automatically generated by gridtools4py:
 * the Python interface to the Gridtools library
 *
 */
#pragma once

#include <gridtools.h>



using gridtools::accessor;
using gridtools::range;

using namespace gridtools;



namespace {{ namespace }}
{
//
// definition of the special regions in the vertical (k) direction
//
typedef gridtools::interval<level<0,-1>, level<1,-1> > x_interval;
typedef gridtools::interval<level<0,-2>, level<1,1> > axis;

//
// the definition of the operators that compose a multistage stencil
// is extracted from the AST analysis of the loop comprehensions
// in Python, which use the 'kernel' function as a starting point
//
{{ functor_src }}

} // namespace {{ namespace }}

