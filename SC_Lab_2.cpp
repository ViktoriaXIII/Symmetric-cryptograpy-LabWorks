#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <windows.h>
using namespace std;

string alphabet = "אבגדהוזחטיךכלםמןנסעףפץצקרשתûü‎‏ÿ";

int get_alphabet_index(unsigned char c) {
	for (int i = 0; i < 32; i++) {
		if ((unsigned char)alphabet[i] == c) return i;
	}
	return -1;
}

string cleaner(const string& text) {
	string result = "";
	for (unsigned char c : text) {
		if (get_alphabet_index(c) != -1) result += c;
	}
	return result;
}

string read_file(const string& name) {
	ifstream file(name);
	if (!file.is_open()) {
		cout << "Error!!! Could not open file " << name << endl;
		return "";
	}
	string text = "";
	string line;
	while (getline(file, line)) text += line;
	file.close();
	return text;
}

string Vigenere_encrypt(const string& plaintext, const string& key) {
	string ciphertext = "";
	int key_len = key.length();
	for (int i = 0; i < plaintext.length(); i++) {
		int p = get_alphabet_index(plaintext[i]);
		int k = get_alphabet_index(key[i % key_len]);
		int c = (p + k) % 32;
		ciphertext += alphabet[c];
	}
	return ciphertext;
}

string Vigenere_decrypt(const string& ciphertext, const string& key) {
	string plaintext = "";
	int key_len = key.length();
	for (int i = 0; i < ciphertext.length(); i++) {
		int c = get_alphabet_index(ciphertext[i]);
		int k = get_alphabet_index(key[i % key_len]);
		int p = (c - k + 32) % 32;
		plaintext += alphabet[p];
	}
	return plaintext;
}

double calculate_I_r(const string& text) {
	long long n = text.length();
	if (n < 2) return 0.0;
	vector<int> count(32, 0);
	for (unsigned char c : text) {
		int index = get_alphabet_index(c);
		if (index != -1) count[index]++;
	}
	long long sum = 0;
	for (int i = 0; i < 32; i++) sum += count[i] * (count[i] - 1);
	return (double)sum / (double)(n * (n - 1));
}

double I_for_blocks(const string& ciphertext, int r) {
	double total = 0.0;
	for (int i = 0; i < r; i++) {
		string block = "";
		for (int j = i; j < ciphertext.length(); j += r) block += ciphertext[j];
		total += calculate_I_r(block);
	}
	return total / r;
}

string recover_key_freq(const string& ciphertext, int r) {
	string recovered_key = "";
	int target = get_alphabet_index(alphabet[14]);
	for (int i = 0; i < r; i++) {
		vector<int> count(32, 0);
		for (int j = i; j < ciphertext.length(); j += r) {
			int index = get_alphabet_index(ciphertext[j]);
			if (index != -1) count[index]++;
		}
		int max_index = 0;
		for (int k = 1; k < 32; k++) {
			if (count[k] > count[max_index]) max_index = k;
		}
		int key_index = (max_index - target + 32) % 32;
		recovered_key += alphabet[key_index];
	}
	return recovered_key;
}

string recover_key_M_i(string ciphertext, int r) {
	//vector<double> p = { 0.0801, 0.0159, 0.0454, 0.017, 0.0298, 0.0845, 0.0094, 0.0121, 0.0735, 0.0121, 0.0349, 0.044, 0.0321, 0.067, 0.1097, 0.0281, 0.0473, 0.0547, 0.0626, 0.0262, 0.0026, 0.0097, 0.0048, 0.0144, 0.0073, 0.0036, 0.0004, 0.019, 0.0174, 0.0032, 0.0064, 0.0201 };
	vector<double> p = { 0.0796, 0.0159, 0.0454, 0.0170, 0.0298, 0.0845, 0.0094, 0.0165, 0.0735, 0.0121, 0.0349, 0.0440, 0.0321, 0.0670, 0.01097, 0.0281, 0.0473, 0.0547, 0.0626, 0.0262, 0.0026, .0097, 0.0048, 0.0144, 0.0073, 0.0036, 0.0004, 0.0190, 0.0174, 0.0032, 0.0064, 0.0201 };
	string recovered_key = "";
	for (int i = 0; i < r; i++) {
		vector<int> count(32, 0); // N_x
		int total_in_block = 0;
		for (int j = i; j < ciphertext.length(); j += r) {
			int idx = get_alphabet_index(ciphertext[j]);
			if (idx != -1) {
				count[idx]++;
				total_in_block++;
			}
		}
		double max_M = -1.0; // g
		int best_g = 0;
		for (int g = 0; g < 32; g++) {
			double M = 0.0;
			for (int t = 0; t < 32; t++) {
				double freq = (double)count[(t + g) % 32] / (double)total_in_block;
				M += p[t] * freq;
			}
			if (M > max_M) {
				max_M = M;
				best_g = g;
			}
		}
		recovered_key += alphabet[best_g];
	}
	return recovered_key;
}

int main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "Russian");
	string plaintext = read_file("plaintext_test.txt");
	plaintext = cleaner(plaintext);
	if (plaintext.empty()) {
		cerr << "Error!!! Plaintext is empty";
		return 1;
	}
	cout << "I (for plaintext) = " << fixed << setprecision(5) << calculate_I_r(plaintext) << endl;
	vector<string> key_files = {"key_2_test.txt", "key_3_test.txt", "key_4_test.txt", "key_5_test.txt", "key_10_test.txt", "key_11_test.txt", "key_12_test.txt", "key_13_test.txt", "key_14_test.txt", "key_15_test.txt", "key_16_test.txt", "key_17_test.txt", "key_18_test.txt", "key_19_test.txt", "key_20_test.txt"};
	for (string file : key_files) {
		string current_key = read_file(file);
		current_key = cleaner(current_key);
		if (current_key.empty()) {
			cerr << "Error!!! Key is empty" << endl;
			return 1;
		}
		string ciphertext = Vigenere_encrypt(plaintext, current_key);
		double I_r = calculate_I_r(ciphertext);
		cout << "r = " << setw(2) << current_key.length() << " \t I_r = " << fixed << setprecision(5) << I_r << endl;
	}
	string ciphertext = read_file("Variant_13_ciphertext.txt");
	ciphertext = cleaner(ciphertext);
	if (ciphertext.empty()) {
		cerr << "Error!!! Ciphertext is empty" << endl;
		return 1;
	}
	cout << "~~~Find best keylength (r) by using I_r" << endl;
	double best_I = -1.0;
	int best_r = 2;
	cout << "Average I_r: " << endl;
	for (int r = 2; r <= 30; r++) {
		double current = I_for_blocks(ciphertext, r);
		cout << "r = " << setw(2) << r << " I_r = " << fixed << setprecision(5) << current << endl;
		if (current > best_I) {
			best_I = current;
			best_r = r;
		}
	}
	cout << "Predicted r = " << best_r << endl;
	string recovered_key_freq = recover_key_freq(ciphertext, best_r);
	string recovered_key_M_i = recover_key_M_i(ciphertext, best_r);
	cout << "Recovered key by frequency: " << recovered_key_freq << endl;
	cout << "Recovered key by M_i: " << recovered_key_M_i << endl;
	string decrypted_text_freq = Vigenere_decrypt(ciphertext, recovered_key_freq);
	string decrypted_text_M_i = Vigenere_decrypt(ciphertext, recovered_key_M_i);
	ofstream result_freq("Variant_13_decrypted_freq.txt");
	if (!result_freq.is_open()) {
		cerr << "Error!!! Cannot open result file" << endl;
		return 1;
	}
	result_freq << decrypted_text_freq;
	result_freq.close();
	ofstream result_M_i("Variant_13_decrypted_M_i.txt");
	if (!result_M_i.is_open()) {
		cerr << "Error!!! Cannot open result file" << endl;
		return 1;
	}
	result_M_i << decrypted_text_M_i;
	result_M_i.close();
	cout << endl;
	/*cout << "~~~Encryption / decryption test" << endl;
	string plaintext_test = read_file("plaintext_test.txt");
	plaintext_test = cleaner(plaintext_test);
	cout << "Plain text: " << plaintext_test << endl;
	string key_test = read_file("key_10_test.txt");
	key_test = cleaner(key_test);
	cout << "Key: " << key_test << endl;
	string ciphertext_test = Vigenere_encrypt(plaintext_test, key_test);
	cout << "Ciphertext : " << ciphertext_test << endl;
	string decrypted_text = Vigenere_decrypt(ciphertext_test, key_test);
	cout << "Decrypted text: " << decrypted_text << endl;
	string recovered_key_freq_test = recover_key_freq(ciphertext_test, 10);
	string recovered_key_M_i_test = recover_key_M_i(ciphertext_test, 10);
	cout << "Recovered key by frequency: " << recovered_key_freq_test << endl;
	cout << "Recovered key by M_i: " << recovered_key_M_i_test << endl;*/
}