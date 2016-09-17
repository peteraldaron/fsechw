#include <iostream>
#include <memory>
#include <mutex>
#include <functional>

namespace TSMap
{

/**
 * since stl container is disabled
 * implement our own pair
 */
template <typename FirstT, typename SecondT>
struct pair
{
    FirstT first;
    SecondT second;

    pair(const FirstT& first, const SecondT &second)
        : first(first),
          second(second)
    {}
}

/**
 * similar to std::make_pair
 */
template <typename FirstT, typename SecondT>
pair<FirstT, SecondT> make_pair(
        const FirstT &first,
        const SecondT &second)
{
    return pair<FirstT, SecondT>(first, second);
}

template <typename KeyT>
struct WrappedKeyObject
{
    KeyT key;
    bool isPopulated;
    std::mutex keyLock;

    /**
     * default constructor
     */
    WrappedKeyObject()
        : isPopulated(false)
    {}

    WrappedKeyObject(const KeyT &key)
        : isPopulated(true),
          key(key)
    {}

    /**
     * overloaded assignment operator
     * for copying
     */
    WrappedKeyObject& operator=(const WrappedKeyObject &rhs)
    {
        this->key = rhs.key;
        this->isPopulated = rhs.isPopulated;
    }

    /**
     * copy constructor
     *
     * note: since mutex is not copy-assignable, create a new mutex instance on
     * copying
     */
    WrappedKeyObject(const WrappedKeyObject & rhs)
    {
        key = rhs.key;
        isPopulated = rhs.isPopulated;
    }
};

template <typename KeyT>
WrappedKeyObject<KeyT> make_wrapped_key(const KeyT &key)
{
    return WrappedKeyObject<KeyT>(key);
}



/**
 * a thread safe hashmap
 *
 */

template <typename KeyT, typename ValueT>
class TSMap
{
private:
    std::shared_ptr<ValueT> values;
    std::shared_ptr<WrappedKeyObject<KeyT> > keys;
    std::mutex globalLockMutex;
    size_t tableSize;
    std::hash<KeyT> hashFunc;
public:
    /**
     * default constructor is deleted
     */
    TSMap() = delete;

    TSMap(size_t tableSize)
        : keys(new WrappedKeyObject<KeyT>[tableSize], std::default_delete< WrappedKeyObject<KeyT>[]>()),
          values(new ValueT[tableSize], std::default_delete<ValueT[]>()),
          tableSize(tableSize)
    {}

    ~TSMap()
    {
    }

    void deleteByKey(const KeyT& key)
    {
    }

    /**
     * insert an entry by key to map
     *
     * key and value pairs should have a default/overloaded copying operator=
     * and/or a copying constructor
     */
    void insert(const KeyT &key, const ValueT &value)
    {
        std::unique_lock<std::mutex> glockTest(this->globalLockMutex, std::defer_lock);
        //hash key:
        auto hashKey = hashFunc(key) % tableSize;
        //if entry at corresponding hash key is not populated,
        //store key and value pair
        //TODO: implement thread safety here
        if(!(keys.get()[hashKey].isPopulated))
        {
            *((keys.get()+hashKey)->key) = key;
            *(values.get()+hashKey) = value;
        }
        else
        {
            //is there really a conflict? check actual hash key of key object
            //TODO: implement thread safety here
            auto originalKeyHash = hashFunc(key);
            auto storedKeyHash = hashFunc(keys.get()[hashKey].key);
            if(originalKeyHash == storedKeyHash)
            {
                //key conflict, update stored value object to new value
                *(values.get()+hashKey) = value;
            }
            //else, false positive, consider rehashing
            else
            {
            }

        }

    }
    void lookup(const KeyT &key)
    {
        auto hashKey = hashFunc(key) % tableSize;
        //try to acquire lock on given key

    }
    /**
     * resizing the hashmap to given size
     *
     * this is a blocking call
     */
    void resize(size_t newSize)
    {
        std::unique_lock<std::mutex> globalLock(this->globalLockMutex);
        globalLock.lock();
        rehash(newSize);
        globalLock.unlock();
    }

    /**
     * for debugging only
     * no thread safety guaranteed
     */
    pair<KeyT, ValueT> KVPairAtIndex(size_t index)
    {
        std::unique_lock<std::mutex> lockKey(keys.get()[index].mutex);
        lockKey.lock();
        auto kvpair = make_pair(keys.get()[index].key, values.get()[index]);
        lockKey.unlock();
        return kvpair;
    }

private:
    /**
     * rehash the map according to current table size
     *
     * this operation must be done in a single thread under a lock
     */
    void rehash(size_t newSize)
    {
        //new key and value ptrs:
        std::shared_ptr<ValueT>
            newvals(new ValueT[newSize], std::default_delete<ValueT[]>());

        std::shared_ptr<WrappedKeyObject<KeyT> >
            newkeys(new WrappedKeyObject<KeyT>[newSize],
                    std::default_delete< WrappedKeyObject<KeyT>[]>());


        for(auto i=0; i<tableSize; ++i)
        {
            auto val = this->values.get()[i];
            auto key = this->keys.get()[i];
            //since this is ran on a single thread, acceptable for key to be
            //non-static
            auto hashKey = hashFunc(key.key) % newSize;
            if(newkeys.get()[hashKey].isPopulated)
            {
                //error: key conflict.
            }
            newvals.get()[hashKey] = val;
            newkeys.get()[hashKey] = make_wrapped_key(key.key);
        }

        this->tableSize = newSize;
    }
};
}//end namespace TSMap
