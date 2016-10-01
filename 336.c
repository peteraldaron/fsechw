class Solution {
public:
    bool isPalindrom(string s){

        for(int i=0;i<s.size()/2;++i){
            if(*(s.begin()+i)!=*(s.rbegin()+i)){
                return false;
            }
        }
        return true;
    }
    
    vector<vector<int>> palindromePairs(vector<string>& words) {
        vector<vector<int> > pairs;
        for(int i=0;i<words.size()-1;++i){
            for(int j=i+1;j<words.size();++j){

                if(isPalindrom(words[i]+words[j])){
                    vector<int> pp={i,j};
                    pairs.push_back(pp);
                }
                if(isPalindrom(words[j]+words[i])){
                    vector<int> pp={j,i};
                    pairs.push_back(pp);
                }
            }
        }
        return pairs;
    }
};
