#include <iostream>
#include <string>

int lineparse(std::string& line) {
    if (line.rfind("ext", 0) == 0) {
        if (line.rfind("start", 4) == 4) {
            
        } else if (line.rfind("help", 4) == 4) {
            std::cout << "equations are prefixed with add, sub, div, or mul.\n";
            std::cout << "example: add 43, 54\n";
            std::cout << "run 'ext help' to learn more about extensions\n";
        }
    } else {
        size_t commaposition = line.find(',');
        if (line.find("add") == 0) {
            if (commaposition != std::string::npos) {
                std::string left_side = line.substr(4, commaposition - 4);
                std::string right_side = line.substr(commaposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                int sum = leftnum + rightnum;
                std::cout << sum << "\n";
            }
        } else if (line.find("sub") == 0) {

        } else if (line.find("mul") == 0) {

        } else if (line.find("div") == 0) {

        }
    }
    return 0;
}

int main() {
    std::string b1;
    std::cout << "Ultracalc" << std::endl;
    std::cout << "Run 'ext start tui' to load the TUI extension\n\n";
    std::cout << "uc> ";
    std::getline(std::cin, b1);  
    lineparse(b1);
    return 0;
}