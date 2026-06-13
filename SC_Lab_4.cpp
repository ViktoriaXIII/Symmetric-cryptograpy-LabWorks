#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

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
    LFSR(int sz, uint32_t poly, uint32_t init_state) {
        size = sz;
        polynomial_mask = poly;
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

AttackConfig calculate_attack_parameters(int l1_size, int l2_size, double alpha) {
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

int main()
{
    
}