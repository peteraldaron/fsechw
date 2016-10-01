//this should work:

class Solution {
public:
    vector<pair<int, int>> reconstructQueue(vector<pair<int, int>>& people) {
        if(!people.size()) return people;
        if(people.size()==1) return people;
        
        std::sort(people.begin(), people.end(), [&](const pair<int,int> & p1, const pair<int,int> & p2){
            return p1.first==p2.first? p1.second<p2.second : p1.first<p2.first;
        });
        for(int i=0;i<people.size()-1;++i){
            for(int j=0;j<people.size()-1;++j){
                if(people[j].second>j){
                    std::swap(people[j], people[j+1]);
                }
            }
        }
        return people;
    }
};
