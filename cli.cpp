#include "math.hpp"
#include "extensionparser.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

int parse_strict_int(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) {
        throw std::invalid_argument("Empty numeric operand");
    }
    size_t pos = 0;
    int val = std::stoi(t, &pos);
    if (pos != t.size()) {
        throw std::invalid_argument("Invalid trailing characters in number: " + t);
    }
    return val;
}

bool split_on_keyword(const std::string& text, const std::string& keyword, std::string& left, std::string& right) {
    std::string lower = to_lower(text);
    size_t pos = lower.find(keyword);
    if (pos == std::string::npos) {
        return false;
    }
    left = trim(text.substr(0, pos));
    right = trim(text.substr(pos + keyword.size()));
    return true;
}

bool split_infix_operator(const std::string& text, char& op_out, std::string& left_out, std::string& right_out) {
    // Scan from index 1 so a leading unary minus on the left operand is not treated as binary '-'
    for (size_t i = 1; i < text.size(); ++i) {
        char c = text[i];
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
            op_out = c;
            left_out = trim(text.substr(0, i));
            right_out = trim(text.substr(i + 1));
            return !left_out.empty() && !right_out.empty();
        }
    }
    return false;
}

int execute_operation(const std::string& op, int a, int b) {
    std::string lower_op = to_lower(op);
    if (lower_op == "add" || lower_op == "+") {
        std::cout << add(a, b) << "\n";
        return 0;
    }
    if (lower_op == "sub" || lower_op == "-") {
        std::cout << subtract(a, b) << "\n";
        return 0;
    }
    if (lower_op == "mul" || lower_op == "*") {
        std::cout << multiplier(a, b) << "\n";
        return 0;
    }
    if (lower_op == "div" || lower_op == "/") {
        std::cout << divider(a, b, 0) << "\n";
        return 0;
    }
    if (lower_op == "mod" || lower_op == "%") {
        std::cout << static_cast<int>(divider(a, b, 1)) << "\n";
        return 0;
    }
    std::cerr << "Error: unknown operator '" << op << "'\n";
    return 1;
}

void print_ext_help() {
    std::cout << "Extension commands:\n"
              << "  ext install <file.uce> [-o <dir>]   Install a .uce extension\n"
              << "  ext validate <file.uce>             Validate a .uce extension\n"
              << "  ext list [-d <dir>]                 List installed extensions\n"
              << "  ext help                            Show this help\n";
}

int handle_ext_args(const std::vector<std::string>& args) {
    if (args.empty() || to_lower(args[0]) == "help") {
        print_ext_help();
        return 0;
    }

    std::string sub = to_lower(args[0]);
    if (sub == "start") {
        return 0;
    }

    if (sub == "list") {
        std::string target_dir;
        for (size_t i = 1; i < args.size(); ++i) {
            if ((args[i] == "-d" || args[i] == "--dir" || args[i] == "-o" || args[i] == "--output") && i + 1 < args.size()) {
                target_dir = args[++i];
            }
        }
        auto exts = list_installed_extensions(target_dir);
        if (exts.empty()) {
            std::cout << "No valid extensions installed.\n";
            return 0;
        }
        for (const auto& ext : exts) {
            std::cout << ext.name << " v" << ext.version
                      << " (main: " << ext.entry_point << ") - " << ext.description << "\n";
        }
        return 0;
    }

    if (sub == "validate") {
        if (args.size() < 2) {
            std::cerr << "Error: ext validate requires <file.uce>\n";
            return 1;
        }
        ExtensionMetadata meta;
        std::string err;
        if (!validate_extension(args[1], &meta, &err)) {
            std::cerr << "Validation failed: " << err << "\n";
            return 1;
        }
        std::cout << "Valid extension: " << meta.name << " v" << meta.version
                  << " (main: " << meta.entry_point << ")\n";
        return 0;
    }

    if (sub == "install" || sub == "load") {
        if (args.size() < 2) {
            std::cerr << "Error: ext " << sub << " requires <file.uce>\n";
            return 1;
        }
        std::string archive_file = args[1];
        std::string dest_dir;
        for (size_t i = 2; i < args.size(); ++i) {
            if ((args[i] == "-o" || args[i] == "--output") && i + 1 < args.size()) {
                dest_dir = args[++i];
            } else if (dest_dir.empty() && args[i][0] != '-') {
                dest_dir = args[i];
            }
        }
        return install_extension(archive_file, dest_dir);
    }

    std::cerr << "Error: unknown ext subcommand '" << args[0] << "'. Type 'ext help' for usage.\n";
    return 1;
}

void print_help() {
    std::cout << "UltraCalc CLI\n"
              << "Natural language commands:\n"
              << "  add <num1>, <num2>\n"
              << "  sub <num1> from <num2>   (computes num2 - num1)\n"
              << "  sub <num1>, <num2>       (computes num1 - num2)\n"
              << "  mul <num1>, <num2>\n"
              << "  div <num1> by <num2>\n"
              << "  mod <num1> by <num2>\n"
              << "Operator syntax:\n"
              << "  <num1> + <num2> | <num1> - <num2> | <num1> * <num2> | <num1> / <num2> | <num1> % <num2>\n"
              << "Extension commands:\n"
              << "  ext install <file.uce> [-o <dir>] | ext validate <file.uce> | ext list\n"
              << "Other commands:\n"
              << "  help | exit | quit\n";
}

} // namespace

int lineparse(const std::string& raw_line) {
    std::string line = trim(raw_line);
    if (line.empty()) {
        return 0;
    }

    std::string lower = to_lower(line);
    if (lower == "exit" || lower == "quit") {
        return -1;
    }
    if (lower == "help") {
        print_help();
        return 0;
    }

    {
        std::istringstream ext_iss(line);
        std::string first;
        ext_iss >> first;
        if (to_lower(first) == "ext") {
            std::vector<std::string> ext_args;
            std::string tok;
            while (ext_iss >> tok) {
                ext_args.push_back(tok);
            }
            return handle_ext_args(ext_args);
        }
    }

    try {
        // Tokenize first word to check for natural language prefix
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        std::string cmd_lower = to_lower(cmd);

        if (cmd_lower == "add" || cmd_lower == "sub" || cmd_lower == "mul" ||
            cmd_lower == "div" || cmd_lower == "mod") {
            std::string rest = trim(line.substr(cmd.size()));
            std::string left_str;
            std::string right_str;
            bool parsed = false;
            bool swap_operands = false;

            if (cmd_lower == "sub" && split_on_keyword(rest, " from ", left_str, right_str)) {
                parsed = true;
                swap_operands = true;
            } else if ((cmd_lower == "div" || cmd_lower == "mod") &&
                       split_on_keyword(rest, " by ", left_str, right_str)) {
                parsed = true;
            } else if (split_on_keyword(rest, ",", left_str, right_str)) {
                parsed = true;
            } else {
                std::istringstream arg_stream(rest);
                if (arg_stream >> left_str >> right_str) {
                    std::string extra;
                    if (!(arg_stream >> extra)) {
                        parsed = true;
                    }
                }
            }

            if (!parsed || left_str.empty() || right_str.empty()) {
                std::cerr << "Error: invalid syntax for '" << cmd_lower << "'\n";
                return 1;
            }

            int a = parse_strict_int(left_str);
            int b = parse_strict_int(right_str);
            if (swap_operands) {
                return execute_operation(cmd_lower, b, a);
            }
            return execute_operation(cmd_lower, a, b);
        }

        char op = 0;
        std::string left_str;
        std::string right_str;
        if (split_infix_operator(line, op, left_str, right_str)) {
            int a = parse_strict_int(left_str);
            int b = parse_strict_int(right_str);
            return execute_operation(std::string(1, op), a, b);
        }

        std::cerr << "Error: unrecognized expression '" << line << "'. Type 'help' for usage.\n";
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: numeric value out of range\n";
        return 1;
    }
}

void run_calc(int previous_flag) {
    if (previous_flag == 0) {
        std::cout << "UltraCalc\n";
    }

    std::string line;
    while (true) {
        std::cout << "uc> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }
        int status = lineparse(line);
        if (status == -1) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Usage:\n"
                      << "  " << argv[0] << "                         Start interactive REPL\n"
                      << "  " << argv[0] << " --calcapi <op> <a> <b>  Evaluate single operation\n"
                      << "  " << argv[0] << " ext <subcommand> [...]  Manage .uce extensions (install, validate, list)\n";
            return 0;
        }
        if (arg1 == "ext") {
            std::vector<std::string> ext_args;
            for (int i = 2; i < argc; ++i) {
                ext_args.emplace_back(argv[i]);
            }
            return handle_ext_args(ext_args);
        }
        if (arg1 == "--calcapi") {
            if (argc != 5) {
                std::cerr << "Error: --calcapi requires 3 arguments: <operator> <num1> <num2>\n";
                return 1;
            }
            try {
                std::string op = argv[2];
                int a = parse_strict_int(argv[3]);
                int b = parse_strict_int(argv[4]);
                return execute_operation(op, a, b);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: numeric value out of range\n";
                return 1;
            }
        }
        std::cerr << "Error: unknown option '" << arg1 << "'. Use --help for usage.\n";
        return 1;
    }

    run_calc(0);
    return 0;
}
