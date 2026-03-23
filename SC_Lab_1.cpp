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

void save_monograms_win1251(ofstream& out, const map<char, long long>& counts, const string& title) {
    out << "\n=== " << title << " ===\n";
    vector<pair<char, long long>> sorted_v(counts.begin(), counts.end());
    sort(sorted_v.begin(), sorted_v.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    long long total = 0;
    for (auto const& p : sorted_v) total += p.second;

    out << "Char\tCount\tProbability\n";
    for (auto const& p : sorted_v) {
        double prob = (double)p.second / total;
        if (p.first == ' ') out << "[space]";
        else out << p.first;
        out << "\t" << p.second << "\t" << fixed << setprecision(6) << prob << "\n";
    }
}

void save_bigrams_win1251(ofstream& out, const string& text_data, const map<string, long long>& bi_counts, const string& title) {
    out << "\n=== MATRIX: " << title << " ===\n";
    map<char, int> alphabet_map;
    for (char c : text_data) alphabet_map[c] = 1;

    vector<char> alphabet;
    for (auto const& [ch, val] : alphabet_map) alphabet.push_back(ch);

    out << " \t";
    for (char col : alphabet) out << col << "\t";
    out << "\n";

    for (char row : alphabet) {
        out << row << "\t";
        for (char col : alphabet) {
            string key = ""; key += row; key += col;
            out << (bi_counts.count(key) ? bi_counts.at(key) : 0) << "\t";
        }
        out << "\n";
    }
}

void save_monograms_utf8(ofstream& out, const map<string, long long>& counts, const string& title) {
    out << "\n=== " << title << " ===\n";
    vector<pair<string, long long>> sorted_v(counts.begin(), counts.end());
    sort(sorted_v.begin(), sorted_v.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    long long total = 0;
    for (auto const& p : sorted_v) total += p.second;

    out << "Char\tCount\tProbability\n";
    for (auto const& p : sorted_v) {
        double prob = (double)p.second / total;
        if (p.first == " ") out << "[space]";
        else out << p.first;
        out << "\t" << p.second << "\t" << fixed << setprecision(6) << prob << "\n";
    }
}

void save_bigrams_utf8(ofstream& out, const vector<string>& text_data, const map<string, long long>& bi_counts, const string& title) {
    out << "\n=== MATRIX: " << title << " ===\n";
    map<string, int> alphabet_map;
    for (const string& s : text_data) alphabet_map[s] = 1;

    vector<string> alphabet;
    for (auto const& [ch, val] : alphabet_map) alphabet.push_back(ch);

    out << " \t";
    for (const string& col : alphabet) out << col << "\t";
    out << "\n";

    for (const string& row : alphabet) {
        out << row << "\t";
        for (const string& col : alphabet) {
            string key = row + col;
            out << (bi_counts.count(key) ? bi_counts.at(key) : 0) << "\t";
        }
        out << "\n";
    }
}

int main(){
    string filename_wind1251 = "text_for_entropy_Windows.txt"; // Файл > 1Мб
    ifstream file_wind1251(filename_wind1251, ios::binary);
    if (!file_wind1251.is_open()) {
        cerr << "ERROR!!! File cannot be open" << endl;
        return 1;
    }
    ofstream res_win("results_windows1251.txt");
    if (!res_win.is_open()) {
        cerr << "Could not create results file!" << endl;
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
    res_win << "--- WINDOWS-1251 (Mono-)Bigrams ---\n";
    save_monograms_win1251(res_win, mono_sp_wind1251, "Monograms (Win-1251) with spaces");
    save_bigrams_win1251(res_win, text_with_sp_wind1251, bi_sp_wind1251, "Bigrams (Win-1251) with spaces");
    string text_no_sp_wind1251 = letter_filter_wind1251(raw_content_wind1251, false);
    auto mono_no_wind1251 = monogram_counter(text_no_sp_wind1251);
    auto bi_no_wind1251 = bigram_counter(text_no_sp_wind1251);
    cout << "Without gapping:" << endl;
    cout << "H_1 (monograms): " << entropy(mono_no_wind1251, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_no_wind1251, 2) << " bit/symb" << endl;
    save_monograms_win1251(res_win, mono_no_wind1251, "Monograms (Win-1251) without spaces");
    save_bigrams_win1251(res_win, text_no_sp_wind1251, bi_no_wind1251, "Bigrams (Win-1251) without spaces");

    string filename_utf8 = "text_for_entropy_UTF.txt"; // Файл > 1Мб
    ifstream file_utf(filename_utf8, ios::binary);
    if (!file_utf.is_open()) {
        cerr << "ERROR!!! File cannot be opened: " << filename_utf8 << endl;
        return 1;
    }
    ofstream res_utf("results_utf8.txt");
    if (!res_utf.is_open()) {
        cerr << "Could not create results file!" << endl;
        return 1;
    }
    string raw_content_utf8((istreambuf_iterator<char>(file_utf)), istreambuf_iterator<char>());
    file_utf.close();
    // Видаляємо BOM (Byte Order Mark), якщо він присутній (перші 3 байти UTF-8 файлу)
    if (raw_content_utf8.size() >= 3 && (unsigned char)raw_content_utf8[0] == 0xEF && (unsigned char)raw_content_utf8[1] == 0xBB && (unsigned char)raw_content_utf8[2] == 0xBF) {
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
    res_utf << "\n\n--- UTF-8 (Mono-)Bigrams ---\n";
    save_monograms_utf8(res_utf, mono_sp_utf8, "Monograms (UTF-8) with spaces");
    save_bigrams_utf8(res_utf, text_with_sp_utf8, bi_sp_utf8, "Bigrams (UTF-8) with spaces");
    vector<string> text_no_sp_utf8 = letter_filter_utf8(raw_content_utf8, false);
    auto mono_no_utf8 = monogram_counter(text_no_sp_utf8);
    auto bi_no_utf8 = bigram_counter(text_no_sp_utf8);
    cout << "Without gapping (UTF-8):" << endl;
    cout << "H_1 (monograms): " << entropy(mono_no_utf8, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_no_utf8, 2) << " bit/symb" << endl;
    save_monograms_utf8(res_utf, mono_no_utf8, "Monograms (UTF-8) without spaces");
    save_bigrams_utf8(res_utf, text_no_sp_utf8, bi_no_utf8, "Bigrams (UTF-8) without spaces");
}