#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;
int main()
{
    std::hash<string> h;
    std::vector<int> e;
    for(auto i='A';i<'z';++i)
    {
        e.push_back(h(to_string(i)) % 100);
    }
    std::sort(e.begin(), e.end());
    for (auto a:e){
        std::cout<<a<<std::endl;
    }
    return -1;
}
