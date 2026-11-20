#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::string shell_quote(const std::string& arg) {
    std::string out = "'";
    for (char c : arg) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
}

fs::path find_gui_script(const char* argv0) {
    std::error_code ec;
    fs::path exe_path = fs::weakly_canonical(fs::path(argv0), ec);
    fs::path exe_dir = exe_path.has_parent_path() ? exe_path.parent_path() : fs::current_path();

    std::vector<fs::path> candidates = {
        exe_dir / "guiqt.py",
        exe_dir.parent_path() / "guiqt.py",
        fs::current_path() / "guiqt.py"
    };

    for (const auto& p : candidates) {
        if (fs::exists(p, ec) && fs::is_regular_file(p, ec)) {
            return p;
        }
    }
    return {};
}

} // namespace

int main(int argc, char* argv[]) {

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Usage: " << argv[0] << "\n"
                      << "Launches the UltraCalc PyQt6 desktop GUI (guiqt.py).\n";
            return 0;
        }
    }

    fs::path script_path = find_gui_script(argv[0]);
    if (script_path.empty()) {
        std::cerr << "Error: could not locate guiqt.py alongside ultracalc-gui or in the project root.\n";
        return 1;
    }

    // Verify Python 3 and PyQt6 availability before launching
    int py_check = std::system("python3 --version >/dev/null 2>&1");
    if (py_check != 0) {
        std::cerr << "Error: python3 interpreter not found in PATH.\n";
        return 1;
    }

    int qt_check = std::system("python3 -c \"import PyQt6\" >/dev/null 2>&1");
    if (qt_check != 0) {
        std::cerr << "Error: PyQt6 is not installed for python3.\n"
                  << "Install it with: pip install PyQt6\n";
        return 1;
    }

    std::string cmd = "python3 " + shell_quote(script_path.string());
    for (int i = 1; i < argc; ++i) {
        cmd += " " + shell_quote(argv[i]);
    }

    int rc = std::system(cmd.c_str());
    return rc == 0 ? 0 : 1;
}
