#pragma once
#include <string>
#include <map>
#include <sstream>
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"


class rtparser {
public:

inline const std::map<std::string, std::string> GetValues() { return m_values; }

inline const void Set(std::string key, std::string val) {  m_values[key] = val; }

inline const void Remove(std::string key, std::string val) { 
    std::map<std::string, std::string>::iterator it = m_values.find(key);

    if (it != m_values.end())
        m_values.erase(it);
}

inline std::string Serialize() {
    std::string text;
    for (auto it = m_values.begin(); it != m_values.end(); it++) {
        text += (it->first + "|" + it->second);
        text += "\n";
    }
    return text;
}

inline std::string Get(std::string key) { return m_values.count(key) > 0 ? m_values[key] : ""; }

inline static rtparser Parse(std::string text) {
    std::stringstream ss(text);
    std::string cline;
    rtparser rtp;


    if (text != "") 
    {
        while (std::getline(ss, cline, '\n')) 
        {
            if (cline == "") continue;

            size_t tk = cline.find('|');
            if (tk == std::string::npos) continue;

            std::string val = "";

            try {
                val = cline.substr(tk + 1);
            } catch (...) {}

            rtp.Set(cline.substr(0, tk), val);
        }
    }

    return rtp;
}


private:
    std::map<std::string, std::string> m_values;
};