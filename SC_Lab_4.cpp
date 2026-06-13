#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct RegisterConfig {
    int size; // deg
    uint32_t poly; // mask
};

struct AttackConfig {
    int n_star;
    int c_l1;
    int c_l2;
};

class LFSR {
public:
    uint32_t state;
    uint32_t polynomial_mask;
    int size;
    LFSR(RegisterConfig cfg, uint32_t init_state) {
        size = cfg.size;
        polynomial_mask = cfg.poly;
        state = init_state & ((1U << size) - 1);
    }
    uint8_t step() {
        uint8_t output = state & 1;
        uint32_t pop = state & polynomial_mask;
        pop ^= pop >> 16;
        pop ^= pop >> 8;
        pop ^= pop >> 4;
        pop ^= pop >> 2;
        pop ^= pop >> 1;
        uint8_t feedback = pop & 1;
        state = (state >> 1) | (static_cast<uint32_t>(feedback) << (size - 1));
        return output;
    }
};

double get_normal_quantile(double p) {
    if (p <= 0.5) return 0.0;
    double alpha_tail = 1.0 - p;
    double t = sqrt(-2.0 * log(alpha_tail));
    const double c0 = 2.515517;
    const double c1 = 0.802853;
    const double c2 = 0.010328;
    const double d1 = 1.432788;
    const double d2 = 0.189269;
    const double d3 = 0.001308;
    return t - ((c0 + c1 * t + c2 * t * t) / (1.0 + d1 * t + d2 * t * t + d3 * t * t * t));
}

AttackConfig calculate_attack_parameters(RegisterConfig l1, RegisterConfig l2, double alpha) {
    int l1_size = l1.size;
    int l2_size = l2.size;
    const double p1 = 0.25; // H_0
    const double p2 = 0.50; // H_1
    double t_alpha = get_normal_quantile(1.0 - alpha);
    double m1 = (1ULL << l1_size) - 1;
    double beta1 = 0.1 / m1; // beta * M < 1
    double t_beta1 = get_normal_quantile(1.0 - beta1);
    // 1.73205 * t_alpha + 2.0 * t_beta
    double sqrt_n1 = 1.7320508 * t_alpha + 2.0 * t_beta1;
    double n1_raw = sqrt_n1 * sqrt_n1;
    double m2 = (1ULL << l2_size) - 1;
    double beta2 = 0.1 / m2; // beta * M < 1
    double t_beta2 = get_normal_quantile(1.0 - beta2);
    double sqrt_n2 = 1.7320508 * t_alpha + 2.0 * t_beta2;
    double n2_raw = sqrt_n2 * sqrt_n2;
    // max N*
    int final_n_star = static_cast<int>(ceil(max(n1_raw, n2_raw)));
    // C
    // R > C => floor
    double std_dev = sqrt(final_n_star * p1 * (1.0 - p1));
    int final_c_l1 = static_cast<int>(floor(final_n_star * p1 + t_alpha * std_dev));
    int final_c_l2 = static_cast<int>(floor(final_n_star * p1 + t_alpha * std_dev));
    return { final_n_star, final_c_l1, final_c_l2 };
}

void correlation_attack(RegisterConfig l1_cfg, RegisterConfig l2_cfg, RegisterConfig l3_cfg,
    const vector<uint8_t>& z, const AttackConfig& params) {
    int N_star = params.n_star;
    // Sorting and filtering L1
    vector<uint32_t> candidates_L1;
    /*uint32_t*/ int max_state_L1 = (1U << l1_cfg.size) - 1;
#pragma omp parallel for schedule(dynamic)
    for (/*uint32_t*/ int init = 1; init <= max_state_L1; ++init) {
        LFSR l1(l1_cfg, init);
        int R = 0;
        for (int i = 0; i < N_star; ++i) {
            if (l1.step() != z[i]) R++;
        }
        if (R <= params.c_l1) { // R <= C
#pragma omp critical
            {
                candidates_L1.push_back(init);
            }
        }
    }
    cout << "Candidates for L1: " << candidates_L1.size() << "\n";
    // Sorting and filtering L2 
    vector<uint32_t> candidates_L2;
    /*uint32_t*/ int max_state_L2 = (1U << l2_cfg.size) - 1;
#pragma omp parallel for schedule(dynamic)
    for (/*uint32_t*/ int init = 1; init <= max_state_L2; ++init) {
        LFSR l2(l2_cfg, init);
        int R = 0;
        for (int i = 0; i < N_star; ++i) {
            if (l2.step() != z[i]) R++;
        }
        if (R <= params.c_l2) {
#pragma omp critical
            {
                candidates_L2.push_back(init);
            }
        }
    }
    cout << "Candidates for L2: " << candidates_L2.size() << "\n";
    // L3
    uint32_t max_state_L3 = (1U << l3_cfg.size) - 1;
    bool success = false;
    for (uint32_t cand1 : candidates_L1) {
        for (uint32_t cand2 : candidates_L2) {
            // x_i and y_i
            vector<uint8_t> x_seq(N_star), y_seq(N_star);
            LFSR l1_t(l1_cfg, cand1);
            LFSR l2_t(l2_cfg, cand2);
            for (int i = 0; i < N_star; ++i) {
                x_seq[i] = l1_t.step();
                y_seq[i] = l2_t.step();
            }
            // Sort L3
            for (uint32_t init3 = 1; init3 <= max_state_L3; ++init3) {
                LFSR l3(l3_cfg, init3);
                bool is_valid = true;
                // x_i != y_i
                for (int i = 0; i < N_star; ++i) {
                    uint8_t s_i = l3.step();
                    if (x_seq[i] != y_seq[i]) {
                        uint8_t expected_s = (z[i] == x_seq[i]) ? 1 : 0;
                        if (s_i != expected_s) {
                            is_valid = false;
                            break;
                        }
                    }
                }
                if (is_valid) { // Match
                    cout << "L1: 0x" << hex << cand1 << "\n";
                    cout << "L2: 0x" << hex << cand2 << "\n";
                    cout << "L3: 0x" << hex << init3 << "\n";
                    success = true;
                    break;
                }
            }
            if (success) break;
        }
        if (success) break;
    }
    if (!success) cout << "Cannot find the solution\n";
}

int main()
{
    /* //Easy test
    RegisterConfig L1 = {25, 0x09};
    RegisterConfig L2 = { 26, 0x47 };
    RegisterConfig L3 = { 27, 0x27 };
    double alpha = 0.01;
    AttackConfig params = calculate_attack_parameters(L1, L2, alpha);
    cout << "Parameters: N^* = " << params.n_star << ", C_L1 = " << params.c_l1 << ", C_L2 = " << params.c_l2 << "\n";
    cout << "Enter z_i:\n";
    string input_bits;
    cin >> input_bits;
    if (input_bits.length() < static_cast<size_t>(params.n_star)) {
        cout << "ERROR!!! z_i is too short\n";
        cout << "Current length: " << input_bits.length() << " á³ò³â.\n";
        cout << "Enough length: " << params.n_star << " á³ò³â.\n";
        return 1;
    }
    vector<uint8_t> z(params.n_star, 0);
    for (int i = 0; i < params.n_star; ++i) {
        if (input_bits[i] == '1') z[i] = 1;
        else z[i] = 0;
    }
    correlation_attack(L1, L2, L3, z, params);*/

    // Test
    RegisterConfig L1 = { 30,  0x53 };
    RegisterConfig L2 = { 31, 0x09 };
    RegisterConfig L3 = { 32, 0xAF };
    double alpha = 0.01;
    AttackConfig params = calculate_attack_parameters(L1, L2, alpha);
    cout << "Parameters: N^* = " << params.n_star << ", C_L1 = " << params.c_l1 << ", C_L2 = " << params.c_l2 << "\n";
    cout << "Enter z_i:\n";
    string input_bits;
    cin >> input_bits;
    if (input_bits.length() < static_cast<size_t>(params.n_star)) {
        cout << "ERROR!!! z_i is too short\n";
        cout << "Current length: " << input_bits.length() << " á³ò³â.\n";
        cout << "Enough length: " << params.n_star << " á³ò³â.\n";
        return 1;
    }
    vector<uint8_t> z(params.n_star, 0);
    for (int i = 0; i < params.n_star; ++i) {
        if (input_bits[i] == '1') z[i] = 1;
        else z[i] = 0;
    }
    correlation_attack(L1, L2, L3, z, params);
}