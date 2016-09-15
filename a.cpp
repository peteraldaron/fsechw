#include <iostream>
#include <cstdint>

uint64_t take(uint64_t num)
{
    //if is power of 2, return 1
    if (num & (num-1)){
        return 1;
    }
}
