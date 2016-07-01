
#pragma once

/**@file storage list for expandable parameters*/

namespace gridtools {

    /**
       @brief the storage objects containing expandable parameters

       The expandable parameters are long lists of storages on which the same stencils are applied,
       in a Single-Stencil-Multiple-Storage way. In order to avoid resource contemption usually
       it is convenient to split the execution in multiple stencil, each stencil operating on a chunk
       of the list. Say that we have an expandable parameters list of length 23, and a chunk size of
       4, we'll execute 5 stencil with a "vector width" of 4, and one stencil with a "vector width"
       of 3 (23%4).

       This storage class allows to aggregate the parameters in a single storage list
       with a static size. It is used at the user level to collect the list of parameters, and it is
       used internally by the library as well, to store the chunks of the storage list which go to
       each stencil computation.
    */
    template < typename Storage, uint_t Size >
    struct expandable_parameters : field< Storage, Size >::type {

        typedef typename field< Storage, Size >::type super;
        typedef typename super::traits traits;
        typedef typename super::basic_type basic_type;
        typedef typename super::pointer_type pointer_type;

        using typename super::data_field;

      public:
        template < typename PT, typename MD, ushort_t FD >
        using type_tt = expandable_parameters< typename super::template type_tt< PT, MD, Size >, Size >;

        // public methods:

        /**
           @brief constructor which by default does not allocate the storage

           This constructor does not allocate the fields array by default (which is not the
           standard behaviour in GridTools constructors)

           \param meta_ the @ref gridtools::storage_info
           \param name a character array identifying the storage
           \param do_allocate_ boolean stating whether the constructor should allocate the fields or not
         */
        expandable_parameters(typename super::storage_info_type const &meta_,
            char const *name = "default expandable param",
            bool do_allocate_ = false)
            : super(meta_, name, do_allocate_ /*do not allocate*/) {}

        // __device__ expandable_parameters(expandable_parameters const &other) : super(other) {}

        /**
           @brief assign a chunk of the pointers array from a large storage list to a smaller one (i.e. this one).

           @param a the larger storage field than this one
           @offset the offsets at which the copy starts
         */
        template < typename Storage2 >
        void set(std::vector< pointer< Storage2 > > const &other, ushort_t const &offset) {

            // GRIDTOOLS_STATIC_ASSERT((OtherSize >= Size), "Cannot assign pointers from a smaller storage");
            for (ushort_t i = 0; i < this->field_dimensions; ++i)
                if (offset + i < other.size()) {
                    this->m_fields[i] = other.at(offset + i)->fields()[0];
                }
            this->is_set = true;
        }

        /**
            @brief access an element of the storage

            the first index is relative to the storage list dimension
         */
        template < typename... UInt >
        typename super::value_type &operator()(uint_t const &dim, UInt const &... idx) {
            assert(this->m_meta_data.index(idx...) < this->m_meta_data.size());
            assert(this->is_set);
            return this->m_fields[this->m_meta_data.index(idx...)];
        }

        /**
           @brief copy constructor doing casts between expandable parameters of different sizes

           A larger storage list can be casted to a smaller one, not vice versa.
         */
        template < ushort_t OtherSize >
        expandable_parameters(expandable_parameters< Storage, OtherSize > const &other) {

            GRIDTOOLS_STATIC_ASSERT((OtherSize >= Size), "Cannot assign pointers from a smaller storage");
            for (ushort_t i = 0; i < Size; ++i)
                this->m_fields[i] = other.fields()[i];
        }
    };

    template < typename Storage, uint_t Size >
    struct is_storage< expandable_parameters< Storage, Size > > : boost::mpl::true_ {};

    template < typename T >
    struct is_expandable_parameters : boost::mpl::false_ {};

    template < typename Storage, uint_t Size >
    struct is_expandable_parameters< expandable_parameters< Storage, Size > > : boost::mpl::true_ {};

} // namespace gridtools
