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
namespace gridtools {

    /**@brief Expression subtracting two arguments*/
    template < typename ArgType1, typename ArgType2 >
    struct expr_minus : public binary_expr< ArgType1, ArgType2 > {
        typedef binary_expr< ArgType1, ArgType2 > super;

        GT_FUNCTION
        constexpr expr_minus(ArgType1 const &first_operand, ArgType2 const &second_operand)
            : super(first_operand, second_operand) {}

        template < typename Arg1, typename Arg2 >
        GT_FUNCTION constexpr expr_minus(expr_minus< Arg1, Arg2 > const &other)
            : super(other) {}

#ifndef __CUDACC__
      private:
#endif
        GT_FUNCTION
        constexpr expr_minus() {}

#ifndef __CUDACC__
        static char constexpr op[] = " - ";
        using operation = string_c< print, op >;

      public:
        // currying and recursion (this gets inherited)
        using to_string = concatenate< tokens::open_par, ArgType1, operation, ArgType2, tokens::closed_par >;
#endif
    };

    /**@brief Expression negating an argument*/
    template < typename ArgType1 >
    struct expr_minus_unary : public unary_expr< ArgType1 > {
        typedef unary_expr< ArgType1 > super;

        GT_FUNCTION
        constexpr expr_minus_unary(ArgType1 const &first_operand) : super(first_operand) {}

        template < typename Arg1 >
        GT_FUNCTION constexpr expr_minus_unary(expr_minus_unary< Arg1 > const &other)
            : super(other) {}

#ifndef __CUDACC__
      private:
#endif
        GT_FUNCTION
        constexpr expr_minus_unary() {}

#ifndef __CUDACC__
        static char constexpr op[] = " - ";
        using operation = string_c< print, op >;

      public:
        // currying and recursion (this gets inherited)
        using to_string = concatenate< tokens::open_par, operation, ArgType1, tokens::closed_par >;
#endif
    };

    template < typename ArgType1, typename ArgType2 >
    struct is_binary_expr< expr_minus< ArgType1, ArgType2 > > : boost::mpl::true_ {};

    template < typename ArgType1 >
    struct is_unary_expr< expr_minus_unary< ArgType1 > > : boost::mpl::true_ {};

    namespace expressions {
        /** minus expression*/
        template < typename ArgType1,
            typename ArgType2,
            typename boost::disable_if< no_expr_nor_accessor_types< ArgType1, ArgType2 >, int >::type = 0 >
        GT_FUNCTION constexpr expr_minus< ArgType1, ArgType2 > operator-(ArgType1 arg1, ArgType2 arg2) {
            return expr_minus< ArgType1, ArgType2 >(arg1, arg2);
        }

        /** minus expr for sign (unary) */
        template < typename ArgType1,
            typename boost::disable_if< no_expr_nor_accessor_types< ArgType1 >, int >::type = 0 >
        GT_FUNCTION constexpr expr_minus_unary< ArgType1 > operator-(ArgType1 arg1) {
            return expr_minus_unary< ArgType1 >(arg1);
        }

        namespace evaluation {

            /** minus evaluation*/
            template < typename IterateDomain, typename ArgType1, typename ArgType2 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_minus< ArgType1, ArgType2 > const &arg)
                -> decltype(it_domain(arg.first_operand) - it_domain(arg.second_operand)) {
                return it_domain(arg.first_operand) - it_domain(arg.second_operand);
            }

            /** subtract with scalar evaluation*/
            template < typename IterateDomain,
                typename ArgType1,
                typename FloatType,
                typename boost::enable_if< typename boost::is_arithmetic< FloatType >::type, int >::type = 0 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_minus< ArgType1, FloatType > const &arg)
                -> decltype(it_domain(arg.first_operand) - arg.second_operand) {
                return it_domain(arg.first_operand) - arg.second_operand;
            }

            /** subtract from scalar evaluation (non commutative) */
            template < typename IterateDomain,
                typename FloatType,
                typename ArgType2,
                typename boost::enable_if< typename boost::is_arithmetic< FloatType >::type, int >::type = 0 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_minus< FloatType, ArgType2 > const &arg)
                -> decltype(arg.first_operand - it_domain(arg.second_operand)) {
                return arg.first_operand - it_domain(arg.second_operand);
            }

            // automatic differentiation
            /** plus derivative evaluation*/
            template < typename IterateDomain, typename ArgType1, typename ArgType2 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_derivative< expr_minus< ArgType1, ArgType2 > > const &arg)
                -> decltype(it_domain(expr_derivative< ArgType1 >(arg.first_operand)) -
                            it_domain(expr_derivative< ArgType2 >(arg.second_operand))) {
                return it_domain(expr_derivative< ArgType1 >(arg.first_operand)) -
                       it_domain(expr_derivative< ArgType2 >(arg.second_operand));
            }

            /** sum with scalar evaluation*/
            template < typename IterateDomain,
                typename ArgType1,
                typename FloatType,
                typename boost::enable_if< typename boost::is_arithmetic< FloatType >::type, int >::type = 0 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_derivative< expr_minus< ArgType1, FloatType > > const &arg)
                -> decltype(it_domain(expr_derivative< ArgType1 >(arg.first_operand))) {
                return it_domain(expr_derivative< ArgType1 >(arg.first_operand));
            }

            /** sum with scalar evaluation*/
            template < typename IterateDomain,
                typename FloatType,
                typename ArgType2,
                typename boost::enable_if< typename boost::is_arithmetic< FloatType >::type, int >::type = 0 >
            GT_FUNCTION auto static constexpr value(
                IterateDomain &it_domain, expr_derivative< expr_minus< FloatType, ArgType2 > > const &arg)
                -> decltype(-it_domain(expr_derivative< ArgType2 >(arg.second_operand))) {
                return -it_domain(expr_derivative< ArgType2 >(arg.second_operand));
            }

            /** minus unary evaluation*/
            template < typename IterateDomain, typename ArgType1 >
            GT_FUNCTION auto static constexpr value(IterateDomain &it_domain, expr_minus_unary< ArgType1 > const &arg)
                -> decltype(-it_domain(arg.first_operand)) {
                return -it_domain(arg.first_operand);
            }

        } // namespace evaluation
    }     // namespace expressions

} // namespace gridtools
