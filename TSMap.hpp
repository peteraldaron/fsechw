#include <iostream>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <condition_variable>
#include <KVPairList.hpp>

namespace TSMap
{

/**
 * a thread safe hashmap
 *
 */

template <typename KeyT, typename ValueT>
class TSMap
{
private:
    std::shared_ptr<utility::KVPairList<KeyT, ValueT> > buckets;
    size_t tableSize;
    std::hash<KeyT> hashFunc;

public:
    /**
     * default constructor: set bucket size to 128
     */
    TSMap() :
    	buckets(new utility::KVPairList<KeyT, ValueT>[128](),
    			std::default_delete<utility::KVPairList<KeyT, ValueT>[] >()),
		tableSize(128)
	{}

    TSMap(size_t tableSize) :
    	buckets(new utility::KVPairList<KeyT, ValueT>[tableSize](),
    			std::default_delete<utility::KVPairList<KeyT, ValueT>[] >()),
		tableSize(tableSize)
    {}

    ~TSMap()
    {}

    void deleteByKey(const KeyT& key)
    {
        auto hashKey = hashFunc(key) % tableSize;
        auto bucket = buckets.get()[hashKey];
        bucket.erase(key);
    }

    /**
     * insert an entry by key to map
     *
     * key and value pairs should have a default/overloaded copying operator=
     * and/or a copying constructor
     */
    void insert(const KeyT &key, const ValueT &value)
    {
        auto hashKey = hashFunc(key) % tableSize;
        //insert to corresponding bucket
        auto bucket = buckets.get()[hashKey];
        bucket.upsert(make_pair(key, value));
    }

    ValueT& lookup(const KeyT &key)
    {
        auto hashKey = hashFunc(key) % tableSize;
        auto bucket = buckets.get()[hashKey];
        return bucket[key];
    }

    /**
     * thread safety not guaranteed
     * for debugging purpose
     */
	friend std::ostream& operator<<(std::ostream &stream, const TSMap& rhs)
	{
		stream<<"{";
		for(auto i=0;i<rhs.tableSize;++i){
			stream<<rhs.buckets.get()[i];
		}
		stream<<"}";
		return stream;
	}

    /**
     * resizing the number of hashmap buckets to the given size
     *
     * this is a blocking call
     */
    void resize(size_t newSize)
    {
    	auto temp_buckets = buckets;
    	//need to acquire locks on ALL buckets
    	//note: this operation is required before the copying takes place
    	for(auto i=0;i<this->tableSize;++i)
    	{
    		std::lock_guard<std::mutex>(temp_buckets.get()[i].mutex);
    	}

    	//also acquire a lock on buckets object:
    	//this is a global lock that is invoked only when bucket resizing is needed.
    	//std::unique_lock<std::mutex> lock(mutex);
    	//cv.wait(lock);

    	//build new buckets with given size:

    	std::shared_ptr<utility::KVPairList<KeyT, ValueT> >
			newBuckets(new utility::KVPairList<KeyT, ValueT>[newSize],
					std::default_delete<utility::KVPairList<KeyT, ValueT> >());

    	for(auto i=0;i<this->tableSize;++i)
    	{
    		auto bucket = temp_buckets[i];
    		for(auto j=0;j<bucket.size();++j)
    		{
				//rehash:
    			auto kvpair = bucket[j];
				auto hashKey = hashFunc(kvpair.first) % newSize;
				auto newBucket = newBuckets.get()[hashKey];
				newBucket.upsert(kvpair);
    		}
    	}
    	//replace size:
    	this->tableSize = newSize;
    	this->buckets = newBuckets;
    	//release lock on old buckets:
    	for(auto i=0;i<this->tableSize;++i)
    	{
    		temp_buckets.get()[i].lock.unlock();
    	}
    	//old buckets should be destructed automatically
    	//lock.unlock();
    	//cv.notify_all();
    }
};
}//end namespace TSMap
