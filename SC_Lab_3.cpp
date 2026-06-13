#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
using namespace std;

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

int main()
{
    
}