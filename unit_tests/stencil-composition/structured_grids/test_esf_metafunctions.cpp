/*
  GridTools Libraries

  Copyright (c) 2017, ETH Zurich and MeteoSwiss
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/

#include <gridtools/stencil-composition/stencil-composition.hpp>
#include <gridtools/tools/backend_select.hpp>

using namespace gridtools;
using namespace enumtype;

constexpr int level_offset_limit = 1;

template <uint_t Splitter, int_t Offset>
using level_t = level<Splitter, Offset, level_offset_limit>;

typedef interval<level_t<0, -1>, level_t<1, -1>> x_interval;

struct functor0 {
    typedef accessor<0, enumtype::in, extent<0, 0, -1, 3, -2, 0>> in0;
    typedef accessor<1, enumtype::in, extent<-1, 1, 0, 2, -1, 2>> in1;
    typedef accessor<2, enumtype::in, extent<-3, 3, -1, 2, 0, 1>> in2;
    typedef accessor<3, enumtype::inout> out;

    typedef boost::mpl::vector<in0, in1, in2, out> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor1 {
    typedef accessor<0, enumtype::in, extent<0, 1, -1, 2, 0, 0>> in0;
    typedef accessor<1, enumtype::inout> out;
    typedef accessor<2, enumtype::in, extent<-3, 0, -3, 0, 0, 2>> in2;
    typedef accessor<3, enumtype::in, extent<0, 2, 0, 2, -2, 3>> in3;

    typedef boost::mpl::vector<in0, out, in2, in3> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor2 {
    typedef accessor<0, enumtype::in, extent<-3, 3, -1, 0, -2, 1>> in0;
    typedef accessor<1, enumtype::in, extent<-3, 1, -2, 1, 0, 2>> in1;
    typedef accessor<2, enumtype::inout> out;

    typedef boost::mpl::vector<in0, in1, out> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor3 {
    typedef accessor<0, enumtype::in, extent<0, 3, 0, 1, -2, 0>> in0;
    typedef accessor<1, enumtype::in, extent<-2, 3, 0, 2, -3, 1>> in1;
    typedef accessor<2, enumtype::inout> out;
    typedef accessor<3, enumtype::in, extent<-1, 3, -3, 0, -3, 2>> in3;

    typedef boost::mpl::vector<in0, in1, out, in3> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor4 {
    typedef accessor<0, enumtype::in, extent<0, 3, -2, 1, -3, 2>> in0;
    typedef accessor<1, enumtype::in, extent<-2, 3, 0, 3, -3, 2>> in1;
    typedef accessor<2, enumtype::in, extent<-1, 1, 0, 3, 0, 3>> in2;
    typedef accessor<3, enumtype::inout> out;

    typedef boost::mpl::vector<in0, in1, in2, out> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor5 {
    typedef accessor<0, enumtype::in, extent<-3, 1, -1, 2, -1, 1>> in0;
    typedef accessor<1, enumtype::in, extent<0, 1, -2, 2, 0, 3>> in1;
    typedef accessor<2, enumtype::in, extent<0, 2, 0, 3, -1, 2>> in2;
    typedef accessor<3, enumtype::inout> out;

    typedef boost::mpl::vector<in0, in1, in2, out> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

struct functor6 {
    typedef accessor<0, enumtype::inout> out;
    typedef accessor<1, enumtype::in, extent<0, 3, -3, 2, 0, 0>> in1;
    typedef accessor<2, enumtype::in, extent<-3, 2, 0, 2, -1, 2>> in2;
    typedef accessor<3, enumtype::in, extent<-1, 0, -1, 0, -1, 3>> in3;

    typedef boost::mpl::vector<out, in1, in2, in3> arg_list;

    template <typename Evaluation>
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {}
};

std::ostream &operator<<(std::ostream &s, functor0) { return s << "functor0"; }
std::ostream &operator<<(std::ostream &s, functor1) { return s << "functor1"; }
std::ostream &operator<<(std::ostream &s, functor2) { return s << "functor2"; }
std::ostream &operator<<(std::ostream &s, functor3) { return s << "functor3"; }
std::ostream &operator<<(std::ostream &s, functor4) { return s << "functor4"; }
std::ostream &operator<<(std::ostream &s, functor5) { return s << "functor5"; }
std::ostream &operator<<(std::ostream &s, functor6) { return s << "functor6"; }

typedef gridtools::storage_traits<backend_t::backend_id_t>::storage_info_t<0, 3> storage_info_t;
typedef gridtools::storage_traits<backend_t::backend_id_t>::data_store_t<float_type, storage_info_t> storage_t;

typedef arg<0, storage_t> o0;
typedef arg<1, storage_t> o1;
typedef arg<2, storage_t> o2;
typedef arg<3, storage_t> o3;
typedef arg<4, storage_t> o4;
typedef arg<5, storage_t> o5;
typedef arg<6, storage_t> o6;
typedef arg<7, storage_t> in0;
typedef arg<8, storage_t> in1;
typedef arg<9, storage_t> in2;
typedef arg<10, storage_t> in3;
int main() {
    typedef decltype(make_stage<functor0>(in0(), in1(), in2(), o0())) functor0__;
    typedef decltype(make_stage<functor1>(in3(), o1(), in0(), o0())) functor1__;
    typedef decltype(make_stage<functor2>(o0(), o1(), o2())) functor2__;
    typedef decltype(make_stage<functor3>(in1(), in2(), o3(), o2())) functor3__;
    typedef decltype(make_stage<functor4>(o0(), o1(), o3(), o4())) functor4__;
    typedef decltype(make_stage<functor5>(in3(), o4(), in0(), o5())) functor5__;
    typedef decltype(make_stage<functor6>(o6(), o5(), in1(), in2())) functor6__;
    typedef decltype(make_multistage(execute<forward>(),
        functor0__(),
        functor1__(),
        functor2__(),
        functor3__(),
        functor4__(),
        functor5__(),
        functor6__())) mss_t;
    typedef boost::mpl::vector<o0, o1, o2, o3, o4, o5, o6, in0, in1, in2, in3> placeholders;

    typedef compute_extents_of<init_map_of_extents<placeholders>::type>::for_mss<mss_t>::type final_map;

    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, o0>::type, extent<-5, 11, -10, 10, -5, 13>>::type::value),
        "o0 extent<-5, 11, -10, 10, -5, 13> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, o1>::type, extent<-5, 9, -10, 8, -3, 10>>::type::value),
        "o1 extent<-5, 9, -10, 8, -3, 10> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, o2>::type, extent<-2, 8, -8, 7, -3, 8>>::type::value),
        "o2 extent<-2, 8, -8, 7, -3, 8> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, o3>::type, extent<-1, 5, -5, 7, 0, 6>>::type::value),
        "o3 extent<-1, 5, -5, 7, 0, 6> ");
    GRIDTOOLS_STATIC_ASSERT((std::is_same<boost::mpl::at<final_map, o4>::type, extent<0, 4, -5, 4, 0, 3>>::type::value),
        "o4 extent<0, 4, -5, 4, 0, 3> ");
    GRIDTOOLS_STATIC_ASSERT((std::is_same<boost::mpl::at<final_map, o5>::type, extent<0, 3, -3, 2, 0, 0>>::type::value),
        "o5 extent<0, 3, -3, 2, 0, 0> ");
    GRIDTOOLS_STATIC_ASSERT((std::is_same<boost::mpl::at<final_map, o6>::type, extent<0, 0, 0, 0, 0, 0>>::type::value),
        "o6 extent<0, 0, 0, 0, 0, 0> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, in0>::type, extent<-8, 11, -13, 13, -7, 13>>::type::value),
        "in0 extent<-8, 11, -13, 13, -7, 13> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, in1>::type, extent<-6, 12, -10, 12, -6, 15>>::type::value),
        "in1 extent<-6, 12, -10, 12, -6, 15> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, in2>::type, extent<-8, 14, -11, 12, -5, 14>>::type::value),
        "in2 extent<-8, 14, -11, 12, -5, 14> ");
    GRIDTOOLS_STATIC_ASSERT(
        (std::is_same<boost::mpl::at<final_map, in3>::type, extent<-5, 10, -11, 10, -3, 10>>::type::value),
        "in3 extent<-5, 10, -11, 10, -3, 10> ");
    /* total placeholders (rounded to 10) _SIZE = 20*/
    return 0;
}
