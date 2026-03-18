#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

string letter_filter_wind1251(const string& raw, bool keep_spaces) {
    string filtered = "";
    for (unsigned char c : raw) {
        if (c >= 192 && c <= 223) { // Велика літера -> в малу
            filtered += (char)(c + 32);
        }
        else if (c >= 224 && c <= 255) { // Мала літера
            filtered += (char)c;
        }
        else if (c == ' ' && keep_spaces) {
            filtered += (char)c;
        }
    }
    return filtered;
}

string is_cyrillic_utf8(unsigned char b1, unsigned char b2) {
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
    if (b1 == 0xD0 && (b2 >= 0xA0 && b2 <= 0xAF)) { // 'Р'-'Я' -> 'р'-'я' (D1 80-8F)
        res += (char)0xD1; res += (char)(b2 - 0x20);
        return res;
    }
    if ((b1 == 0xD0 && (b2 >= 0xB0 && b2 <= 0xBF)) || (b1 == 0xD1 && (b2 >= 0x80 && b2 <= 0x8F))) { // 'а'-'я'
        res += (char)b1; res += (char)b2;
        return res;
    }
    return ""; // Не кирилиця
}

vector<string> letter_filter_utf8(const string& text, bool keep_spaces) {
    vector<string> result;
    for (size_t i = 0; i < text.length(); ) {
        unsigned char c1 = (unsigned char)text[i];
        if (c1 == 0x20) { // Пробіл
            if (keep_spaces) result.push_back(" ");
            i += 1;
        }
        else if (i + 1 < text.length()) {
            unsigned char c2 = (unsigned char)text[i + 1];
            string lower_char = is_cyrillic_utf8(c1, c2);
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
map<typename T::value_type, long long> monogram_counter(const T& data) {
    map<typename T::value_type, long long> counts;
    for (const auto& item : data) {
        counts[item]++;
    }
    return counts;
}

template <typename T>
map<string, long long> bigram_counter(const T& data) {
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

template <typename T>
double entropy(const map<T, long long>& counts, int n) {
    long long total = 0;
    for (auto const& [item, count] : counts) total += count;

    double h = 0;
    for (auto const& [item, count] : counts) {
        double p = (double)count / total;
        h -= p * log2(p);
    }
    return h / n;
}

int main(){
    string filename_wind1251 = "text_for_entropy_Windows.txt"; // Файл > 1Мб
    ifstream file_wind1251(filename_wind1251, ios::binary);
    if (!file_wind1251.is_open()) {
        cerr << "ERROR!!! File cannot be open" << endl;
        return 1;
    }
    string raw_content_wind1251((istreambuf_iterator<char>(file_wind1251)), istreambuf_iterator<char>());
    file_wind1251.close();
    cout << fixed << setprecision(6);

    cout << "--- Results for Windows-1251 ---" << endl << endl;
    string text_with_sp_wind1251 = letter_filter_wind1251(raw_content_wind1251, true);
    auto mono_sp_wind1251 = monogram_counter(text_with_sp_wind1251);
    auto bi_sp_wind1251 = bigram_counter(text_with_sp_wind1251);
    cout << "With gapping:" << endl;
    cout << "H_1 (monograms): " << entropy(mono_sp_wind1251, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_sp_wind1251, 2) << " bit/symb" << endl;
    string text_no_sp_wind1251 = letter_filter_wind1251(raw_content_wind1251, false);
    auto mono_no_wind1251 = monogram_counter(text_no_sp_wind1251);
    auto bi_no_wind1251 = bigram_counter(text_no_sp_wind1251);
    cout << "Without gapping:" << endl;
    cout << "H_1 (monograms): " << entropy(mono_no_wind1251, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_no_wind1251, 2) << " bit/symb" << endl;

    string filename_utf8 = "text_for_entropy_UTF.txt"; // Файл > 1Мб
    ifstream file_utf(filename_utf8, ios::binary);
    if (!file_utf.is_open()) {
        cerr << "ERROR!!! File cannot be opened: " << filename_utf8 << endl;
        return 1;
    }
    string raw_content_utf8((istreambuf_iterator<char>(file_utf)), istreambuf_iterator<char>());
    file_utf.close();
    // Видаляємо BOM (Byte Order Mark), якщо він присутній (перші 3 байти UTF-8 файлу)
    if (raw_content_utf8.size() >= 3 &&
        (unsigned char)raw_content_utf8[0] == 0xEF &&
        (unsigned char)raw_content_utf8[1] == 0xBB &&
        (unsigned char)raw_content_utf8[2] == 0xBF) {
        raw_content_utf8.erase(0, 3);
    }
    cout << fixed << setprecision(6);

    cout << "--- Results for UTF-8 ---" << endl << endl;
    vector<string> text_with_sp_utf8 = letter_filter_utf8(raw_content_utf8, true);
    auto mono_sp_utf8 = monogram_counter(text_with_sp_utf8);
    auto bi_sp_utf8 = bigram_counter(text_with_sp_utf8);
    cout << "With gapping (UTF-8):" << endl;
    cout << "H_1 (monograms): " << fixed << setprecision(6) << entropy(mono_sp_utf8, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_sp_utf8, 2) << " bit/symb" << endl;
    vector<string> text_no_sp_utf8 = letter_filter_utf8(raw_content_utf8, false);
    auto mono_no_utf8 = monogram_counter(text_no_sp_utf8);
    auto bi_no_utf8 = bigram_counter(text_no_sp_utf8);
    cout << "Without gapping (UTF-8):" << endl;
    cout << "H_1 (monograms): " << entropy(mono_no_utf8, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_no_utf8, 2) << " bit/symb" << endl;
}