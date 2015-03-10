/*
 * meta_array.h
 *
 *  Created on: Feb 20, 2015
 *      Author: carlosos
 */

#pragma once
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/fold.hpp>

namespace gridtools {

/*
 * @struct is_sequence_of
 * metafunction that determines if a mpl sequence is a sequence of types determined by the filter
 * @param TSeq sequence to query
 * @param TPred filter that determines the condition
 */
template<typename Seq, template<typename> class Pred>
struct is_sequence_of
{
    typedef boost::mpl::quote1<Pred> pred_t;

    typedef typename boost::mpl::lambda<
        boost::mpl::not_<
            typename pred_t::template apply<boost::mpl::_1>
        >
    >::type neg_pred_t;

    typedef typename boost::mpl::eval_if<
        boost::mpl::is_sequence<Seq>,
        boost::mpl::eval_if<
            boost::is_same<
                typename boost::mpl::find_if<Seq, neg_pred_t >::type,
                typename boost::mpl::end<Seq>::type
            >,
            boost::mpl::true_,
            boost::mpl::false_
        >,
        boost::mpl::false_
    >::type type;

    BOOST_STATIC_CONSTANT(bool, value = (type::value) );
};

/**
 * @brief wrapper class around a sequence of types. The goal of the class is to identify that a type is an array of types that
 * fulfil a predicate
 * (without having to inspect each element of the sequence)
 */
template<typename Sequence, typename Pred>
struct meta_array{
    BOOST_STATIC_ASSERT((boost::mpl::is_sequence<Sequence>::value));

    //check that predicate returns true for all elements
    typedef typename boost::mpl::fold<
        Sequence,
        boost::mpl::true_,
        boost::mpl::and_<
            boost::mpl::_1,
            typename Pred::template apply<boost::mpl::_2>
        >
    >::type is_array_of_pred_t;

    BOOST_STATIC_ASSERT((is_array_of_pred_t::value));

    typedef Sequence elements_t;
};

//type traits for meta_array
template<typename T> struct is_meta_array : boost::mpl::false_{};

template<typename sequence, typename TPred> struct is_meta_array< meta_array<sequence, TPred> > : boost::mpl::true_{};

template<typename T, template<typename> class pred> struct is_meta_array_of : boost::mpl::false_{};

template<typename sequence, typename pred, template<typename> class pred_query>
struct is_meta_array_of< meta_array<sequence, pred>, pred_query>
{
    typedef typename boost::is_same<boost::mpl::quote1<pred_query>, pred >::type type;
    BOOST_STATIC_CONSTANT(bool, value=(type::value));
};

} //namespace gridtools
