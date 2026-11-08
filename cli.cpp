#include <iostream>
#include <string>
#include <vector>
#include "argparse/argparse.hpp"

void run_calc(int previous_flag);
int apiparse(const std::string& op, const std::string& num1, const std::string& num2) {
    char oper = '\0';
    if (op == "+" || op == "add") {
        oper = 'A';
    } else if (op == "-" || op == "sub") {
        oper = 'S';        
    } else if (op == "×" || op == "x" || op == "*" || op == "mul") {
        oper = 'M';
    } else if (op == "÷" || op == "/" || op == "div") {
        oper = 'D';
    } else if (op == "%" || op == "mod") {
        oper = 'O';
    }

    int n1 = std::stoi(num1);
    int n2 = std::stoi(num2);

    switch(oper) {
        case 'A':
            return n1 + n2;
        case 'S':
            return n1 - n2;
        case 'M':
            return n1 * n2;
        case 'D':
            if (n2 != 0) return n1 / n2;
            std::cerr << "Error 3: division by zero\n";
            return 0;
        case 'O':
            if (n2 != 0) return n1 % n2;
            std::cerr << "Error 3: division by zero\n";
            return 0;
        default:
            std::cerr << "Error 2: Unknown Operator/n";
            return 0;
    }
}
int lineparse(std::string& line) {
    if (line == "exit") {
        std::exit(EXIT_SUCCESS); 
        }
    if (line.rfind("ext", 0) == 0) {
        if (line.rfind("start", 4) == 4) {
            run_calc(1);
        } else if (line.rfind("help", 4) == 4) {
            std::cout << "equations are prefixed with add, sub, div, or mul.\n";
            std::cout << "examples: add 43, 54 or sub 12 from 32 or div 8 by 2\n";
            run_calc(1);
        }
    } else {
        size_t commaposition = line.find(',');
        size_t fromposition = line.find("from");
        size_t byposition = line.find("by");
        size_t plusposition = line.find('+');
        size_t minusposition = line.find('-');
        size_t multiposition = line.find('*');
        size_t divideposition = line.find('/');
        size_t modulusposition = line.find('%');
        
        if (line.find("add") == 0) {
            size_t split_pos = (commaposition != std::string::npos) ? commaposition : plusposition;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                std::cout << (leftnum + rightnum) << "\n";
                run_calc(1);
            }
        } else if (line.find("sub") == 0) {
            if (fromposition != std::string::npos) {
                std::string left_side = line.substr(4, fromposition - 4);
                std::string right_side = line.substr(fromposition + 4);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                std::cout << (rightnum - leftnum) << "\n";
                run_calc(1);
            } else if (minusposition != std::string::npos) {
                std::string left_side = line.substr(4, minusposition - 4);
                std::string right_side = line.substr(minusposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                std::cout << (leftnum - rightnum) << "\n";
                run_calc(1);
            } else if (commaposition != std::string::npos) {
                std::string left_side = line.substr(4, commaposition - 4);
                std::string right_side = line.substr(commaposition + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                std::cout << (leftnum - rightnum) << "\n";
                run_calc(1);
            }
        } else if (line.find("mul") == 0) {
            size_t split_pos = (commaposition != std::string::npos) ? commaposition : multiposition;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + 1);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                std::cout << (leftnum * rightnum) << "\n";
                run_calc(1);
            }
        } else if (line.find("div") == 0) {
            size_t split_pos = (byposition != std::string::npos) ? byposition : 
                               (commaposition != std::string::npos) ? commaposition : divideposition;
            size_t offset = (byposition != std::string::npos) ? 2 : 1;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + offset);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                if (rightnum != 0) {
                    std::cout << (leftnum / rightnum) << "\n";
                } else {
                    std::cout << "Error: division by zero\n";
                }
                run_calc(1);
            }
        } else if (line.find("mod") == 0) {
            size_t split_pos = (byposition != std::string::npos) ? byposition : 
                               (commaposition != std::string::npos) ? commaposition : modulusposition;
            size_t offset = (byposition != std::string::npos) ? 2 : 1;
            if (split_pos != std::string::npos) {
                std::string left_side = line.substr(4, split_pos - 4);
                std::string right_side = line.substr(split_pos + offset);
                int leftnum = std::stoi(left_side);
                int rightnum = std::stoi(right_side);
                if (rightnum != 0) {
                    std::cout << (leftnum % rightnum) << "\n";
                } else {
                    std::cout << "Error: division by zero\n";
                }
                run_calc(1);
            }
        } else if (plusposition != std::string::npos) {
            int leftnum = std::stoi(line.substr(0, plusposition));
            int rightnum = std::stoi(line.substr(plusposition + 1));
            std::cout << (leftnum + rightnum) << "\n";
            run_calc(1);
        } else if (minusposition != std::string::npos) {
            int leftnum = std::stoi(line.substr(0, minusposition));
            int rightnum = std::stoi(line.substr(minusposition + 1));
            std::cout << (leftnum - rightnum) << "\n";
            run_calc(1);
        } else if (multiposition != std::string::npos) {
            int leftnum = std::stoi(line.substr(0, multiposition));
            int rightnum = std::stoi(line.substr(multiposition + 1));
            std::cout << (leftnum * rightnum) << "\n";
            run_calc(1);
        } else if (divideposition != std::string::npos) {
            int leftnum = std::stoi(line.substr(0, divideposition));
            int rightnum = std::stoi(line.substr(divideposition + 1));
            if (rightnum != 0) {
                std::cout << (leftnum / rightnum) << "\n";
            } else {
                std::cout << "Error: division by zero\n";
            }
            run_calc(1);
        } else if (modulusposition != std::string::npos) {
            int leftnum = std::stoi(line.substr(0, modulusposition));
            int rightnum = std::stoi(line.substr(modulusposition + 1));
            if (rightnum != 0) {
                std::cout << (leftnum % rightnum) << "\n";
            } else {
                std::cout << "Error: division by zero\n";
            }
            run_calc(1);
        }
    }
    return 0;
}

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
// this is used to 
int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("ultracalc");

    program.add_argument("--calcapi")
        .help("extensions use this.")
        .nargs(3);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.is_used("--calcapi")) {
        auto calcapi = program.get<std::vector<std::string>>("--calcapi");
        std::string op = calcapi[0];
        std::string num1 = calcapi[1];
        std::string num2 = calcapi[2];
        int result = apiparse(op, num1, num2);
        std::cout << result << std::endl;
        return 0;
    }

    run_calc(0);
    return 0;
}
