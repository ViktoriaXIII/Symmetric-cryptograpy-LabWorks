#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

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



int main()
{
    
}