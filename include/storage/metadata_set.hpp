#pragma once
#include <boost/fusion/include/as_set.hpp>

/**
@file
@brief implementing a set
*/

namespace gridtools{


    template<typename T>
    struct is_pointer : boost::mpl::false_{};

    template<typename T>
    struct is_pointer<pointer<T> > : boost::mpl::true_{};

    /**
       @brief class that given a generig MPL sequence creates a fusion set.

       The interface of this class allows to insert and get elements of the sequence give its type.
       It also allows to query if the element corresponding to a given type has been or not initialized

     */
    template<typename Sequence>
    struct metadata_set{
        GRIDTOOLS_STATIC_ASSERT( boost::mpl::is_sequence<Sequence>::value,
                                 "internal error: not a sequence" );
        GRIDTOOLS_STATIC_ASSERT( (is_sequence_of<Sequence, is_pointer>::value),
                                 "internal error: not a sequence of pointers");
        typedef typename boost::fusion::result_of::as_set<Sequence>::type set_t;

        // DISALLOW_COPY_AND_ASSIGN(metadata_set);

    private:
        set_t m_set;

    public:
        GT_FUNCTION
        metadata_set() : m_set(){};

        __device__
        metadata_set(metadata_set const& other) : m_set(other.m_set){};

        /**
           @brief inserts a new instance on the vector
        */
        template <typename T>
        GT_FUNCTION
        void insert(T new_instance)
            {
                // set_t::fuck();
                // T::fuck();
                GRIDTOOLS_STATIC_ASSERT((boost::fusion::result_of::has_key<set_t, T>::type::value),
                                        "the type used for the lookup in the metadata set is not present in the set. Did you use the correct type as meta storage?");
                assert(!present<T>());//must be uninitialized
                boost::fusion::at_key<T>(m_set)=new_instance;
            }

        /**
           @brief returns the raw pointer
        */
        template <typename T>
        GT_FUNCTION
        T const& get()
            {
                GRIDTOOLS_STATIC_ASSERT((boost::fusion::result_of::has_key<set_t, T>::type::value), "Internal error: calling metadata_set::get with a metadata type which has not been defined.");
                assert(present<T>());//must be initialized
                return boost::fusion::at_key<T>(m_set);
            }

        /**@brief returns the sequence by non-const reference*/
        GT_FUNCTION
        set_t& sequence_view() {return m_set;}

        /**@brief returns the sequence by const reference*/
        GT_FUNCTION
        set_t const& sequence_view() const {return m_set;}

        /**@bief queries if the given key corresponds to a pointer which is being initialized*/
        template <typename T>
        GT_FUNCTION
        bool present() {
            GRIDTOOLS_STATIC_ASSERT((boost::fusion::result_of::has_key<set_t, T>::type::value), "internal error: calling metadata_set::present with a metadata type which has not been defined");
            return boost::fusion::at_key<T>(m_set).get();
        }


    };



    template <typename T>
    struct is_metadata_set : boost::mpl::false_{};

    template <typename T>
    struct is_metadata_set<metadata_set<T> > : boost::mpl::true_{};

}
