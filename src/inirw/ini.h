#ifndef INIRW_H
#define INIRW_H

#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <fstream>
#include <string_view>

namespace INIRW{
    template<typename Key, typename Value> class oMap{
        public:
            using DList = typename std::list<std::pair<Key, Value>>;
            using DListIt = typename DList::iterator;
            using CDListIt = typename DList::const_iterator;
            using LList = std::unordered_map<Key, DListIt>;
            using LListIt = typename LList::iterator;
        private:
            DList data;
            LList locs;
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
            size_t size() const { return data.size(); }
            DListIt begin() { return data.begin(); }
            DListIt end() { return data.end(); }
            CDListIt begin() const { return data.begin(); }
            CDListIt end()  const { return data.end(); }
            bool empty() const { return data.empty(); }
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
            CDListIt find(const Key& k) const {
                LListIt found = locs.find(k);
                if(found!=locs.end()){
                    return found->second;
                }else{
                    return data.end();
                }
            }
    };

    inline std::vector<std::string> split(const std::string& str, const std::string& del, bool vP = false, int bI = -1){
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

    inline Esc escape(const std::string& str, char esc, bool bF, const std::string_view& filter = ""){
        Esc output;
        bool escaped = false;
        size_t filtLoc = 0;
        bool broken = false;
        for(char c : str){
            filtLoc = filter.find(c);
            if(escaped){
                escaped = false;
                if(filtLoc==std::string::npos){
                    output.o+=esc;
                }else{
                    if(!broken) output.i++;
                }
                if(c!=esc) output.o+=c;
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

    inline std::string trim(const std::string& str, const std::string& del){
        size_t start = str.find_first_not_of(del);
        if(start==std::string::npos) return "";
        size_t end = str.find_last_not_of(del);
        return str.substr(start, end-start+1);
    }

    using IniData = oMap<std::string, oMap<std::string, std::string>>;
    using Section = oMap<std::string, std::string>;

    inline std::string spaces = " \t\n\r";
    inline std::string comments = ";#";
    inline std::string sectId = "[]";
    inline std::string tEsc = spaces + comments + sectId + '\\';

    inline std::string rescape(const std::string& str, const std::string& del){
        std::string output;
        for(char c : str){
            if(del.find(c)!=std::string::npos){
                output+="\\";
            }
            output+=c;
        }
        return output;
    }

    inline IniData read(const std::string& file, bool write = false, IniData wD = IniData{}, std::vector<std::string>* wL = nullptr){
        size_t lKI = 0;
        if(write && wL==nullptr){
            std::cerr << "Cannot write without a vector pointer!" << "\n";
            return IniData{};
        }
        if(write) wL->clear();
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
                if(line.size()>=3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            }
            Esc escL = escape(line, '\\', true, comments);
            std::vector<std::string> sL = split(escL.o, comments, true, escL.i);
            std::string rawLine = line;
            line = sL[0];
            std::string comment = "";
            if(sL.size() == 2) comment = rawLine.substr(sL[0].size() + escL.i);
            line = trim(line, spaces);
            if(line=="") {
                if(write) wL->push_back(rawLine);
                continue;
            }
            if(!line.empty() && line.front() == '[' && line.back() == ']'){
                section = trim(line, "[]");
                if(write){
                    Section& sectData = wD[lastSect];
                    size_t offset = 0;
                    for(const std::pair<std::string, std::string>& kV : sectData){
                        wL->insert(wL->begin()+lKI+offset, rescape(kV.first, tEsc) + " = " + rescape(kV.second, tEsc));
                        offset++;
                        // wL.push_back(kV.first + " = " + kV.second);
                    }
                    wD.erase(lastSect);
                    lastSect = section;
                    if(wD.find(section)!=wD.end()){
                        wL->push_back("[" + section + "]" + comment);
                        lKI = wL->size();
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
                                wL->push_back(rescape(found->first, tEsc) + " = " + rescape(found->second, tEsc) + comment);
                                sectData.erase(key);
                            }else{
                                wL->push_back(rawLine);
                                sectData.erase(key);
                            }
                        }
                        lKI = wL->size();
                    }else{
                        output[section][key] = val;
                    }
                }
            }
        }
        if(write){
            for(std::pair<std::string, Section>& sect : wD){
                if(!sect.second.empty() && lastSect!=sect.first) wL->push_back("[" + sect.first + "]"); 
                for(const std::pair<std::string, std::string>& kV : sect.second){
                    wL->push_back(rescape(kV.first, tEsc) + " = " + rescape(kV.second, tEsc));
                }
            }
        }
        return output;
    }

    inline bool write(const std::string& file, const IniData& data){
        std::vector<std::string> wL;
        read(file, true, data, &wL);
        std::ofstream f(file);
        if(!f.is_open()){
            std::cerr << "INIRW: Failed to open " << file << " for writing";
            return false;
        }
        for(const std::string& line : wL){
            f << line << "\n";
        }
        f.close();

        return true;
    }
}

#endif
