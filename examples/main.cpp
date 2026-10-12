#include "../src/inirw/ini.h"

int main(){
    // READING DATA
    INIRW::IniData data = INIRW::read("test.ini");

    // ITERATION
    for(const std::pair<std::string, INIRW::Section>& sect : data){
        std::cout << "SECTION NAME: " << sect.first << "\n";
        for(const std::pair<std::string, std::string>& kV : sect.second){
            std::cout << "Key: " << kV.first << " || " << "Value: " << kV.second << "\n";
        }
    }

    // CHANGING VALUES
    data["SECT1"]["k1"] = "v5";
    data["SECT1"]["k3"] = "v11";
    data["SECT1"]["k4"] = "v7";
    data["SECT2"]["k4"] = "v8 ; Comments can be added like this";
    data["SECT3"]["k1"] = "v10";

    // ERASING VALUES
    data["SECT1"].erase("k2");

    // UPDATING / GENERATING
    bool updated = INIRW::write("test.ini", data);
    std::cout << (updated ? "File updated!" : "File not updated!") << "\n";
}
