#include <iostream>
#include "toml.hpp"
#include <string>

void parse_joml(std::string& manifest) {
    struct UButton {
        int xsize;
        int ysize;
        std::string bglocation;
        int xlocation;
        int ylocation;
        std::string buttonfunction;
    };
    struct CalcDisplay {
        int xsize;
        int ysize;
        std::string bglocation;
        int ylocation;
        int xlocation;
        std::string borderstyle;
        std::string fontlocation;     
    };
    // TODO: Parse JSON/TOML UI layout specifications into UButton and CalcDisplay structures (post-1.0)
}

int main() {
    std::cout << "building fine\n";
    return 0;
}