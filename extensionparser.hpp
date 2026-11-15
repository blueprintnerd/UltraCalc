#pragma once

#include <string>
#include <vector>

enum class ArchiveFormat {
    Unknown,
    Zip
};

struct ExtensionMetadata {
    std::string name;
    std::string version;
    std::string description;
    std::string entry_point;
    std::string extension_type;
    bool valid = false;
};

ArchiveFormat detect_archive_format(const std::string& filepath);
bool extract_zip(const std::string& archive_path, const std::string& destination_dir);
int dearchive(const std::string& extension_file, const std::string& destination_dir = ".");
bool validate_extension(const std::string& archive_path, ExtensionMetadata* out_meta = nullptr, std::string* out_error = nullptr);
std::vector<ExtensionMetadata> list_installed_extensions(const std::string& target_dir = "");
int install_extension(const std::string& extension_file, const std::string& custom_base_dir = "");
