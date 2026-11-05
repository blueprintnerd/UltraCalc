#include <iostream>

int add(int num1, int num2) {
    return num1 + num2;
}

float divider(int num1, int num2, int flag_nodec) {
    if (flag_nodec == 0) {
        return static_cast<float>(num1) / num2;
    } else if (flag_nodec == 1) {
        return static_cast<float>(num1 % num2);
    } else {
        return static_cast<float>(num1) / num2;
    }
}

int multiplier(int num1, int num2) {
    return num1 * num2;
}

int subtract(int num1, int num2) {
    return num1 - num2;
}
