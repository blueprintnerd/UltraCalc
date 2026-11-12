// orbit_hide:collapsed (Includes (dependencies)
#include "extensionparser.hpp"
#include "miniz.c"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
// orbit_hide:end

namespace fs = std::filesystem;
using json = nlohmann::json;


struct ExtensionMetadata {
    std::string name;
    std::string version = "1.0.0";
    std::string description;
    std::string entry_point;
    std::string extension_type;
    bool valid = false;
};

static std::string get_username() {
    const char* user = std::getenv("USER");
    if (!user) user = std::getenv("USERNAME"); 
    return user ? std::string(user) : "default_user";
}

// orbit_hide:collapsed (Unzipping functions)
static std::string to_lower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lower;
}

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
    if (ext == ".zip") {
        return ArchiveFormat::Zip;
    }

    return ArchiveFormat::Unknown;
}

static bool is_safe_relative_path(const fs::path& rel_path) {
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

int dearchive(const std::string& extension_file, const std::string& destiination_dir) {
    ArchiveFormat format = detect_archive_format(extension_file);
    switch (format) {
        case ArchiveFormat::Zip:
            return extract_zip(extension_file, destination_dir) ? 0 : 1;
        default:
            std::cerr << "[ex2] Error: unsupported extension format for " << extension_file << "\n";
            return 2;
    }
}
// orbit_hide:end

static std::string sanitize_folder_name(const std::string& name) {
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

ExtensionMetadata parse_extension_json(const fs::path& extension_dir) {
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
        return meta;
    }

    std::ifstream file(json_path);
    if (!file.is_open()) {
        std::cerr << "[ex2] (error) Failed to open JSON manifest: " << json_path << "\n";
        return meta;
    }

    try {
        json data = json::parse(file);
        meta.name        = data.value("name", "");
        meta.version     = data.value("version", "1.0.0");
        meta.description = data.value("description", "");
        meta.entry_point = data.value("main", "");
        meta.extensiontype = data.value("exttype", "");
        meta.valid       = !meta.name.empty();
    } catch (const json::parse_error& e) {
        std::cerr << "[ex3] (error) Invalid JSON in " << json_path << ": " << e.what() << "\n";
    }

    return meta;
}
int install_extension(const std::string& extension_file, const std::string& custom_base_dir = "") {
    std::string username = get_username();

    fs::path base_folder = custom_base_dir.empty()
        ? fs::path("/home") / username / "UltraCalc" / "extension"
        : fs::path(custom_base_dir);

    std::error_code ec;
    fs::create_directories(base_folder, ec);
    if (ec) {
        std::cerr << "Error: could not create base extension directory: " << base_folder << "\n";
        return 1;
    }

    
    fs::path archive_path(extension_file);
    std::string stem = archive_path.stem().string();
    if (stem.empty()) {
        stem = "unnamed_extension";
    }

    fs::path target_folder = base_folder / stem;
    fs::create_directories(target_folder, ec);

    int status = dearchive(extension_file, target_folder.string());
    if (status != 0) {
        return status;
    }

    // Check for JSON metadata and rename the extension folder to match the manifest name if present
    ExtensionMetadata meta = parse_extension_json(target_folder);
    if (meta.valid) {
        std::string safe_name = sanitize_folder_name(meta.name);
        if (!safe_name.empty()) {
            fs::path new_folder = base_folder / safe_name;
            if (new_folder != target_folder) {
                fs::remove_all(new_folder, ec); // Overwrite previous install of the same extensio
                fs::rename(target_folder, new_folder, ec);
                if (!ec) {
                    target_folder = new_folder;
                }
            }
        }
        std::cout << "Installed extension: " << meta.name << " (v" << meta.version << ") -> " << target_folder << "\n";
    } else {
        std::cout << "Extracted extension (no JSON manifest found) -> " << target_folder << "\n";
    }

    return 0;
}

#ifndef EXTENSIONPARSER_NO_MAIN
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <extension.zip> [-o <output_base_dir>]\n";
        return 1;
    }

    std::string archive_file;
    std::string dest_dir;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            dest_dir = argv[++i];
        } else if (archive_file.empty() && arg[0] != '-') {
            archive_file = arg;
        } else if (dest_dir.empty() && arg[0] != '-') {
            dest_dir = arg;
        }
    }

    if (archive_file.empty()) {
        std::cerr << "[ex1] (error) Extension not selected\n";
        return 1;
    }

    return install_extension(archive_file, dest_dir);
}
