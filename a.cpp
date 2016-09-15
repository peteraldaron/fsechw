#include <iostream>
#include <cstdint>
#include <vector>

#define isPowerOfTwo(number) (number & (number-1) == 0)

uint64_t take(uint64_t num)
{
    //if is power of 2 or if is smaller than 3, return 1
    if(isPowerOfTwo(num) || num < 3)
    {
        return 1;
    }
    //otherwise if number divisible by 2,
    //tail call:
    if(num % 2 == 0)
    {
        return take(num/2);
    }
    else
    {
        //here, number must not be divisible by 2 due to the previous condition
        uint64_t result = 0;
        auto ptr = 0;

        //do a out-of-sequence tree traversal....
        //std::vector<uint64_t> q;
        //q.push_back(num);
        uint64_t q[64];
        q[ptr++] = num;

        while(ptr>0)
        {
            uint64_t front = q[--ptr];

            if(isPowerOfTwo(front) || front<3)
            {
                result+=1;
                continue;
            }

            uint64_t n = front / 2;

            //essentially two subtrees: n and n-1
            //one of the two is divisible by 2
            auto even = n % 2 ? n-1 : n;
            auto odd = even == n ? n-1 : n;
            //std::cerr<<odd<<" "<<even<<" ptr:"<<ptr<<std::endl;
            //left element (even)
            if(isPowerOfTwo(even) || even<3)
            {
                result+=1;
            }
            else{
                //divide the even element by 2 until no longer is even:
                while(!(even % 2)) even/=2;
                if(even<3)
                {
                    result+=1;
                }
                //push to next wavefront
                else
                {
                    q[ptr++] = even;
                    //q.push_back(even);
                }
            }
            //right element (odd):
            if(odd<3)
            {
                result+=1;
            }
            else
            {
                //q.push_back(odd);
                q[ptr++] = odd;
            }
        }
        return result;
    }
}

int main()
{
    //for(auto x = 0; x<20; ++x)
    //{
    //    std::cout<<take(x)<<std::endl;
    //}
    std::cout<<take(123456789012345678)<<std::endl;
    return 0;
}
