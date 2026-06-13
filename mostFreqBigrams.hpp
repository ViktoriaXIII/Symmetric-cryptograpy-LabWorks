#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;

namespace BigramAnalyzer {
    inline string letter_filter_wind1251(const string& raw, bool keep_spaces = false) {
        string filtered = "";
        for (unsigned char c : raw) {
            if (c >= 192 && c <= 223) { // Велика літера -> в малу
                filtered += static_cast<char>(c + 32);
            }
            else if (c >= 224 && c <= 255) { // Мала літера
                filtered += static_cast<char>(c);
            }
            else if (c == ' ' && keep_spaces) {
                filtered += static_cast<char>(c);
            }
        }
        return filtered;
    }

    inline string is_cyrillic_utf8(unsigned char b1, unsigned char b2) {
        string res = "";
        if (b1 == 0xD0 && b2 == 0x81) { // 'Ё' -> 'ё'
            res += (char)0xD1; res += (char)0x91;
            return res;
        }
        if (b1 == 0xD1 && b2 == 0x91) { // 'ё'
            res += (char)b1; res += (char)b2;
            return res;
        }
        if (b1 == 0xD0 && (b2 >= 0x90 && b2 <= 0x9F)) { // 'А'-'П' -> 'а'-'п'
            res += (char)0xD0; res += (char)(b2 + 0x20);
            return res;
        }
        if (b1 == 0xD0 && (b2 >= 0xA0 && b2 <= 0xAF)) { // 'Р'-'Я' -> 'р'-'я'
            res += (char)0xD1; res += (char)(b2 - 0x20);
            return res;
        }
        if ((b1 == 0xD0 && (b2 >= 0xB0 && b2 <= 0xBF)) || (b1 == 0xD1 && (b2 >= 0x80 && b2 <= 0x8F))) { // 'а'-'я'
            res += (char)b1; res += (char)b2;
            return res;
        }
        return ""; // Не кирилиця
    }

    inline vector<string> letter_filter_utf8(const string& text, bool keep_spaces = false) {
        vector<string> result;
        for (size_t i = 0; i < text.length(); ) {
            unsigned char c1 = static_cast<unsigned char>(text[i]);
            if (c1 == 0x20) {  // Пробіл
                if (keep_spaces) result.push_back(" ");
                i += 1;
            }
            else if (i + 1 < text.length()) {
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                std::string lower_char = is_cyrillic_utf8(c1, c2);
                if (!lower_char.empty()) {
                    result.push_back(lower_char);
                    i += 2;
                }
                else {
                    i += 1;
                }
            }
            else {
                i += 1;
            }
        }
        return result;
    }

    template <typename T>
    inline map<string, long long> bigram_counter(const T& data) {
        map<string, long long> counts;
        if (data.size() < 2) return counts;
        for (size_t i = 0; i < data.size() - 1; ++i) { // "-1", бо остання літера не матиме "правої" пари 
            string bi = "";
            bi += data[i];
            bi += data[i + 1];
            counts[bi]++;
        }
        return counts;
    }
    // --- ТОП-5 біграм ---
    inline vector<string> get_top_5_bigrams_win1251(const string& raw_text) {
        auto filtered = letter_filter_wind1251(raw_text, false);
        auto counts = bigram_counter(filtered);
        vector<pair<string, long long>> sorted_v(counts.begin(), counts.end());
        sort(sorted_v.begin(), sorted_v.end(), [](const auto& a, const auto& b) {return a.second > b.second; });
        vector<string> top_5;
        for (size_t i = 0; i < sorted_v.size() && i < 5; ++i) top_5.push_back(sorted_v[i].first);
        return top_5;
    }

    inline vector<string> get_top_5_bigrams_utf8(const string& raw_text) {
        // Без пробілів
        auto filtered = letter_filter_utf8(raw_text, false);
        auto counts = bigram_counter(filtered);
        vector<pair<string, long long>> sorted_v(counts.begin(), counts.end());
        sort(sorted_v.begin(), sorted_v.end(), [](const auto& a, const auto& b) {return a.second > b.second; });
        vector<string> top_5;
        for (size_t i = 0; i < sorted_v.size() && i < 5; ++i) top_5.push_back(sorted_v[i].first);
        return top_5;
    }
}