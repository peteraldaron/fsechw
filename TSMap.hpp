#pragma once
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
        buckets.get()[hashKey].erase(key);
    }

    size_t count(const KeyT& key)
    {
        return buckets.get()[hashFunc(key)%tableSize].count(key);
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
        buckets.get()[hashKey].upsert(make_pair(key, value));
    }

    ValueT& lookup(const KeyT &key)
    {
        auto hashKey = hashFunc(key) % tableSize;
        return buckets.get()[hashKey][key];
    }

    ValueT& operator[](const KeyT &key)
    {
        auto hashKey = hashFunc(key) % tableSize;
        return buckets.get()[hashKey][key];
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
};
}//end namespace TSMap
