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

int main() {
	ifstream input_file("plaintext_test.txt");
	ifstream key_file("key_2_test.txt");
	if (!input_file.is_open() || !key_file.is_open()) {
		cerr << "Error!!! Could not open files for encryption" << endl;
		return 1;
	}
	string plaintext, key;
	getline(input_file, plaintext);
	getline(key_file, key);
	if (plaintext.empty() || key.empty()) {
		cerr << "Error!!! Text or key is empty" << endl;
		return 1;
	}
	string ciphertext = Vigenere_encrypt(plaintext, key);
	ofstream output_file("ciphertext_test.txt");
	output_file << ciphertext;
}