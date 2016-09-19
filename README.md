# Brief description of Exercises

Peter Zhang

Exercise 1 file:
    - recursion.hpp

Exercise 2 files:
    - KVPairList.hpp
    - TSMap.hpp
    - TSMap.cpp (for unit tests)

both exercises are written with -std=c++14

## Exercise 1:

The name of the function that calculates the recursion relationship is called
``calculateRecursionValue'' that takes an uint64 as parameter. There will be a
wrap-around if the value entered is out of range (e.g., UINT64_MAX+1 = 0). Since
the result of the function is going to be smaller than the input, no worries on
that.

The main function tests values for f(n) where n=0..19, 123456789012345678
(example) and UINT64_MAX. The algorithm uses dynamic programming, where the
factors are first decomposed top-bottom, then the values of the factors are
reconstructed bottom-up, which yields the final value of the function. The
actual runtime of the whole main function is less than 0.01s with -O3 flag on
gcc 6 according to $time, which puts the runtime in the ms range. For more
details, please see comments in file.


## Exercise 2:

KVPairList.hpp contains the definition of a templated pair class similar to
std::pair due to the disability to use STL containers, as well as a key-value
pair array where thread safety is guaranteed.

TSMap.hpp contains the definition of the (T)hread (S)afe Map, including
lookup (access of stored value), insertion (upsertion similar to
std::unordered_map::insert_or_assign), and deletion (by key).

TSMap.cpp contains unit tests for KVPairList and TSMap classes using boost's
unit testing framework. **One is required to have boost installed for this
feature to work**.

### thread safety

At the TSMap level, there is no global thread lock, as any given number of
threads can call TSMap class functions at any given time, thus gives us
concurrent access. The map is constructed out of a given number of buckets which
defaults to 128, and any object that can be hashed through std::hash can be used
as key. std::hash allows for thread safety iff its arguments are const, which is
acceptable in our case.

Thus, at the insertion/deletion/lookup phase on the TSMap class level, each
thread can calculate the corresponding hash of the key it wants to use, which
can be done without any race condition or conflict. The hash then determines
which bucket the key object is located in, which is hash % number_of_buckets.
After the corresponding bucket is found, the thread with the key accesses the
bucket, which is an instance of KVPairList, which is an array of key-value
pairs, or where the data is actually stored.

At the KVPairList class level, **all access, including insertion, deletion and
element lookup are atomic in essence**. This is guaranteed by a class-wide mutex
with lock_guard called at the beginning of each public member function. As a
result, a KVPairList is accessed linearly by all threads that happen to have
keys with the same hash % number_of_buckets. This, however, is NOT a global
thread lock, for if the number of buckets is relatively large comparing to the
number of threads, the chances of more than one thread is trying to access the
same bucket is relatively small. Consequently, thread safety is achieved without
having to serialize all read/write actions using a global thread lock at the map
level.

Smart pointers are used in both KVPairList and TSMap classes to construct arrays
of data, which adds the ease of automatic memory management in a multi-threaded
environment.

### resizing
TSMap is not constructed with a fixed size upfront that determines how many
elements can be stored in map, which is a choice by design. The user can specify
the number of buckets the map should have, which essentially could be used to
limit the number of elements in the map if the keys to be inserted happen to
hash to different buckets. Each bucket, or KVPairList object, is responsible for
resizing its own size.  Currently it is designed that each bucket has 32
elements by default, and if a new element is added to a bucket whose size
already reached its previous allocated maximum number, the bucket will increase
its size by 50%, allowing a virtually unlimited number of elements be added to
the map up to the amount of memory available. One can also choose to downsize
the max number of elements of a bucket, but one might argue that the increasing
of size tends to have more applicable situations than vice versa.

### genericity

With templated classes, genericity is easily achieved for all objects that
can be hashed by std::hash. An enhancement to this feature would be an
overloadable template parameter that allows the user to specify a different hash
function, in which case the TSMap class can be used with virtually any type.

### tests

boost's unit test framework is used for tests. Please see TSMap.cpp for details.
