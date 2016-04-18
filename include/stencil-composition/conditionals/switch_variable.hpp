#pragma once
#ifdef CXX11_ENABLED
#include <memory>
#else
#include <vector>
#include <boost/scoped_ptr.hpp>
#endif
#include "condition.hpp"

/**@file*/

namespace gridtools {

    /**
       @brief defines a variable which is used in the @ref gridtools::switch_ statement

       The value stored in this object is used to compare with all the different cases defined
       by the user. This class also holds the vector of conditionals which is automatically
       generated by the @ref gridtools::switch_ statement

       NOTE: there is a 1-to-1 relation between switch_variable and switch_ statements. The case in
       which 2 switch_ statements share the same switch_variable will produce an error. This because
       each switch_ statement would try to fill the same m_conditions vector in the switch_variable
       with the list of the cases defined by the user.
       TODO: add a protection for that

       \tparam Tag a unique integer identifying this switch_variable
       \tparam T the type used for the comparisons (usually an integral type)
     */
    template < uint_t Tag, typename T >
    class switch_variable {

        T m_value;
        uint_t m_num_cases;

      public:
        typedef static_uint< Tag > index_t;
        static const uint_t index_value = index_t::value;

#ifdef CXX11_ENABLED
        std::unique_ptr< std::vector< short_t > > m_conditions; // generated conditions
        std::unique_ptr< std::vector< T > > m_cases;            // all possible cases (redundant)
#else
        boost::scoped_ptr< std::vector< short_t > > m_conditions; // generated conditions
        boost::scoped_ptr< std::vector< T > > m_cases;            // all possible cases (redundant)
#endif
        /**@brief enpty constructor*/
        constexpr switch_variable() // try to avoid this?
            : m_value(),
              m_conditions(new std::vector< short_t >()),
              m_cases(std::vector< T >()) {}

        /**@brief constructor

           @param c the value assigned for the comparisons
         */
        constexpr switch_variable(T const &c)
            : m_value(c), m_conditions(new std::vector< short_t >()), m_cases(new std::vector< T >()) {}

        ~switch_variable() {}

        /**@brief API to insert a condition*/
        void push_back_condition(short_t c) { m_conditions->push_back((short_t)c); }
        /**@brief API to insert a case value*/
        void push_back_case(T c) { m_cases->push_back(c); }
        /**@brief returns by non const reference the std::vector of condiitons*/
        std::vector< short_t > &conditions() { return *m_conditions; }
        /**@brief returns by non const reference the std::vector of cases*/
        std::vector< T > &cases() { return *m_cases; }
        /**@brief returns the number of cases for the switch associated to this variable*/
        uint_t num_conditions() { return m_conditions->size(); }

        /**@brief returns the value of the switch_variable*/
        constexpr T value() const { return m_value; }

        void operator=(switch_variable const &other) { reset_conditional(*this, other.value()); }

        void operator=(short_t other) { reset_conditional(*this, other); }

      private:
        switch_variable(switch_variable const &);
    };

    template < typename T >
    struct is_switch_variable : boost::mpl::false_ {};

    template < uint_t Tag, typename T >
    struct is_switch_variable< switch_variable< Tag, T > > : boost::mpl::true_ {};
} // namespace gridtools
