#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>


/**
 * returns true if num is a power of 2
 * otherwise false
 */
template <typename T>
inline bool isPowerOfTwo(const T &num)
{
    T number = num;
    return ((number & (number-1)) == 0);
}

/**
 * returns the greatest factor of num
 * that is odd
 */
template <typename T>
inline T divideByTwoIfEven(const T &num)
{
    T number = num;
    while(number%2==0 && number > 0) {number/=2;}
    return number;
}

/**
 * returns the odd factor for the decomposition relationship
 * f(2n+1) = f(n) + f(n-1)
 */
template <typename T>
inline T getOddFactor(const T &num)
{
    T number = num;
    return ((number / 2) % 2 ? number/2 : (number/2)-1);
}

/**
 * returns the even factor for the decomposition relationship
 * f(2n+1) = f(n) + f(n-1)
 */
template <typename T>
inline T getEvenFactor(const T &num)
{
    T number = num;
    return ((number/2) % 2 ? (number/2)-1 : number/2);
}

/**
 * lookup table for dynamic programming
 */
std::unordered_map<uint64_t, uint64_t> table;



/**
 * calculates f(n) defined by the following recursion relationship:
 *
 * f(0) = 1
 * f(1) = 1
 * f(2n) = f(n)
 * f(2n+1) = f(n) + f(n-1)
 *
 * param: n
 * returns: f(n)
 *
 * please use a valid number in the uint64_t range. note that with the given
 * relationship f(n) will always be smaller than n. there will be a wrap around
 * if integer overflow were to occur, and will still give a valid value
 * (UINT64_MAX+1 = 0)
 */
uint64_t calculateRecursionValue(uint64_t num)
{
    //shortcuts:
    //if number is power of 2, we know that the number is decomposed to 1
    //because f(1) = 1, f(2) = f(2/1) = 1, f(2^2) = f(2^2/2) = f(1) = 1...
    //etc...
    if(isPowerOfTwo(num)) return 1;
    if(table.count(num)) return table[num];

    //make number odd if even
    num = divideByTwoIfEven(num);

    //factor decomposition: top->down
    std::vector<uint64_t> factors;
    factors.push_back(num);

    //root of the search tree(factor decomposition)
    auto root = num;

    //populate list of factors, down to 1, which is a base case of the
    //decomposition
    while(factors.back() > 1)
    {
        factors.push_back(num/2);
        factors.push_back((num/2)-1);
        num = factors.back();
    }

    //build search tree bottom-up, with DP on lookup table
    while(factors.size())
    {
        //take the smallest factor in factors
        auto factor = factors.back();
        factors.pop_back();

        //if value already in lookup table, do not populate value in table again
        if(table.count(factor)) continue;

        //decompose the factors of the current factor
        auto odd = getOddFactor(factor);
        auto even = getEvenFactor(factor);

        //shortcut for when the even factor is power of 2
        if(isPowerOfTwo(even))
        {
            table[even] = 1;
        }
        else
        {
            //reduce the even number to the greatest odd factor, since the
            //contribution is the same given f(2n) = f(n)
            even = divideByTwoIfEven(even);
        }

        //if both factors exist in table, directly look them up and populate the
        //factor in table
        if(table.count(odd) && table.count(even))
        {
            table[factor] = table[odd] + table[even];
        }
        //add odd factor to the next round of calculation if not previously
        //looked-up
        if (!table.count(odd))
        {
            factors.push_back(odd);
        }
        //same process for the even factor.
        if (!table.count(even))
        {
            factors.push_back(even);
        }
    }
    //finally, look up the value in table for root, which is the value we want.
    return table[root];

}

int main()
{
    //initialize table:
    table[0] = 1;
    table[1] = 1;

    //calculate f(n) for n=0..19
    for(auto x = 0; x<20; ++x)
    {
        std::cout<<calculateRecursionValue(x)<<std::endl;
    }

    std::cout<<calculateRecursionValue(123456789012345678)<<std::endl;
    //test for max value
    std::cout<<calculateRecursionValue(UINT64_MAX)<<std::endl;
    return 0;
}
