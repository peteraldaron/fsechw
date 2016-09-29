#include <unordered_map>
#include <deque>
#include <queue>
#include <cstddef>

class Solution {
public:
    int wordDistance(string word1, string word2){
        if(word1.size()!=word2.size()) return INT32_MAX;
        int dist=0;
        for(int i=0;i<word1.size();++i)
        {
            if(word1[i]!=word2[i]){
                dist++;
            }
        }
        return dist;
    }
    vector<vector<string>> findLadders(string beginWord, string endWord, unordered_set<string> &wordList) {
        unordered_map<string, vector<vector<string> > > minPath;
        
        priority_queue<pair<int, string> > q([](const pair<int, string> &a, const pair<int, string> &b){
            return a.first < b.first;
        });
        q.push(make_pair(beginWord, wordDistance(beginWord, endWord)));
        vector<vector<string> > beginwordVector;
        beginwordVector.push_back(vector<string>(1, beginWord));
        minPath[beginWord] = beginwordVector;
        
        while(q.size()){
            auto element = q.top();
            q.pop();
            auto elementVector = minPath[element];
            vector<string> neighbors;
            for(auto w:wordList){
                if(wordDistance(w, element)==1){
                    neighbors.push_back(w);
                }
            }
            
        }
        
    }
};
