#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>

template<typename Key, typename Value> class oMap{
    using DList = typename std::list<std::pair<Key, Value>>;
    DList data;
    using DListIt = typename DList::iterator;
    using LList = std::unordered_map<Key, DListIt>;
    LList locs;
    using LListIt = typename LList::iterator;
    public:
        oMap() = default;
        oMap(oMap&&) = default;
        oMap& operator=(oMap&&) = default;
        oMap(const oMap& other) : data(other.data){
            for(DListIt it = data.begin(); it!=data.end(); ++it){
                locs.emplace(it->first, it);
            }
        }
        oMap& operator=(const oMap& other){
            if(this!=other){
                oMap temp(other);
                data.swap(temp.data);
                locs.swap(temp.locs);
            }
            return *this;
        }
        Value& operator[](const Key& k){
            LListIt found = locs.find(k);
            if(found!=locs.end()){
                return found->second->second;
            }else{
                DListIt entry = data.emplace(data.end(), k, Value{});
                locs.emplace(k, entry);
                return entry->second;
            }
        }
        size_t size() { return data.size(); }
        DListIt begin() { return data.begin(); }
        DListIt end() { return data.end(); }
        bool empty() { return data.empty(); }
        void clear() { locs.clear(); data.clear(); }
        void erase(const Key& k) { 
            LListIt found = locs.find(k);
            if(found!=locs.end()){
                data.erase(found->second);
                locs.erase(found);
            }
        }
};

using IniData = oMap<std::string, oMap<std::string, std::string>>;

int main(){
    IniData test;
    test["hello"]["world"] = "Hello World!";
    for(std::pair<std::string,oMap<std::string, std::string>> cat : test){
        std::cout << "[" << cat.first << "]\n";
        for(std::pair<std::string, std::string> kV : cat.second){
            std::cout << kV.first << " = " << kV.second << "\n";
        }
    }
}
