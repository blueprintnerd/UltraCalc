#include <iostream>
#include <string>
void main() {
    std::string b1;
    cout << "Ultracalc" << endl;
    cout << "Run 'ext start tui' to load the TUI extension\n\n"
    cout << "uc>"
    cin >> b1;
    lineparse(b1);
}

int lineparse (line) {
    if (line.rfind("ext", 0) == 0) {
            if (line.rfind("start", 4) == 4) {
                // later: add a file with all the installed extensions
            elseif (line.rfind("add")) {

            } elseif (line.rfind("sub")) {
                
            } elseif (line.rfind("mul")) {

            } elseif (line.rfind("div")) {

            } elseif (line.rfind("help")) {
                std::cout << "equations are prefixed with add, sub, div, or mul.\n";
                std::cout << "example: add 43, 54 (comma not required)\n"
                std::cout << "run 'ext help' to learn more about extensions"
            }



    }
    
}