#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <fstream>
#include <string_view>

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
            if(this!=&other){
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
    bool pB = (bI==-1);
    if(pB) output.reserve(2);
    int it = 0;
    size_t start = 0;
    size_t end = 0;
    while((end=str.find_first_of(del, start))!=std::string::npos){
        if(pB){
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
    if(pB && start <= str.length()){
        output.push_back(str.substr(start));
    }else{
        output.push_back(str);
    }
    return output;
}

struct Esc{
    std::string o = "";
    unsigned int i = 0;
};

Esc escape(const std::string& str, const char& esc, bool bF, const std::string_view& filter = ""){
    Esc output;
    bool escaped = false;
    size_t filtLoc = 0;
    bool broken = false;
    for(const char& c : str){
        filtLoc = filter.find(c);
        if(escaped){
            escaped = false;
            if(filtLoc==std::string::npos){
                output.o+=esc;
            }else{
                if(!broken) output.i++;
            }
            output.o+=c;
        }else if(c == esc){
            escaped = true;
        }else{
            if(!broken && filtLoc!=std::string::npos && bF) broken = true;
            output.o+=c;
        }
    }
    if(escaped) output.o += esc;
    return output;
}

std::string trim(const std::string& str, const std::string& del){
    size_t start = str.find_first_not_of(del);
    if(start==std::string::npos) return "";
    size_t end = str.find_last_not_of(del);
    return str.substr(start, end-start+1);
}

using IniData = oMap<std::string, oMap<std::string, std::string>>;

std::string spaces = " \t\n\r";
std::string comments = ";#";
std::string sectId = "[]";

IniData read(const std::string& file){
    IniData output;
    std::ifstream f(file);
    if(!f.is_open()){
        std::cerr << "INIRW: Failed to open file " << file << "\n";
        return IniData{};
    }
    std::string line = "";
    std::string section = "";
    bool fR = true;
    while(std::getline(f, line)){
        if(fR){
            fR = false;
            if(line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
        }
        Esc escL = escape(line, '\\', true, comments);
        line = split(escL.o, comments, escL.i)[0];
        line = trim(line, spaces);
        if(line=="") continue;
        if(!line.empty() && line.front() == '[' && line.back() == ']'){
            section = trim(line, "[]");
            output[section];
        }else{
            Esc escE = escape(line, '\\', true, "=");
            std::vector<std::string> kV;
            kV = split(escE.o, "=", escE.i);
            if(kV.size()==2){
                std::string key = escape(trim(kV[0], spaces), '\\', false, sectId).o;
                std::string val = escape(trim(kV[1], spaces), '\\', false, sectId).o;
                output[section][key] = val;
            }
        }
    }
    return output;
}

int main(){
    IniData data = read("test.ini");
    for(std::pair<std::string, oMap<std::string, std::string>> section : data){
        std::cout << "[" << section.first << "]\n";
        for(std::pair<std::string, std::string> entry : section.second){
            std::cout << "KEY: " << entry.first << "," << "VALUE: " << entry.second << "\n";
        }
    }
    std::cout << "Hello World!\n";
}
