#include "math.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    std::cout << "Running test_math...\n";

    // Addition
    assert(add(2, 3) == 5);
    assert(add(-4, 10) == 6);
    assert(add(-5, -7) == -12);
    assert(add(0, 0) == 0);

    // Subtraction
    assert(subtract(10, 3) == 7);
    assert(subtract(3, 10) == -7);
    assert(subtract(-5, -2) == -3);

    // Multiplication
    assert(multiplier(6, 7) == 42);
    assert(multiplier(-3, 5) == -15);
    assert(multiplier(0, 999) == 0);

    // Division (flag_nodec = 0)
    assert(std::fabs(divider(8, 2, 0) - 4.0f) < 1e-6f);
    assert(std::fabs(divider(7, 2, 0) - 3.5f) < 1e-6f);
    assert(std::fabs(divider(-9, 3, 0) - (-3.0f)) < 1e-6f);
    assert(std::fabs(divider(0, 5, 0) - 0.0f) < 1e-6f);

    // Modulo (flag_nodec = 1)
    assert(std::fabs(divider(17, 5, 1) - 2.0f) < 1e-6f);
    assert(std::fabs(divider(10, 5, 1) - 0.0f) < 1e-6f);

    // Division by zero
    bool threw_div_zero = false;
    try {
        divider(10, 0, 0);
    } catch (const std::invalid_argument&) {
        threw_div_zero = true;
    }
    assert(threw_div_zero);

    // Modulo by zero
    bool threw_mod_zero = false;
    try {
        divider(10, 0, 1);
    } catch (const std::invalid_argument&) {
        threw_mod_zero = true;
    }
    assert(threw_mod_zero);

    std::cout << "All math tests passed!\n";
    return 0;
}
