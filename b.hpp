#include <iostream>
#include <memory>

/**
 * a thread safe hashmap
 *
 */

template <typename KeyT, typename ValueT>
class TSMap
{
private:
    std::shared_ptr<ValueT> values;
    std::shared_ptr<KeyT> keys;
public:
    TSMap(size_t tableSize)
        : keys(new KeyT[tableSize], std::default_delete<KeyT[]>()),
          values(new ValueT[tableSize], std::default_delete<ValueT[]>())
    {}

    void deleteByKey(KeyT & key)
    {
    }

    /**
     * key and value pairs should have a default/overloaded copying operator=
     * and/or a copying constructor
     */
    void insert(KeyT &key, ValueT &value)
    {
    }
    void lookup(KeyT &key)
    {
    }
};
