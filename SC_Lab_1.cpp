#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

string letter_filter(const string& raw, bool keep_spaces) {
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

map<char, long long> monogram_counter(const string& text) {
    map<char, long long> counts;
    for (char c : text) {
        counts[c]++;
    }
    return counts;
}

map<string, long long> bigram_counter(const string& text) {
    map<string, long long> counts;
    if (text.length() < 2) return counts;
    for (size_t i = 0; i < text.length() - 1; ++i) {
        string bi = "";
        bi += text[i];
        bi += text[i + 1];
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
    string filename = "text_for_entropy_Windows.txt"; // Файл > 1Мб
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR!!! File cannot be open" << endl;
        return 1;
    }
    string raw_content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    cout << fixed << setprecision(6);
    cout << "--- Results for Windows-1251 ---" << endl << endl;
    string text_with_sp = letter_filter(raw_content, true);
    auto mono_sp = monogram_counter(text_with_sp);
    auto bi_sp = bigram_counter(text_with_sp);
    cout << "With gapping:" << endl;
    cout << "H_1 (monograms): " << entropy(mono_sp, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_sp, 2) << " bit/symb" << endl;
    string text_no_sp = letter_filter(raw_content, false);
    auto mono_no = monogram_counter(text_no_sp);
    auto bi_no = bigram_counter(text_no_sp);
    cout << "Without gapping:" << endl;
    cout << "H_1 (monograms): " << entropy(mono_no, 1) << " bit/symb" << endl;
    cout << "H_2 (bigrams):   " << entropy(bi_no, 2) << " bit/symb" << endl;
}