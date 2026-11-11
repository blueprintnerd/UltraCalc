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
    //TODO: Parse JSON to this spec
}

int main() {
    std::cout << "building fine\n";
    return 0;
}