#include "b.hpp"
#include <iostream>
#include <string>

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE threadsafe_map_test

BOOST_AUTO_TEST_SUITE(threadsafe_map_test)

BOOST_AUTO_TEST_CASE(test_single_thread_kvlist)
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;
	for(auto i='A';i<='z';++i)
	{
		pl.upsert(make_pair(std::to_string(i), (int)i));
	}
}


BOOST_AUTO_TEST_SUITE_END()
