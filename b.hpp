#include <iostream>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>

#define printList() \
		std::cerr<<"{";\
		for(auto i=0;i<length;++i){\
			std::cerr<<list.get()[i].first<<":"<<list.get()[i].second<<", ";\
		}\
		std::cerr<<"}"<<std::endl;

namespace TSMap
{
/**
 * since stl containers are disabled
 * implement our own pair
 */
template <typename FirstT, typename SecondT>
struct pair
{
    FirstT first;
    SecondT second;

    pair() {}

    pair(const FirstT &first, const SecondT &second)
        : first(first),
          second(second)
    {}

    /**
     * copy constructor
     */
    pair(const pair<FirstT, SecondT> & rhs)
    {
    	this->first = rhs.first;
    	this->second = rhs.second;
    }

    pair<FirstT, SecondT> & operator=(const pair<FirstT, SecondT>& rhs)
    {
    	this->first = rhs.first;
    	this->second = rhs.second;
    	return *this;
    }
};

/**
 * similar to std::make_pair
 */
template <typename FirstT, typename SecondT>
const pair<FirstT, SecondT> make_pair(
        const FirstT &first,
        const SecondT &second)
{
    return pair<FirstT, SecondT>(first, second);
}

namespace utility
{

/**
 * a "bucket" in map for hash function level collision
 * needs to be thread-safe at this level.
 */
template <typename KeyT, typename ValueT>
class KVPairList
{
	std::mutex mutex;
	std::shared_ptr<pair<KeyT, ValueT> > list;
	size_t capacity;
	size_t length;
	//defaults capacity to 32
public:
	KVPairList() :
		list(new pair<KeyT, ValueT>[32], std::default_delete<pair<KeyT, ValueT>[]>()),
		capacity(32),
		length(0)
	{}

	/**
	 * copy constructor for resizing
	 */
	KVPairList(const KVPairList<KeyT, ValueT> & rhs) :
		list(rhs.list),
		capacity(rhs.capacity),
		length(rhs.length)
	{}

	/**
	 * operator= for assignment
	 *
	 */
	KVPairList & operator=(const KVPairList<KeyT, ValueT> rhs)
	{
		//need to wait for mutex on both lists
		std::lock_guard<std::mutex> rlock(rhs.mutex);
		std::lock_guard<std::mutex> lock(mutex);

		list = rhs.list;
		capacity = rhs.capacity;
		length = rhs.length;

		return *this;
	}

	size_t size()
	{
		//acquire lock:
		std::lock_guard<std::mutex> lock(mutex);
		auto retVal = length;
		return retVal;
	}

	/**
	 * insert key-value pair if key didnt exist in list
	 * otherwise update preexisting key's value
	 */
	void upsert(const pair<KeyT, ValueT> &kv)
	{
		//acquire lock:
		std::lock_guard<std::mutex> lock(mutex);

		if(length >= capacity)
		{
			resize((size_t)(capacity*1.5));
		}
		//if key exists in list, update
		auto i = indexOf(kv.first);
		if(i != -1)
		{
			std::cerr<<"key \""<<kv.first<<"\" found at "<<i<<", whose value is \""<<list.get()[i].second<<"\""<<std::endl;
			list.get()[i].second = kv.second;
		}
		else
		{
			this->list.get()[length++] = kv;
		}
	}
	void erase(const KeyT &key)
	{
		//erase object with given key and "move" all elements after removed element
		std::lock_guard<std::mutex> lock(mutex);
		//linear search
		if(auto i = indexOf(key) != -1)
		{
			for(auto j=i;j<length-1;++j)
			{
				std::cerr<<"index found at "<<i<<std::endl;
				list.get()[j] = list.get()[j+1];
			}
			--length;
		}
		//if element not found in list, it is considered to be ``removed''
	}

	/**
	 * return 1 if key exists in list
	 * 0 otherwise
	 */
	size_t count(const KeyT &key)
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto count = indexOf(key) == -1 ? 0 : 1;
		return count;
	}


	/**
	 * get object with given key
	 */
	ValueT & operator[](const KeyT & key)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if(auto i = indexOf(key) != -1)
		{
			auto retVal = list.get()[i];
			return retVal;
		}
		//else:
		//throw error:
		throw new std::invalid_argument("invalid key given in KVList");
	}

	friend class TSMap;

private:
	/**
	 * return index of key-value pair if key exists in list
	 * -1 otherwise
	 *
	 * not thread safe
	 */

	size_t indexOf(const KeyT &key)
	{
		auto index = -1;
		for(auto i=0;i<length;++i)
		{
			if(list.get()[i].first == key)
			{
				index = i;
				break;
			}
		}
		return index;
	}
	void resize(size_t newCapacity)
	{
		//do nothing
		if(newCapacity == capacity) return;

		if(newCapacity >= length)
		{
			std::shared_ptr<pair<KeyT, ValueT> >
				newList(new pair<KeyT, ValueT>[(size_t)(newCapacity)],
						std::default_delete<pair<KeyT, ValueT>[]>());
			//copy to newlist:
			for(auto i=0;i<this->length;++i){
				newList.get()[i] = list.get()[i];
			}
			list = newList;
			this->capacity = newCapacity;
		}
		else
		{
			//throw error
			throw new std::invalid_argument(
					"new capacity in KVPairList::resize is smaller than current length");
		}
	}
};

}//end utility namespace


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
    	buckets(new utility::KVPairList<KeyT, ValueT>[128],
    			std::default_delete<utility::KVPairList<KeyT, ValueT>[] >()),
		tableSize(128)
	{}

    TSMap(size_t tableSize) :
    	buckets(new utility::KVPairList<KeyT, ValueT>[tableSize],
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
    }
};
}//end namespace TSMap
