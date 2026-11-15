#include "httplib.h"
#include "json.hpp"
#include "math.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

int parse_strict_int(const std::string& s) {
    if (s.empty()) {
        throw std::invalid_argument("Empty numeric operand");
    }
    size_t pos = 0;
    int val = std::stoi(s, &pos);
    if (pos != s.size()) {
        throw std::invalid_argument("Invalid numeric operand: " + s);
    }
    return val;
}

int parse_json_int(const json& val, const char* name) {
    if (val.is_number_integer()) {
        return val.get<int>();
    }
    if (val.is_number_float()) {
        double d = val.get<double>();
        if (static_cast<double>(static_cast<int>(d)) == d) {
            return static_cast<int>(d);
        }
        throw std::invalid_argument(std::string("Operand '") + name + "' must be an integer");
    }
    if (val.is_string()) {
        return parse_strict_int(val.get<std::string>());
    }
    throw std::invalid_argument(std::string("Operand '") + name + "' must be a number");
}

json compute_result_json(const std::string& op, int a, int b) {
    std::string lower_op = to_lower(op);
    if (lower_op == "add" || lower_op == "+") {
        return json{{"result", add(a, b)}};
    }
    if (lower_op == "sub" || lower_op == "-") {
        return json{{"result", subtract(a, b)}};
    }
    if (lower_op == "mul" || lower_op == "*") {
        return json{{"result", multiplier(a, b)}};
    }
    if (lower_op == "div" || lower_op == "/") {
        float res = divider(a, b, 0);
        if (static_cast<float>(static_cast<int>(res)) == res) {
            return json{{"result", static_cast<int>(res)}};
        }
        return json{{"result", res}};
    }
    if (lower_op == "mod" || lower_op == "%") {
        return json{{"result", static_cast<int>(divider(a, b, 1))}};
    }
    throw std::invalid_argument("Unknown operator: " + op);
}

fs::path find_asset_html(const char* argv0, const std::string& filename) {
    std::error_code ec;
    fs::path exe_path = fs::weakly_canonical(fs::path(argv0), ec);
    fs::path exe_dir = exe_path.has_parent_path() ? exe_path.parent_path() : fs::current_path();

    std::vector<fs::path> candidates = {
        exe_dir / filename,
        exe_dir.parent_path() / filename,
        fs::current_path() / filename
    };

    for (const auto& p : candidates) {
        if (fs::exists(p, ec) && fs::is_regular_file(p, ec)) {
            return p;
        }
    }
    return {};
}

void serve_html_file(const fs::path& html_path, const char* missing_msg, httplib::Response& res) {
    if (html_path.empty()) {
        res.status = 404;
        res.set_content(json{{"error", missing_msg}}.dump(), "application/json");
        return;
    }
    std::ifstream ifs(html_path, std::ios::binary);
    if (!ifs.is_open()) {
        res.status = 500;
        res.set_content(json{{"error", "Failed to read HTML file"}}.dump(), "application/json");
        return;
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    res.set_content(content, "text/html; charset=utf-8");
}

} // namespace

int main(int argc, char* argv[]) {
    std::string host = "0.0.0.0";
    int port = 8080;

    if (const char* env_host = std::getenv("ULTRACALC_HOST")) {
        if (*env_host) host = env_host;
    }
    if (const char* env_port = std::getenv("ULTRACALC_PORT")) {
        try {
            port = std::stoi(env_port);
        } catch (...) {
        }
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--host <host>] [--port <port>]\n"
                      << "Default: 0.0.0.0:8080\n";
            return 0;
        } else if ((arg == "--host" || arg == "-H") && i + 1 < argc) {
            host = argv[++i];
        } else if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Error: invalid port number\n";
                return 1;
            }
        }
    }

    fs::path html_path = find_asset_html(argv[0], "uigen.html");
    httplib::Server svr;

    svr.Get("/", [html_path](const httplib::Request&, httplib::Response& res) {
        serve_html_file(html_path, "uigen.html not found", res);
    });

    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(json{{"status", "ok"}}.dump(), "application/json");
    });

    svr.Get("/api/calculate", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("op") || !req.has_param("a") || !req.has_param("b")) {
            res.status = 400;
            res.set_content(json{{"error", "Missing required query parameters: op, a, b"}}.dump(), "application/json");
            return;
        }
        try {
            std::string op = req.get_param_value("op");
            int a = parse_strict_int(req.get_param_value("a"));
            int b = parse_strict_int(req.get_param_value("b"));
            json out = compute_result_json(op, a, b);
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::invalid_argument& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 400;
            res.set_content(json{{"error", "Numeric value out of range"}}.dump(), "application/json");
        }
    });

    svr.Post("/api/calculate", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            if (!body.is_object() || !body.contains("op") || !body.contains("a") || !body.contains("b")) {
                res.status = 400;
                res.set_content(json{{"error", "JSON payload must contain 'op', 'a', and 'b'"}}.dump(), "application/json");
                return;
            }
            if (!body["op"].is_string()) {
                res.status = 400;
                res.set_content(json{{"error", "'op' must be a string"}}.dump(), "application/json");
                return;
            }
            std::string op = body["op"].get<std::string>();
            int a = parse_json_int(body["a"], "a");
            int b = parse_json_int(body["b"], "b");
            json out = compute_result_json(op, a, b);
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const json::parse_error& e) {
            res.status = 400;
            res.set_content(json{{"error", std::string("Invalid JSON: ") + e.what()}}.dump(), "application/json");
        } catch (const std::invalid_argument& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 400;
            res.set_content(json{{"error", "Numeric value out of range"}}.dump(), "application/json");
        }
    });

    std::cout << "UltraCalc server listening on http://" << host << ":" << port << std::endl;
    if (!svr.listen(host, port)) {
        std::cerr << "Error: failed to bind to " << host << ":" << port << std::endl;
        return 1;
    }

    return 0;
}
