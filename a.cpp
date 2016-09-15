#include <iostream>
#include <cstdint>

#define isPowerOfTwo(number) (number & (number-1) == 0)

uint64_t take(uint64_t num)
{
    //if is power of 2, return 1
    if (isPowerOfTwo(num))
    {
        return 1;
    }
    //otherwise if number divisible by 2,
    //tail call:
    if (num % 2 == 0)
    {
        return take(num/2);
    }
    else
    {
        uint64_t result = 0;
        auto n = num / 2;
        //essentially two iterations: n and n-1
        for(auto i=n;i>=n-1;--i)
        {
            //construct two halves of binary tree:
            //since we have log(N) space complexity, it's okay to store the whole
            //tree
            //f(2n+1) = f(n) + f(n-1)
            if(isPowerOfTwo(i))
            {
                result += 1;
            }
            uint64_t tree[63];
            auto leafPtr = 0;

        }
    }
}
