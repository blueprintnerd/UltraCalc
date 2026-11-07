#include <iostream>
#include <string>

void run_calc(int previous_flag);

int lineparse(std::string& line) {
    if (line.rfind("ext", 0) == 0) {
        if (line.rfind("start", 4) == 4) {
            run_calc(1);
        } else if (line.rfind("help", 4) == 4) {
            std::cout << "equations are prefixed with add, sub, div, or mul.\n";
            std::cout << "examples: add 43, 54 or sub 12 from 32 or div 8 by 2\n";
            run_calc(1);
        }
    } else {
        // orbit_hide:expanded (size_t for operators)
        size_t commaposition = line.find(',');
        size_t fromposition = line.find("from");
        size_t byposition = line.find("by");
        size_t plusposition = line.find("+");
        size_t minusposition = line.find("-");
        size_t multiposition = line.find("*");
        size_t divideposition = line.find("/");
        size_t modulusposition = line.find("%");
        
        // orbit_hide:end
        if (line.find("add") == 0) {
            if (commaposition != std::string::npos) {
                std::string left_side = line.substr(4, commaposition - 4);
                std::string right_side = line.substr(commaposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                int sum = leftnum + rightnum;
                std::cout << sum << "\n";
                run_calc(1);
            } else if (line.find("+") == 1) {
                if (plusposition != std::string::npos) {
                    std::string left_side = line.substr(4, plusposition - 4);
                    std::string right_side = line.substr(plusposition + 1);
                    int leftnum = std::stoi(left_side);
                    int rightnum = std::stoi(right_side);
                    int sum = leftnum + rightnum;
                    std::cout << sum << "\n";
                    run_calc(1);
            }
        } else if (line.find("sub") == 0) {
            if (fromposition != std::string::npos) {
                std::string left_side = line.substr(4, fromposition - 4);
                std::string right_side = line.substr(fromposition + 4);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                int sum = rightnum - leftnum;
                
              std::cout << sum << "\n";
                run_calc(1);
              
            }{;
        } } } else if (line.find('-')){
            std::string left_side = line.substr(4, minusposition -4);
            std::string right_side = line.substr(minusposition +4);
            int leftnum = std::stoi(left_side);
            int rightnum = std::stoi(right_side);
            int sum = leftnum - rightnum;
            std::cout << sum << "\n";
            run_calc(1);
            
        } else if (line.find("mul") == 0) {
            if (commaposition != std::string::npos) {
                std::string left_side = line.substr(4, commaposition - 4);
                std::string right_side = line.substr(commaposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                int sum = leftnum * rightnum;
                std::cout << sum << "\n";
                run_calc(1);
                
                
            } else if (line.find('*')) {
                if (multiposition != std::string::npos) {
                std::string left_side = line.substr(1, multiposition - 1);
                std::string right_side = line.substr(commaposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                int sum = leftnum * rightnum;
                std::cout << sum << "\n";
                run_calc(1);
                }
            }
        } else if (line.find("div") == 0) {
            size_t split_pos = (byposition != std::string::npos) ? byposition : commaposition;
            size_t offset = (byposition != std::string::npos) ? 2 : 1;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + offset);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                if (rightnum != 0) {
                    int divided_num = leftnum / rightnum;
                    std::cout << divided_num << "\n";
                } else {
                    std::cout << "Error: division by zero\n";
                }
                run_calc(1);
            }
        }
        //This was just to fill out my hours to a 6 cleanly. tomorrow add modulus
        else if (line.find("mod") == 0) {
            //modulus uses division code because it's basically division 
            // I'll have to have to make this trigger a flag. anyway onto the net
            size_t split_pos = (byposition != std::string::npos) ? byposition : commaposition;
            size_t offset = (byposition != std::string::npos) ? 2 : 1;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + offset);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                if (rightnum != 0) {
                    int moduluednum = leftnum % rightnum;
                    std::cout << moduluednum << "\n";
                }
            
            
        }
    }
    return 0;
} }

void run_calc(int previous_flag) {
    std::string b1;
    if (previous_flag == 1) {
        std::cout << "uc> ";
        if (std::getline(std::cin, b1)) {
            lineparse(b1);
        }
    } else {
        std::cout << "UltraCalc\n";
        std::cout << "uc> ";
        if (std::getline(std::cin, b1)) {
            lineparse(b1);
        }
    }
}

int main() {
    run_calc(0);
    return 0;
}