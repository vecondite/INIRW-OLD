#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <fstream>

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

std::vector<std::string> split(const std::string& str, const std::string& del, const int& bI = -1){
    std::vector<std::string> output;
    unsigned int it = 0;
    size_t start = 0;
    size_t end = 0;
    while((end=str.find_first_of(del, start))!=std::string::npos){
        if(bI==-1){
            output.push_back(str.substr(start, end-start));
        }else{
            if(it == bI) {
                output.push_back(str.substr(0, end));
                output.push_back(str.substr(end+1));
                return output;
            }
        }
        start = end+1;
        it++;
    }
    if(bI==-1 && start <= str.length()){
        output.push_back(str.substr(start));
    }else{
        output.push_back(str);
    }
    return output;
}

std::pair<std::string, unsigned int> escape(const std::string& str, const char& esc, const std::string_view& filter = ""){
    std::pair<std::string, unsigned int> output;
    output.second = 0;
    bool escaped = false;
    for(const char& c : str){
        if(escaped){
            escaped = false;
            if(filter.find(c)==std::string::npos){
                output.first+=esc;
                output.second++;
            }
            output.first+=c;
        }else if(c == esc){
            escaped = true;
        }else{
            output.first+=c;
        }
    }
    return output;
}

std::string trim(const std::string& str, const std::string& del){
    size_t start = str.find_first_not_of(del);
    if(start==std::string::npos) return "";
    size_t end = str.find_last_not_of(del);
    return str.substr(start, end-start+1);
}

using IniData = oMap<std::string, oMap<std::string, std::string>>;

IniData read(std::string file){
    return IniData{};
}

int main(){
    std::cout << "Hello World!\n";
}
