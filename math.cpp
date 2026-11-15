#include "math.hpp"
#include <stdexcept>

int add(int num1, int num2) {
    return num1 + num2;
}

float divider(int num1, int num2, int flag_nodec) {
    if (num2 == 0) {
        if (flag_nodec == 1) {
            throw std::invalid_argument("Modulo by zero");
        }
        throw std::invalid_argument("Division by zero");
    }

    if (flag_nodec == 0) {
        return static_cast<float>(num1) / static_cast<float>(num2);
    } else if (flag_nodec == 1) {
        return static_cast<float>(num1 % num2);
    } else {
        return static_cast<float>(num1) / static_cast<float>(num2);
    }
}

int multiplier(int num1, int num2) {
    return num1 * num2;
}

int subtract(int num1, int num2) {
    return num1 - num2;
}
