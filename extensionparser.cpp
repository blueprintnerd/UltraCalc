#include "extensionparser.hpp"
#include "miniz.c"
#include "json.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

std::string get_username() {
    const char* user = std::getenv("USER");
    if (!user) user = std::getenv("USERNAME");
    return user ? std::string(user) : "default_user";
}

fs::path default_extensions_dir() {
    const char* home = std::getenv("HOME");
    if (home && *home) {
        return fs::path(home) / "UltraCalc" / "extension";
    }
    return fs::path("/home") / get_username() / "UltraCalc" / "extension";
}

std::string to_lower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lower;
}

bool is_safe_relative_path(const fs::path& rel_path) {
    if (rel_path.empty()) {
        return false;
    }
    fs::path normal = rel_path.lexically_normal();
    if (normal.is_absolute()) {
        return false;
    }
    for (const auto& part : normal) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

std::string sanitize_folder_name(const std::string& name) {
    std::string safe;
    for (unsigned char c : name) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.') {
            safe += static_cast<char>(c);
        } else if (std::isspace(c)) {
            safe += '_';
        }
    }
    return safe;
}

bool parse_manifest_json_content(const std::string& content, ExtensionMetadata& meta, std::string& error) {
    try {
        json data = json::parse(content);
        if (!data.is_object()) {
            error = "Manifest root must be a JSON object";
            return false;
        }

        const char* required_fields[] = {"name", "version", "description", "main"};
        for (const char* field : required_fields) {
            if (!data.contains(field) || !data[field].is_string() || data[field].get<std::string>().empty()) {
                error = std::string("Missing or empty required manifest field: '") + field + "'";
                return false;
            }
        }

        meta.name = data["name"].get<std::string>();
        meta.version = data["version"].get<std::string>();
        meta.description = data["description"].get<std::string>();
        meta.entry_point = data["main"].get<std::string>();
        if (data.contains("exttype") && data["exttype"].is_string()) {
            meta.extension_type = data["exttype"].get<std::string>();
        }

        if (!is_safe_relative_path(fs::path(meta.entry_point))) {
            error = "Unsafe 'main' entry point path: " + meta.entry_point;
            return false;
        }

        meta.valid = true;
        return true;
    } catch (const json::parse_error& e) {
        error = std::string("Invalid JSON in manifest: ") + e.what();
        return false;
    }
}

ExtensionMetadata parse_extension_json(const fs::path& extension_dir, std::string* out_error = nullptr) {
    ExtensionMetadata meta;
    fs::path json_path = extension_dir / "manifest.json";

    if (!fs::exists(json_path)) {
        json_path = extension_dir / "extension.json";
    }
    if (!fs::exists(json_path)) {
        std::error_code ec;
        for (const auto& entry : fs::directory_iterator(extension_dir, ec)) {
            if (entry.is_directory()) {
                if (fs::exists(entry.path() / "manifest.json")) {
                    json_path = entry.path() / "manifest.json";
                    break;
                }
                if (fs::exists(entry.path() / "extension.json")) {
                    json_path = entry.path() / "extension.json";
                    break;
                }
            }
        }
    }

    if (!fs::exists(json_path)) {
        if (out_error) *out_error = "No manifest.json found";
        return meta;
    }

    std::ifstream file(json_path);
    if (!file.is_open()) {
        if (out_error) *out_error = "Failed to open manifest: " + json_path.string();
        return meta;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string err;
    if (!parse_manifest_json_content(content, meta, err) && out_error) {
        *out_error = err;
    }
    return meta;
}

} // namespace

ArchiveFormat detect_archive_format(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (file.is_open()) {
        uint8_t magic[4] = {0};
        file.read(reinterpret_cast<char*>(magic), sizeof(magic));
        std::streamsize bytes_read = file.gcount();

        if (bytes_read >= 4 && magic[0] == 0x50 && magic[1] == 0x4B) {
            if ((magic[2] == 0x03 && magic[3] == 0x04) ||
                (magic[2] == 0x05 && magic[3] == 0x06) ||
                (magic[2] == 0x07 && magic[3] == 0x08)) {
                return ArchiveFormat::Zip;
            }
        }
    }

    fs::path p(filepath);
    std::string ext = to_lower(p.extension().string());
    if (ext == ".uce" || ext == ".zip") {
        return ArchiveFormat::Zip;
    }

    return ArchiveFormat::Unknown;
}

bool extract_zip(const std::string& archive_path, const std::string& destination_dir) {
    mz_zip_archive zip_archive;
    std::memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, archive_path.c_str(), 0)) {
        std::cerr << "[ex5] Error: failed to open extension archive: " << archive_path << "\n";
        return false;
    }

    fs::path dest(destination_dir);
    std::error_code ec;
    fs::create_directories(dest, ec);

    mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);
    for (mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            continue;
        }

        std::string filename = file_stat.m_filename;
        fs::path entry_rel_path(filename);

        if (!is_safe_relative_path(entry_rel_path)) {
            std::cerr << "[ex6] Warning: skipped unsafe path in archive: " << filename << "\n";
            continue;
        }

        fs::path target_path = dest / entry_rel_path.lexically_normal();

        if (mz_zip_reader_is_file_a_directory(&zip_archive, i) || (!filename.empty() && filename.back() == '/')) {
            fs::create_directories(target_path, ec);
        } else {
            if (target_path.has_parent_path()) {
                fs::create_directories(target_path.parent_path(), ec);
            }
            if (!mz_zip_reader_extract_to_file(&zip_archive, i, target_path.string().c_str(), 0)) {
                std::cerr << "[ex4] Error: failed to extract extension: " << filename << "\n";
                mz_zip_reader_end(&zip_archive);
                return false;
            }
        }
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}

int dearchive(const std::string& extension_file, const std::string& destination_dir) {
    ArchiveFormat format = detect_archive_format(extension_file);
    switch (format) {
        case ArchiveFormat::Zip:
            return extract_zip(extension_file, destination_dir) ? 0 : 1;
        default:
            std::cerr << "[ex2] Error: unsupported extension format for " << extension_file << "\n";
            return 2;
    }
}

bool validate_extension(const std::string& archive_path, ExtensionMetadata* out_meta, std::string* out_error) {
    if (!fs::exists(archive_path)) {
        if (out_error) *out_error = "Archive file does not exist: " + archive_path;
        return false;
    }
    if (detect_archive_format(archive_path) != ArchiveFormat::Zip) {
        if (out_error) *out_error = "Unsupported archive format (expected .uce or .zip): " + archive_path;
        return false;
    }

    mz_zip_archive zip_archive;
    std::memset(&zip_archive, 0, sizeof(zip_archive));
    if (!mz_zip_reader_init_file(&zip_archive, archive_path.c_str(), 0)) {
        if (out_error) *out_error = "Failed to open zip archive: " + archive_path;
        return false;
    }

    mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);
    int manifest_index = -1;

    for (mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            continue;
        }
        std::string filename = file_stat.m_filename;
        if (!is_safe_relative_path(fs::path(filename))) {
            mz_zip_reader_end(&zip_archive);
            if (out_error) *out_error = "Archive contains unsafe path: " + filename;
            return false;
        }

        fs::path p(filename);
        if (p.filename() == "manifest.json") {
            // Prefer top-level manifest.json
            if (manifest_index == -1 || !p.has_parent_path()) {
                manifest_index = static_cast<int>(i);
            }
        }
    }

    if (manifest_index < 0) {
        mz_zip_reader_end(&zip_archive);
        if (out_error) *out_error = "Archive is missing required manifest.json";
        return false;
    }

    size_t uncomp_size = 0;
    void* data = mz_zip_reader_extract_to_heap(&zip_archive, static_cast<mz_uint>(manifest_index), &uncomp_size, 0);
    if (!data) {
        mz_zip_reader_end(&zip_archive);
        if (out_error) *out_error = "Failed to read manifest.json from archive";
        return false;
    }

    std::string manifest_content(static_cast<const char*>(data), uncomp_size);
    mz_free(data);
    mz_zip_reader_end(&zip_archive);

    ExtensionMetadata meta;
    std::string parse_err;
    if (!parse_manifest_json_content(manifest_content, meta, parse_err)) {
        if (out_error) *out_error = parse_err;
        return false;
    }

    if (out_meta) {
        *out_meta = meta;
    }
    return true;
}

std::vector<ExtensionMetadata> list_installed_extensions(const std::string& target_dir) {
    std::vector<ExtensionMetadata> result;
    fs::path base = target_dir.empty() ? default_extensions_dir() : fs::path(target_dir);
    if (!fs::exists(base) || !fs::is_directory(base)) {
        return result;
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(base, ec)) {
        if (!entry.is_directory()) {
            continue;
        }
        ExtensionMetadata meta = parse_extension_json(entry.path());
        if (meta.valid) {
            result.push_back(meta);
        }
    }
    return result;
}

int install_extension(const std::string& extension_file, const std::string& custom_base_dir) {
    ExtensionMetadata validated_meta;
    std::string validation_err;
    if (!validate_extension(extension_file, &validated_meta, &validation_err)) {
        std::cerr << "[ex3] Error: invalid extension archive: " << validation_err << "\n";
        return 1;
    }

    fs::path base_folder = custom_base_dir.empty()
        ? default_extensions_dir()
        : fs::path(custom_base_dir);

    std::error_code ec;
    fs::create_directories(base_folder, ec);
    if (ec) {
        std::cerr << "Error: could not create base extension directory: " << base_folder << "\n";
        return 1;
    }

    std::string safe_name = sanitize_folder_name(validated_meta.name);
    if (safe_name.empty()) {
        safe_name = "unnamed_extension";
    }

    fs::path target_folder = base_folder / safe_name;
    fs::remove_all(target_folder, ec);
    fs::create_directories(target_folder, ec);

    int status = dearchive(extension_file, target_folder.string());
    if (status != 0) {
        return status;
    }

    std::cout << "Installed extension: " << validated_meta.name
              << " (v" << validated_meta.version << ") -> " << target_folder << "\n";
    return 0;
}

#ifndef EXTENSIONPARSER_NO_MAIN
static void print_usage(const char* prog) {
    std::cout << "Usage:\n"
              << "  " << prog << " install <archive.uce> [-o <dir>]   Install an extension\n"
              << "  " << prog << " list [-d <dir>]                    List installed extensions\n"
              << "  " << prog << " validate <archive.uce>             Validate an extension archive\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "--help" || cmd == "-h") {
        print_usage(argv[0]);
        return 0;
    }

    if (cmd == "list") {
        std::string target_dir;
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-d" || arg == "--dir" || arg == "-o" || arg == "--output") && i + 1 < argc) {
                target_dir = argv[++i];
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

    if (cmd == "validate") {
        if (argc < 3) {
            std::cerr << "Error: validate requires <archive.uce>\n";
            return 1;
        }
        ExtensionMetadata meta;
        std::string err;
        if (!validate_extension(argv[2], &meta, &err)) {
            std::cerr << "Validation failed: " << err << "\n";
            return 1;
        }
        std::cout << "Valid extension: " << meta.name << " v" << meta.version
                  << " (main: " << meta.entry_point << ")\n";
        return 0;
    }

    if (cmd == "install") {
        if (argc < 3) {
            std::cerr << "Error: install requires <archive.uce>\n";
            return 1;
        }
        std::string archive_file = argv[2];
        std::string dest_dir;
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
                dest_dir = argv[++i];
            }
        }
        return install_extension(archive_file, dest_dir);
    }

    // Fallback: treat first positional arg as archive path
    std::string archive_file = cmd;
    std::string dest_dir;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            dest_dir = argv[++i];
        } else if (dest_dir.empty() && arg[0] != '-') {
            dest_dir = arg;
        }
    }
    return install_extension(archive_file, dest_dir);
}
#endif
