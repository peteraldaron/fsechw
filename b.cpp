#include <iostream>
#include <string>
#include <thread>
#include <TSMap.hpp>

#if 1

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define BOOST_TEST_MODULE threadsafe_map_test

BOOST_AUTO_TEST_SUITE(threadsafe_map_test)

//BOOST_AUTO_TEST_CASE(test_single_thread_kvlist_no_resize)
//{
//	using namespace TSMap;
//	using string = std::string;
//	utility::KVPairList<string, int> pl;
//	for(auto i='A';i<='Z';++i)
//	{
//		pl.upsert(::TSMap::make_pair(std::to_string(i), (int)i));
//	}
//}
//
//BOOST_AUTO_TEST_CASE(test_single_thread_kvlist_with_resize)
//{
//	using namespace TSMap;
//	using string = std::string;
//	utility::KVPairList<string, int> pl;
//	for(auto i='A';i<='z';++i)
//	{
//		pl.upsert(::TSMap::make_pair(std::to_string(i), (int)i));
//	}
//}
//
//BOOST_AUTO_TEST_CASE(test_multiple_threads_kvlist_with_resize)
//{
//	using namespace TSMap;
//	using string = std::string;
//	utility::KVPairList<string, int> pl;
//
//	std::thread tr[3];
//	for(int j=0;j<3;++j)
//	{
//		tr[j] = std::thread([&](const int val)
//		{
//			auto id = std::this_thread::get_id();
//			for(auto i='A';i<='z';++i)
//			{
//				pl.upsert(::TSMap::make_pair(std::to_string((char)i)+std::to_string(val), (int)i));
//			}
//		}, j);
//	}
//	for(int j=0;j<3;++j)
//	{
//		tr[j].join();
//	}
//}
//
//BOOST_AUTO_TEST_CASE(test_multiple_threads_kvlist_erase)
//{
//	using namespace TSMap;
//	using string = std::string;
//	utility::KVPairList<string, int> pl;
//
//	std::thread tr[3];
//	std::thread tr2[3];
//	for(int j=0;j<3;++j)
//	{
//		tr[j] = std::thread([&](const int val)
//		{
//			auto id = std::this_thread::get_id();
//			for(auto i='A';i<='z';++i)
//			{
//				pl.upsert(::TSMap::make_pair(std::to_string((char)i)+std::to_string(val), (int)i));
//			}
//		}, j);
//	}
//
//	for(int j=0;j<3;++j)
//	{
//		tr[j].join();
//	}
//
//	for(int j=0;j<3;++j)
//	{
//		tr2[j] = std::thread([&](const int val)
//		{
//			auto id = std::this_thread::get_id();
//			for(auto i='A';i<='z';++i)
//			{
//				pl.erase(std::to_string((char)i)+std::to_string(val));
//			}
//		}, j);
//
//	}
//	for(int j=0;j<3;++j)
//	{
//		tr2[j].join();
//	}
//}
//
//BOOST_AUTO_TEST_CASE(test_multiple_threads_kvlist_erase_with_resize_validity)
//{
//	using namespace TSMap;
//	using string = std::string;
//	utility::KVPairList<string, int> pl(2);
//
//	pl.upsert(::TSMap::make_pair(string("one"), 1));
//	pl.upsert(::TSMap::make_pair(string("two"), 2));
//	pl.upsert(::TSMap::make_pair(string("three"), 3));
//	pl.upsert(::TSMap::make_pair(string("four"), 4));
//	pl.upsert(::TSMap::make_pair(string("five"), 5));
//
//	std::thread t1([&](){pl.erase("one");});
//	std::thread t2([&](){pl.erase("three");});
//	std::thread t3([&](){pl.erase("five");});
//
//	t1.join();
//	t2.join();
//	t3.join();
//
//	BOOST_TEST(pl["two"] == 2);
//	BOOST_TEST(pl["four"] == 4);
//}

BOOST_AUTO_TEST_CASE(TSMap_insert_test)
{
	using string = std::string;
	TSMap::TSMap<string, int> map;
	map.insert("one", 1);
	map.insert("two", 2);
	map.insert("three", 3);
	map.insert("four", 4);
	map.insert("five", 5);
	std::cout<<map<<std::endl;
	map.insert("five", 7);
	std::cout<<map<<std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
#endif
