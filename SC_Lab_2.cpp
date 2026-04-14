#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

string Vigenere_encrypt(const string& text, const string& key) {
	string alphabet = "абвгдежзийклмнопрстуфхцчшщъыьэюя";
	int char_to_index[256];
	for (int i = 0; i < 256; i++) char_to_index[i] = -1;
	for (int i = 0; i < alphabet.length(); i++) char_to_index[(unsigned char)alphabet[i]] = i;
	string encrypted_text = "";
	int key_len = key.length();
	int key_pos = 0;
	for (unsigned char p_char : text) {
		int p = char_to_index[p_char];
		if (p == -1) continue; //Символ не з алфавіту => Пропуск
		int k = char_to_index[(unsigned char)key[key_pos % key_len]];
		if (k == -1) { // Проігноруємо символ, що не з алфавіту (провсяк випадок)
			key_pos++;
			continue;
		}
		int c = (p + k) % 32;
		encrypted_text += alphabet[c];
		key_pos++;
	}
	return encrypted_text;
}

double calculate_I_r(const string& text) {
	int n = text.length();
	if (n <= 1) return 0.0;
	string alphabet = "абвгдежзийклмнопрстуфхцчшщъыьэюя";
	vector<long long> counts(32, 0);
	for (unsigned char c : text) {
		size_t pos = alphabet.find(c);
		if (pos != string::npos) counts[pos]++;
	}
	long long sum = 0;
	for (int i = 0; i < 32; i++) sum += counts[i] * (counts[i] - 1);
	return (double)sum / (n * (n - 1));
}

int main() {
	ifstream input_file("plaintext_test.txt");
	if (!input_file.is_open()) {
		cerr << "Error!!! Could not open file for encryption" << endl;
		return 1;
	}
	string plaintext;
	getline(input_file, plaintext);
	if (plaintext.empty()) {
		cerr << "Error!!! Text is empty" << endl;
		return 1;
	}
	double I_plain = calculate_I_r(plaintext);
	cout << "r = 0 \t I_r = " << I_plain << endl;
	vector<string> key_files = {"key_2_test.txt", "key_3_test.txt", "key_4_test.txt", "key_5_test.txt", "key_10_test.txt", "key_15_test.txt", "key_20_test.txt"};
	for (const string& file_name : key_files) {
		ifstream key_file(file_name);
		if (!key_file.is_open()) {
			cerr << "Error!!! Could not open file with key for encryption" << endl;
			return 1;
		}
		string current_key;
		getline(key_file, current_key);
		if (current_key.empty()) {
			cerr << "Error!!! Key is empty" << endl;
			return 1;
		}
		string ciphertext = Vigenere_encrypt(plaintext, current_key);
		double I_r = calculate_I_r(ciphertext);
		cout << "r = " << current_key.length() << " \t I_r = " << I_r << endl;
	}
}