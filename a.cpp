#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>

#define isPowerOfTwo(number) ((number & (number-1)) == 0)

#define divideByTwoIfEven(number) while(number%2==0 && number > 0) {number/=2;}

#define getOddFactor(number) ((number / 2) % 2 ? number/2 : (number/2)-1)

#define getEvenFactor(number) ((number/2) % 2 ? (number/2)-1 : number/2)

std::unordered_map<uint64_t, uint64_t> table;
using namespace std;

uint64_t take(uint64_t num)
{
    //shortcuts:
    if(isPowerOfTwo(num)) return 1;
    if(table.count(num)) return table[num];

    //make number odd if even
    divideByTwoIfEven(num);

    //factor decomposition:
    std::vector<uint64_t> factors;
    factors.push_back(num);

    auto root = num;

    while(factors.back() > 1)
    {
        factors.push_back(num/2);
        factors.push_back((num/2)-1);
        num = factors.back();
    }
    //for(auto i:factors)
    //    cerr<<i<<endl;
    //build search tree bottom-up, with DP on lookup table
    while(factors.size())
    {
        auto factor = factors.back();
        factors.pop_back();

        if(table.count(factor)) continue;

        auto odd = getOddFactor(factor);
        auto even = getEvenFactor(factor);

        //cerr<<factor<<"is power of 2:"<<even<<" "<<isPowerOfTwo(even)<<endl;

        if(isPowerOfTwo(even))
        {
            table[even] = 1;
        }
        else
        {
            divideByTwoIfEven(even);
        }

        if(table.count(odd) && table.count(even))
        {
            table[factor] = table[odd] + table[even];
        }
        if (!table.count(odd))
        {
            factors.push_back(odd);
        }
        if (!table.count(even))
        {
            factors.push_back(even);
        }
    }
    //for(auto i:table)
    //    cerr<<i.first<<" "<<i.second<<endl;
    return table[root];

}

int main()
{
    //initialize table:
    table[0] = 1;
    table[1] = 1;
    //table[2] = 1;
    //for(auto x = 0; x<20; ++x)
    //{
    //    std::cout<<take(x)<<std::endl;
    //}
    //std::cout<<take(5)<<std::endl;
    std::cout<<take(123456789012345678)<<std::endl;
    return 0;
}
