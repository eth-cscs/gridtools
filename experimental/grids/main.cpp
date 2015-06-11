using uint_t = unsigned int;
#include <iostream>
#include "grid.hpp"
#include <storage/base_storage.h>
#include <common/layout_map.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/adapted/mpl/detail/size_impl.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <type_traits>
#include "array_addons.hpp"
#include "placeholders.hpp"
#include "make_stencil.hpp"
#include "accessor.hpp"

using gridtools::layout_map;
using gridtools::wrap_pointer;

using cell_storage_type = gridtools::base_storage<wrap_pointer<double>, layout_map<0,1,2> >;
using edge_storage_type = gridtools::base_storage<wrap_pointer<double>, layout_map<0,1,2> >;

using trapezoid_2D = gridtools::trapezoid_2D_no_tile<cell_storage_type, edge_storage_type>;

struct stencil_on_cells {
    typedef accessor<0, trapezoid_2D::cells> out;
    typedef accessor<1, trapezoid_2D::cells> in;
    typedef accessor<2, trapezoid_2D::edges> out_edges_NOT_USED;
    typedef accessor<3, trapezoid_2D::edges> in_edges;

    template <typename GridAccessors>
    void
    operator()(GridAccessors /*const*/& eval/*, region*/) const {
        // std::cout << "i = " << eval.i()
        //           << " j = " << eval.j()
        //           << std::endl;
        auto ff = [](const double _in, const double _res) -> double {return _in+_res;};
        eval(out()) = eval(on_neighbors<in>(ff), 0.0) + eval(on_neighbors<in_edges>(ff), 0.0);
    }
};

struct stencil_on_edges {
    typedef accessor<0, trapezoid_2D::cells> out_NOT_USED;
    typedef accessor<1, trapezoid_2D::cells> in;
    typedef accessor<2, trapezoid_2D::edges> out_edges;
    typedef accessor<3, trapezoid_2D::edges> in_edges;

    template <typename GridAccessors>
    void
    operator()(GridAccessors /*const*/& eval/*, region*/) const {
        // std::cout << "i = " << eval.i()
        //           << " j = " << eval.j()
        //           << std::endl;
        auto ff = [](const double _in, const double _res) -> double {return _in+_res;};
        eval(out_edges()) = eval(on_neighbors<in>(ff), 0.0) + eval(on_neighbors<in_edges>(ff), 0.0);
    }
};

#define EVAL(f,x,y)                                                     \
    std::cout << #f << ": " << gridtools::array<decltype(x),2>{x,y} << " -> " << (grid.f({x,y})) << std::endl

#define EVAL_C(f,x,y, result)                                                    \
    std::cout << #f << ": " << gridtools::array<decltype(x),2>{x,y} << " -> " << (grid.f({x,y})) << ", expect " << result; \
    std::cout << ": Passed? " << std::boolalpha << (grid.f({x,y}) == result) << std::endl

int main() {
    uint_t NC = trapezoid_2D::u_cell_size_i(6);
    uint_t MC = trapezoid_2D::u_cell_size_j(12);
   
    uint_t NE = trapezoid_2D::u_edge_size_i(6);
    uint_t ME = trapezoid_2D::u_edge_size_j(12);

    std::cout << "NC = " << NC << " "
              << "MC = " << MC
              << std::endl;
    
    std::cout << "NE = " << NE << " "
              << "ME = " << ME
              << std::endl;

    cell_storage_type cells(trapezoid_2D::u_cell_size(gridtools::array<uint_t, 2>{NC, MC}));
    edge_storage_type edges(trapezoid_2D::u_edge_size(gridtools::array<uint_t, 2>{NE, ME}));

    trapezoid_2D grid(/*cells, edges,*/ 6, 12);

    EVAL_C(cell2cells_ll_p0, 1, 1, (gridtools::array<uint_t,3>{9, 24, 25}));
    EVAL_C(cell2cells_ll_p0, 1, 2, (gridtools::array<uint_t,3>{10, 25, 26}));
    EVAL_C(cell2cells_ll_p1, 1, 3, (gridtools::array<uint_t,3>{19, 20, 35}));
    EVAL_C(cell2cells_ll_p1, 1, 4, (gridtools::array<uint_t,3>{20, 21, 36}));
    EVAL_C(cell2cells, 2, 3, (gridtools::array<uint_t,3>{33, 34, 49}));
    EVAL_C(cell2cells, 2, 4, (gridtools::array<uint_t,3>{26, 41, 42}));
    EVAL_C(cell2cells, 3, 3, (gridtools::array<uint_t,3>{49, 50, 65}));
    EVAL_C(cell2cells, 3, 4, (gridtools::array<uint_t,3>{42, 57, 58}));

    EVAL_C(edge2edges_ll_p0, 2, 3, (gridtools::array<uint_t,4>{66,67,59,82}));
    EVAL_C(edge2edges_ll_p1, 2, 3, (gridtools::array<uint_t,4>{43,28,51,67}));
    EVAL_C(edge2edges_ll_p2, 2, 3, (gridtools::array<uint_t,4>{51,59,52,83}));
    EVAL_C(edge2edges, 2, 2, (gridtools::array<uint_t,4>{48,56,49,80}));
    EVAL_C(edge2edges, 2, 3, (gridtools::array<uint_t,4>{64,57,65,80}));
    EVAL_C(edge2edges, 2, 4, (gridtools::array<uint_t,4>{41,26,49,65}));

    EVAL_C(cell2edges_ll_p0, 2, 3, (gridtools::array<uint_t,3>{51,59,67}));
    EVAL_C(cell2edges_ll_p1, 2, 3, (gridtools::array<uint_t,3>{67,52,83}));
    EVAL_C(cell2edges, 2, 3, (gridtools::array<uint_t,3>{65,50,81}));
    EVAL_C(cell2edges, 2, 4, (gridtools::array<uint_t,3>{58,50,66}));

    EVAL_C(edge2cells_ll_p0, 2, 3, (gridtools::array<uint_t,2>{42,35}));
    EVAL_C(edge2cells_ll_p1, 2, 3, (gridtools::array<uint_t,2>{27,35}));
    EVAL_C(edge2cells_ll_p2, 2, 3, (gridtools::array<uint_t,2>{35,43}));
    EVAL_C(edge2cells, 2, 3, (gridtools::array<uint_t,2>{33,40}));
    EVAL_C(edge2cells, 2, 4, (gridtools::array<uint_t,2>{25,33}));
    EVAL_C(edge2cells, 2, 5, (gridtools::array<uint_t,2>{33,41}));


    cell_storage_type cells_out(trapezoid_2D::u_cell_size(gridtools::array<uint_t, 2>{NC, MC}));
    edge_storage_type edges_out(trapezoid_2D::u_edge_size(gridtools::array<uint_t, 2>{NE, ME}));

    //    trapezoid_2D grid_out(cells_out, edges_out, NC, MC);

    typedef arg<0, trapezoid_2D::cells> out_cells;
    typedef arg<1, trapezoid_2D::cells> in_cells;
    typedef arg<2, trapezoid_2D::edges> out_edges;
    typedef arg<3, trapezoid_2D::edges> in_edges;

    auto x = make_esf<stencil_on_cells, trapezoid_2D, trapezoid_2D::cells>
        (out_cells(), in_cells(), out_edges(), in_edges());

    accessor_type<boost::mpl::vector<in_cells, out_cells, out_edges, in_edges>,
                  trapezoid_2D, trapezoid_2D::cells> acc
        (boost::fusion::vector<cell_storage_type*, cell_storage_type*, edge_storage_type*, edge_storage_type*>
         (&cells_out, &cells, &edges_out, &edges), grid, 0,0);


    /** Iteration on CELLS
     */
    for (int i = 1; i < NC-1; ++i) {
        acc.set_ij(i, 1);
        for (int j = 2; j < MC-2; ++j) {
            acc.inc_j();
            decltype(x)::functor()(acc);
        }
    }

    auto y = make_esf<stencil_on_edges, trapezoid_2D, trapezoid_2D::edges>
        (out_cells(), out_cells(), out_edges(), in_edges());

    accessor_type<boost::mpl::vector<in_cells, out_cells, out_edges, in_edges>,
                  trapezoid_2D, trapezoid_2D::edges> accy
        (boost::fusion::vector<cell_storage_type*, cell_storage_type*, edge_storage_type*, edge_storage_type*>
         (&cells_out, &cells, &edges_out, &edges), grid, 0,0);


    /** Iteration on CELLS
     */
    for (int i = 1; i < NE-1; ++i) {
        acc.set_ij(i, 1);
        for (int j = 3; j < ME-3; ++j) {
            acc.inc_j();
            decltype(x)::functor()(acc);
        }
    }

    return 0;
}
