#pragma once
#include <iostream>
#include <memory>
#include <algorithm>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <condition_variable>


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
    //mark stored value as valid/invalid. for fast erase.
    std::shared_ptr<bool> validity;
    size_t capacity;
    size_t lastElementPtr;
    size_t validSize;
    //defaults capacity to 32
public:
    KVPairList(size_t capacity) :
        list(new pair<KeyT, ValueT>[capacity], std::default_delete<pair<KeyT, ValueT>[]>()),
        validity(new bool[capacity], std::default_delete<bool[]>()),
        capacity(capacity),
        lastElementPtr(0),
        validSize(0)
    {}
    /**
     * default constructor: bucket size = 32
     */
    KVPairList() :
        KVPairList(32)
    {}

    /**
     * copy constructor for resizing
     *
     * warning: not thread safe
     */
    KVPairList(const KVPairList<KeyT, ValueT> & rhs) :
        list(rhs.list),
        capacity(rhs.capacity),
        lastElementPtr(rhs.lastElementPtr),
        validSize(rhs.validSize)
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
        lastElementPtr = rhs.lastElementPtr;
        validSize = rhs.validSize;

        return *this;
    }

    size_t size()
    {
        //acquire lock:
        std::lock_guard<std::mutex> lock(mutex);
        auto retVal = validSize;
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
        //debug:


        if(lastElementPtr >= capacity)
        {
            resize((size_t)(capacity*1.5));
        }
        //if key exists in list, update
        auto i = indexOf(kv.first);
        if(i != -1)
        {
            list.get()[i].second = kv.second;
            validity.get()[i] = true;
        }
        else
        {
            this->list.get()[lastElementPtr] = kv;
            this->validity.get()[lastElementPtr] = true;
            lastElementPtr++;
            validSize++;
        }
    }

    /**
     * thread safety is not guaranteed
     */
    friend std::ostream& operator<<(std::ostream &stream, const KVPairList& rhs)
    {
        //stream<<"{";
        for(auto i=0;i<rhs.lastElementPtr;++i){
            if(rhs.validity.get()[i])
            stream<<rhs.list.get()[i].first<<":"<<rhs.list.get()[i].second<<", ";
        }
        //stream<<"}";
        return stream;
    }

    void erase(const KeyT &key)
    {
        //erase object with given key by marking validity as invalid
        std::lock_guard<std::mutex> lock(mutex);
        //linear search
        auto i = indexOf(key);
        if(i != -1)
        {
            validity.get()[i] = false;

            --validSize;
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
        auto i = indexOf(key);
        if(i != -1)
        {
            return list.get()[i].second;
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
        for(auto i=0;i<lastElementPtr;++i)
        {
            if(list.get()[i].first == key && validity.get()[i])
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

        if(newCapacity >= lastElementPtr)
        {
            std::shared_ptr<pair<KeyT, ValueT> >
                newList(new pair<KeyT, ValueT>[(size_t)(newCapacity)],
                        std::default_delete<pair<KeyT, ValueT>[]>());
            std::shared_ptr<bool>
                newValidity(new bool[(size_t)newCapacity](),
                            std::default_delete<bool[]>());
            //copy to newlist only the valid entries:
            auto newListPtr = 0;
            for(auto i=0;i<this->lastElementPtr;++i){
                if (validity.get()[i]){
                    newList.get()[newListPtr] = list.get()[i];
                    newValidity.get()[newListPtr] = true;
                    newListPtr++;
                }
            }
            list = newList;
            validity = newValidity;
            lastElementPtr = newListPtr;
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

}//end tsmap ns
