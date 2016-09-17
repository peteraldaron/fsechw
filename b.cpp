#include "b.hpp"
#include <iostream>
#include <string>
#include <thread>

#if 1

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE threadsafe_map_test

BOOST_AUTO_TEST_SUITE(threadsafe_map_test)

BOOST_AUTO_TEST_CASE(test_single_thread_kvlist_no_resize)
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;
	for(auto i='A';i<='Z';++i)
	{
		pl.upsert(::TSMap::make_pair(std::to_string(i), (int)i));
	}
}

BOOST_AUTO_TEST_CASE(test_single_thread_kvlist_with_resize)
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;
	for(auto i='A';i<='z';++i)
	{
		pl.upsert(::TSMap::make_pair(std::to_string(i), (int)i));
	}
}

BOOST_AUTO_TEST_CASE(test_multiple_threads_kvlist_with_resize)
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;

	std::thread tr[3];
	for(int j=0;j<3;++j)
	{
		tr[j] = std::thread([&](const int val)
		{
			auto id = std::this_thread::get_id();
			for(auto i='A';i<='z';++i)
			{
				pl.upsert(::TSMap::make_pair(std::to_string((char)i)+std::to_string(val), (int)i));
			}
		}, j);
	}
	for(int j=0;j<3;++j)
	{
		tr[j].join();
	}
}

BOOST_AUTO_TEST_CASE(test_multiple_threads_kvlist_erase)
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;

	std::thread tr[3];
	std::thread tr2[4];
	for(int j=0;j<3;++j)
	{
		tr[j] = std::thread([&](const int val)
		{
			auto id = std::this_thread::get_id();
			for(auto i='A';i<='z';++i)
			{
				pl.upsert(::TSMap::make_pair(std::to_string((char)i)+std::to_string(val), (int)i));
			}
		}, j);
	}
	for(int j=0;j<3;++j)
	{
		tr[j].join();
		tr2[j].join();
	}
	tr2[3].join();
}


BOOST_AUTO_TEST_SUITE_END()
#endif

#if 0
int main()
{
	using namespace TSMap;
	using string = std::string;
	utility::KVPairList<string, int> pl;
	for(auto i='A';i<='z';++i)
	{
		std::cout<<i<<std::endl;
		pl.upsert(::TSMap::make_pair(std::to_string(i), (int)i));
	}
	return 0;
}
#endif
