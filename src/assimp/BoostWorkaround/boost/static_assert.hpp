
#ifndef AI_BOOST_STATIC_ASSERT_INCLUDED
#define AI_BOOST_STATIC_ASSERT_INCLUDED

#ifndef BOOST_STATIC_ASSERT

namespace boost {
	namespace detail {

		template <bool b>  class static_assertion_failure;
		template <>        class static_assertion_failure<true> {};
	}
}


#define BOOST_STATIC_ASSERT(eval) \
assert(eval);
//{boost::detail::static_assertion_failure<(eval)> assert_dummy;assert_dummy;assert(false);}

#endif
#endif // !! AI_BOOST_STATIC_ASSERT_INCLUDED