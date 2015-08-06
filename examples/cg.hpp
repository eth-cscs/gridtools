#pragma once

#include <gridtools.hpp>

#include <stencil-composition/backend.hpp>

#include <stencil-composition/interval.hpp>
#include <stencil-composition/make_computation.hpp>

#include <boost/timer/timer.hpp>
/*
  @file This file shows an implementation of the various stencil operations.

 1nd order in time:
  3-point constant-coefficient stencil in one dimension, with symmetry.
  7-point constant-coefficient isotropic stencil in three dimensions, with symmetry.
  7-point variable-coefficient stencil in three dimension, with no coefficient symmetry.
  25-point variable-coefficient, anisotropic stencil in 3D, with symmetry across each axis.

 2nd order in time:
  25-point constant-coefficient, isotropic stencil in 3D, with symmetry across each axis.

  A diagonal dominant matrix is used with "N" as a center-point value for a N-point stencil   
  and "-1/N" as a off-diagonal value.
 */

using gridtools::level;
using gridtools::accessor;
using gridtools::range;
using gridtools::arg;

namespace cg{

using namespace gridtools;
using namespace enumtype;

#ifdef CXX11_ENABLED
using namespace expressions;
#endif

// This is the definition of the special regions in the "vertical" direction
// The K dimension can be split into logical splitters. They lie in between array elements.
// A level represent an element on the third coordinate, such that level<2,1> represent
// the array index ofllowing splitter<2>, while lavel<2,-1> represents the array value
// before splitter<2>.
typedef gridtools::interval<level<0,-1>, level<1,-1> > x_interval;
typedef gridtools::interval<level<0,-1>, level<1,1> > axis;

// 1st order in time, 3-point constant-coefficient stencil in one dimension, with symmetry.
struct d1point3{
    typedef accessor<0> out;
    typedef accessor<1, range<-1,1,0,0> > in; // this says to access x-1 anx x+1
    typedef boost::mpl::vector<out, in> arg_list;

    template <typename Domain>
    GT_FUNCTION
    static void Do(Domain const & dom, x_interval) {
        dom(out()) = 3.0*dom(in()) -0.33*(dom(in(x(-1)))+dom(in(x(+1))));
    }
};

// 1st order in time, 7-point constant-coefficient isotropic stencil in 3D, with symmetry.
struct d3point7{
    typedef accessor<0> out;
    typedef accessor<1, range<-1,1,-1,1> > in; // this says to access 6 neighbors
    typedef boost::mpl::vector<out, in> arg_list;

    template <typename Domain>
    GT_FUNCTION
    static void Do(Domain const & dom, x_interval) {
        dom(out()) = 7.0*dom(in())
                    - 1.0/7.0 * (dom(in(x(-1)))+dom(in(x(+1))))
                    - 1.0/7.0 * (dom(in(y(-1)))+dom(in(y(+1))))
                    - 1.0/7.0 * (dom(in(z(-1)))+dom(in(z(+1))));
    }
};

bool solver(uint_t x, uint_t y, uint_t z) {

    uint_t d1 = x;
    uint_t d2 = y;
    uint_t d3 = z;

#ifdef CUDA_EXAMPLE
#define BACKEND backend<Cuda, Block >
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend<Host, Block >
#else
#define BACKEND backend<Host, Naive >
#endif
#endif

    //    typedef gridtools::STORAGE<double, gridtools::layout_map<0,1,2> > storage_type;
    typedef gridtools::layout_map<0,1,2> layout_t;
    typedef gridtools::BACKEND::storage_type<float_type, layout_t >::type storage_type;
    typedef gridtools::BACKEND::temporary_storage_type<float_type, layout_t >::type tmp_storage_type;

     // Definition of the actual data fields that are used for input/output
    //storage_type in(d1,d2,d3,-1, "in"));
    storage_type out3d(d1,d2,d3,0., "domain_out");
    storage_type in3d(d1,d2,d3,1., "domain_in");

    storage_type out1d(d1,1,1,0., "domain_out");
    storage_type in1d(d1,1,1,1., "domain_in");

    // Definition of placeholders. The order of them reflect the order the user will deal with them
    // especially the non-temporary ones, in the construction of the domain
    typedef arg<0, storage_type > p_out; //domain
    typedef arg<1, storage_type > p_in;

    // An array of placeholders to be passed to the domain
    // I'm using mpl::vector, but the final API should look slightly simpler
    typedef boost::mpl::vector<p_out, p_in> accessor_list;

    // construction of the domain. The domain is the physical domain of the problem, with all the physical fields that are used, temporary and not
    // It must be noted that the only fields to be passed to the constructor are the non-temporary.
    // The order in which they have to be passed is the order in which they appear scanning the placeholders in order. (I don't particularly like this)
    gridtools::domain_type<accessor_list> domain3d
        (boost::fusion::make_vector(&out3d, &in3d));

    gridtools::domain_type<accessor_list> domain1d
        (boost::fusion::make_vector(&out1d, &in1d));

    // Definition of the physical dimensions of the problem.
    // The constructor takes the horizontal plane dimensions,
    // while the vertical ones are set according the the axis property soon after

    //Informs the library that the iteration space in the first two dimensions
    //is from 0 to d1-1 (included) in I (or x) direction
    uint_t di[5] = {0, 0, 1, d1-2, d1};
    //and and 0 to 0 on J (or y) direction
    uint_t dj[5] = {0, 0, 1, d2-2, d2};

    gridtools::coordinates<axis> coords3d(di, dj);
    coords3d.value_list[0] = 1; //specifying index of the splitter<0,-1>
    coords3d.value_list[1] = d3-2; //specifying index of the splitter<1,-1>

    uint_t dii[5] = {0, 0, 1, d1-2, d1};
    uint_t djj[5] = {0, 0, 0, 0, 1};
    gridtools::coordinates<axis> coords1d(dii, djj);
    coords1d.value_list[0] = 0; //specifying index of the splitter<0,-1>
    coords1d.value_list[1] = 0; //specifying index of the splitter<1,-1>

    /*
      Here we do lot of stuff
      1) We pass to the intermediate representation ::run function the description
      of the stencil, which is a multi-stage stencil (mss)
      The mss includes (in order of execution) a laplacian, two fluxes which are independent
      and a final step that is the out_function
      2) The logical physical domain with the fields to use
      3) The actual domain dimensions
     */

//------------------------------------------------------------------------------
#ifdef __CUDACC__
    gridtools::computation* stencil_step_1 =
#else
        boost::shared_ptr<gridtools::computation> stencil_step_1 =
#endif
      gridtools::make_computation<gridtools::BACKEND, layout_t>
        (
            gridtools::make_mss // mss_descriptor
            (
                execute<forward>(),
                gridtools::make_esf<d1point3>(p_out(), p_in()) // esf_descriptor
                ),
            domain1d, coords1d
            );


    //prepare computation
    stencil_step_1->ready();
    stencil_step_1->steady();
    
    //start timer
    boost::timer::cpu_timer time1;

    //TODO: swap domains between time iteration
    for(int i=0; i<2; i++)
        stencil_step_1->run();

    boost::timer::cpu_times lapse_time1 = time1.elapsed();
    stencil_step_1->finalize();

    printf("Print domain after computation\n");
    out1d.print();

    std::cout << "TIME d1point3: " << boost::timer::format(lapse_time1) << std::endl;
//------------------------------------------------------------------------------
#ifdef __CUDACC__
    gridtools::computation* stencil_step_2 =
#else
        boost::shared_ptr<gridtools::computation> stencil_step_2 =
#endif
      gridtools::make_computation<gridtools::BACKEND, layout_t>
        (
            gridtools::make_mss // mss_descriptor
            (
                execute<forward>(),
                gridtools::make_esf<d3point7>(p_out(), p_in()) // esf_descriptor
                ),
            domain3d, coords3d
            );


    //prepare computation
    stencil_step_2->ready();
    stencil_step_2->steady();
    
    //start timer
    boost::timer::cpu_timer time2;

    //TODO: swap domains between time iteration
    for(int i=0; i<2; i++)
        stencil_step_2->run();

    boost::timer::cpu_times lapse_time2 = time2.elapsed();
    stencil_step_2->finalize();

    printf("Print domain after computation\n");
    out3d.print();

    std::cout << "TIME d3point7: " << boost::timer::format(lapse_time2) << std::endl;
//------------------------------------------------------------------------------

    return 1;
    }//solver
}//namespace tridiagonal
