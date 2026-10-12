#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <fstream>
#include <string_view>

template<typename Key, typename Value> class oMap{
    public:
        using DList = typename std::list<std::pair<Key, Value>>;
        DList data;
        using DListIt = typename DList::iterator;
        using LList = std::unordered_map<Key, DListIt>;
        LList locs;
        using LListIt = typename LList::iterator;

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
        DListIt find(const Key& k) {
            LListIt found = locs.find(k);
            if(found!=locs.end()){
                return found->second;
            }else{
                return data.end();
            }
        }
};

std::vector<std::string> split(const std::string& str, const std::string& del, const bool& vP = false, const int& bI = -1){
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
                output.push_back(str.substr(vP ? end : end+1));
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
using Section = oMap<std::string, std::string>;

std::string spaces = " \t\n\r";
std::string comments = ";#";
std::string sectId = "[]";
std::vector<std::string> wL;

IniData read(const std::string& file, bool write = false, IniData wD = IniData{}){
    size_t lKI = 0;
    if(write) wL.clear();
    IniData output;
    std::ifstream f(file);
    if(!f.is_open() && !write){
        std::cerr << "INIRW: Failed to open file " << file << "\n";
        return IniData{};
    }
    std::string line = "";
    std::string section = "";
    std::string lastSect = "";
    bool fR = true;
    while(std::getline(f, line)){
        if(fR){
            fR = false;
            if(line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
        }
        Esc escL = escape(line, '\\', true, comments);
        std::vector<std::string> sL = split(escL.o, comments, true, escL.i);
        std::string rawLine = line;
        line = sL[0];
        std::string comment = "";
        if(sL.size() == 2) comment = rawLine.substr(sL[0].size() + escL.i);
        line = trim(line, spaces);
        if(line=="") {
            if(write) wL.push_back(rawLine);
            continue;
        }
        if(!line.empty() && line.front() == '[' && line.back() == ']'){
            section = trim(line, "[]");
            if(write){
                Section& sectData = wD[lastSect];
                size_t offset = 0;
                for(std::pair<std::string, std::string> kV : sectData){
                    wL.insert(wL.begin()+lKI+offset, kV.first + " = " + kV.second);
                    offset++;
                    // wL.push_back(kV.first + " = " + kV.second);
                }
                wD.erase(lastSect);
                lastSect = section;
                if(wD.find(section)!=wD.end()){
                    wL.push_back("[" + section + "]" + comment);
                    lKI = wL.size();
                }
            }else{
                output[section];
            }
        }else{
            Esc escE = escape(line, '\\', true, "=");
            std::vector<std::string> kV;
            kV = split(escE.o, "=", false, escE.i);
            if(kV.size()==2){
                std::string key = escape(trim(kV[0], spaces), '\\', false, sectId).o;
                std::string val = escape(trim(kV[1], spaces), '\\', false, sectId).o;
                if(write){
                    Section& sectData = wD[section];
                    Section::DListIt found = sectData.find(key);
                    if(found!=sectData.end()){
                        Esc escS = escape(found->second, '\\', true, comments);
                        std::vector<std::string> sV = split(escS.o, comments, true, escS.i);
                        if(val!=trim(sV[0], spaces) || comment!=found->second.substr(sV[0].size() + escS.i)){
                            wL.push_back(found->first + " = " + found->second);
                            sectData.erase(key);
                        }else{
                            wL.push_back(rawLine);
                            sectData.erase(key);
                        }
                    }
                    lKI = wL.size();
                }else{
                    output[section][key] = val;
                }
            }
        }
    }
    if(write){
        for(std::pair<std::string, Section>& sect : wD){
            if(!sect.second.empty()) wL.push_back("[" + sect.first + "]"); 
            for(const std::pair<std::string, std::string>& kV : sect.second){
                wL.push_back(kV.first + " = " + kV.second);
            }
        }
    }
    return output;
}

bool write(const std::string& file, const IniData& data){
    read(file, true, data);
    std::ofstream f(file);
    if(!f.is_open()){
        std::cerr << "INIRW: Failed to open " << file << " for writing";
        return false;
    }
    for(std::string line : wL){
        f << line << "\n";
        std::cout <<line <<"\n";
    }
    f.close();

    return true;
}

int main(){
    IniData data = read("test2.ini");
    // for(std::pair<std::string, oMap<std::string, std::string>> section : data){
    //     std::cout << "[" << section.first << "]\n";
    //     for(std::pair<std::string, std::string> entry : section.second){
    //         std::cout << "KEY: " << entry.first << "," << "VALUE: " << entry.second << "\n";
    //     }
    // }
    data["SECT1"]["k1"] = "v5";
    data["SECT1"]["k3"] = "v6";
    data["SECT1"]["k4"] = "v7";
    data["SECT1"].erase("k2");
    data["SECT2"]["k4"] = "v8 ; Comments can be added like this";

    write("test2.ini", data);

    std::cout << "Hello World!\n";
}
