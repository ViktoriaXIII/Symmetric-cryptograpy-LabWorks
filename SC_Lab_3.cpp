#include <iostream>
#include <fstream>
#include <windows.h>
#include "mostFreqBigrams.hpp"
using namespace std;

struct KeyCandidate {
    long long a;
    long long b;
};

string utf8_to_cp1251(const string& utf8) {
    string res = "";
    for (size_t i = 0; i < utf8.length(); ) {
        unsigned char b1 = utf8[i];
        if (b1 < 0x80) {
            res += utf8[i];
            i++;
        }
        else if (i + 1 < utf8.length()) {
            unsigned char b2 = utf8[i + 1];
            if (b1 == 0xD0) {
                if (b2 >= 0x90 && b2 <= 0xBF) res += static_cast<char>(b2 + 48);
                else if (b2 == 0x81) res += static_cast<char>(229); // ® -> Â
            }
            else if (b1 == 0xD1) {
                if (b2 >= 0x80 && b2 <= 0x8F) res += static_cast<char>(b2 + 112);
                else if (b2 == 0x91) res += static_cast<char>(229); // ∏ -> Â
            }
            i += 2;
        }
        else i++;
    }
    return res;
}

// –¿≈: x, y : a*x + b*y = gcd(a, b)
long long EEA(long long a, long long b, long long& x, long long& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long x1, y1;
    long long d = EEA(b, a % b, x1, y1);
    x = y1;
    y = x1 - y1 * (a / b);
    return d;
}

// Inverse element
long long inv_el(long long a, long long m) {
    long long x, y;
    long long g = EEA((a % m + m) % m, m, x, y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

// All solutions for the Linear Congruence: A * x = B (mod M)
vector<long long> lin_cong(long long A, long long B, long long M) {
    vector<long long> solutions;
    A = (A % M + M) % M;
    B = (B % M + M) % M;
    long long x0, y0;
    long long g = EEA(A, M, x0, y0);
    // B !|  g = gcd(A, M) => no solutions
    if (B % g != 0) return solutions;
    // A' * x = B' (mod M')
    long long A_prime = A / g;
    long long B_prime = B / g;
    long long M_prime = M / g;
    // Base solution
    long long inv = inv_el(A_prime, M_prime);
    long long base_sol = (B_prime * inv) % M_prime;
    if (base_sol < 0) base_sol += M_prime;
    //All solutions
    for (int k = 0; k < g; ++k) solutions.push_back((base_sol + k * M_prime) % M);
    return solutions;
}

class BigramConverter {
private:
    string alphabet;
    int m;
    int char_size;

public:
    BigramConverter(const string& auth_alphabet) {
        alphabet = auth_alphabet;
        char_size = (alphabet.length() == 62) ? 2 : 1;
        m = (char_size == 2) ? (alphabet.length() / 2) : alphabet.length();
    }
    int get_m2() const { return m * m; }
    long long bigram_to_id(const string& bi) const {
        if (bi.length() < static_cast<size_t>(2 * char_size)) return -1;
        string c1 = bi.substr(0, char_size);
        string c2 = bi.substr(char_size, char_size);
        auto p1 = alphabet.find(c1);
        auto p2 = alphabet.find(c2);
        if (p1 == string::npos || p2 == string::npos) return -1;
        long long id1 = p1 / 2;
        long long id2 = p2 / 2;
        return static_cast<long long>((p1 / char_size) * m + (p2 / char_size));
    }
};

vector<KeyCandidate> find_key_candidates(const vector<string>& cipher_top5, const string& alphabet) {
    bool is_src_utf8 = (alphabet.length() == 62);
    vector<string> localized_cipher = cipher_top5;
    if (!is_src_utf8) {
        for (auto& bi : localized_cipher) {
            bi = utf8_to_cp1251(bi);
        }
    }
    BigramConverter conv(alphabet);
    long long m2 = conv.get_m2();
    vector<KeyCandidate> candidates;
    vector<string> lang_top5 = { "ÒÚ", "ÌÓ", "ÂÌ", "ÚÓ", "Ì‡" };
    // 1. X*
    for (size_t i = 0; i < lang_top5.size(); ++i) {
        // 2. X**
        for (size_t j = 0; j < lang_top5.size(); ++j) {
            if (i == j) continue;
            long long X_star = conv.bigram_to_id(lang_top5[i]);
            long long X_double_star = conv.bigram_to_id(lang_top5[j]);
            // Y*
            for (size_t k = 0; k < cipher_top5.size(); ++k) {
                // 4. Y**
                for (size_t l = 0; l < cipher_top5.size(); ++l) {
                    if (k == l) continue;
                    long long Y_star = conv.bigram_to_id(localized_cipher[k]);
                    long long Y_double_star = conv.bigram_to_id(localized_cipher[l]);
                    if (X_star == -1 || X_double_star == -1 || Y_star == -1 || Y_double_star == -1) {
                        continue; // if symbol doesn't belong to the alphabet
                    }
                    // a * (X* - X**) == Y* - Y** (mod m^2) -> A * a == B (mod m^2)
                    long long A = (X_star - X_double_star + m2) % m2;
                    long long B = (Y_star - Y_double_star + m2) % m2;
                    vector<long long> a_sols = lin_cong(A, B, m2);
                    for (long long a : a_sols) { 
                        // gcd(a,m^2)=1 
                        long long dummy_x, dummy_y;
                        if (EEA(a, m2, dummy_x, dummy_y) != 1) continue;
                        // b = (Y* - a * X*) mod m^2
                        long long b = (Y_star - (a * X_star) % m2 + m2) % m2;
                        candidates.push_back({ a, b });
                    }
                }
            }
        }
    }

    cout << candidates.size() << " potential candidates have been found for the key (a, b)\n";
    return candidates;
}

int main()
{
    SetConsoleCP(65001);       // ƒÎˇ ‚‚Â‰ÂÌÌˇ (Ì‡ ‚ÒˇÍËÈ ‚ËÔ‡‰ÓÍ)
    SetConsoleOutputCP(65001);
    string alphabet1 = "‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˚¸˝˛ˇ";
    string alphabet2 = "‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘¸˚˝˛ˇ";
    string filename = "V13";
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error!!! Cannot open the file " << filename << "\n";
        return 1;
    }
    string ciphertext;
    string line;
    while (getline(file, line)) ciphertext += line + "\n";
    file.close();
    if (ciphertext.empty()) {
        cout << "The file is empty\n";
        return 0;
    }
    vector<string> top_bigrams = BigramAnalyzer::get_top_5_bigrams_utf8(ciphertext);
    cout << "Top-5 bigrams of the file:\n";
    if (top_bigrams.empty()) cout << "Cannot find any\n";
    else {
        for (size_t i = 0; i < top_bigrams.size(); ++i) cout << i + 1 << ": \"" << top_bigrams[i] << "\"\n";
    }
    cout << "--------------------------------------------------\n";
    vector<KeyCandidate> keys = find_key_candidates(top_bigrams, alphabet1);
    cout << "Some examples of the keys:\n";
    for (size_t i = 0; i < keys.size() && i < 10; ++i) {
        cout << i + 1 << ": a = " << keys[i].a << ", b = " << keys[i].b << "\n";
    }
}