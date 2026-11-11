#pragma once

#include <string>

enum class ArchiveFormat {
    Unknown,
    Zip
};

ArchiveFormat detect_archive_format(const std::string& filepath);
bool extract_zip(const std::string& archive_path, const std::string& destination_dir);
int dearchive(const std::string& extension_file, const std::string& destination_dir = ".");
