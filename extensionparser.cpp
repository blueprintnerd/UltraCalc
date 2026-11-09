// orbit_hide:collapsed (Includes (dependencies))
#include "extensionparser.hpp"
#include "miniz.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <lzma.h>
// orbit_hide:end

namespace fs = std::filesystem;

// archive contains photos, manifest and layout. converted zip maybe.

// orbit_hide:collapsed (Unzipping functions)
struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

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
        uint8_t magic[6] = {0};
        file.read(reinterpret_cast<char*>(magic), sizeof(magic));
        std::streamsize bytes_read = file.gcount();

        if (bytes_read >= 4 && magic[0] == 0x50 && magic[1] == 0x4B) {
            if ((magic[2] == 0x03 && magic[3] == 0x04) ||
                (magic[2] == 0x05 && magic[3] == 0x06) ||
                (magic[2] == 0x07 && magic[3] == 0x08)) {
                return ArchiveFormat::Zip;
            }
        }

        if (bytes_read >= 6 && magic[0] == 0xFD && magic[1] == '7' &&
            magic[2] == 'z' && magic[3] == 'X' && magic[4] == 'Z' && magic[5] == 0x00) {
            return ArchiveFormat::Xz;
        }
    }

    fs::path p(filepath);
    std::string ext = to_lower(p.extension().string());
    if (ext == ".zip") {
        return ArchiveFormat::Zip;
    }
    if (ext == ".xz" || ext == ".txz") {
        return ArchiveFormat::Xz;
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
        std::cerr << "Error: failed to open zip archive: " << archive_path << "\n";
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
                std::cerr << "Error: failed to extract zip entry: " << filename << "\n";
                mz_zip_reader_end(&zip_archive);
                return false;
            }
        }
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}

static uint64_t parse_tar_octal(const char* str, size_t len) {
    uint64_t val = 0;
    size_t i = 0;
    while (i < len && (str[i] == ' ' || str[i] == '\0')) i++;
    while (i < len && str[i] >= '0' && str[i] <= '7') {
        val = (val << 3) + static_cast<uint64_t>(str[i] - '0');
        i++;
    }
    return val;
}

static bool is_tar_block_zero(const uint8_t* block) {
    for (size_t i = 0; i < 512; ++i) {
        if (block[i] != 0) return false;
    }
    return true;
}

static bool verify_tar_checksum(const TarHeader* hdr) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(hdr);
    uint32_t unsigned_sum = 0;
    int32_t signed_sum = 0;
    for (size_t i = 0; i < 512; ++i) {
        if (i >= 148 && i < 156) {
            unsigned_sum += ' ';
            signed_sum += ' ';
        } else {
            unsigned_sum += bytes[i];
            signed_sum += static_cast<int8_t>(bytes[i]);
        }
    }
    uint64_t stored_chk = parse_tar_octal(hdr->chksum, sizeof(hdr->chksum));
    return (stored_chk == unsigned_sum || stored_chk == static_cast<uint64_t>(signed_sum));
}

static bool is_tar_archive(const std::vector<uint8_t>& data) {
    if (data.size() < 512) return false;
    const TarHeader* hdr = reinterpret_cast<const TarHeader*>(data.data());
    return verify_tar_checksum(hdr);
}

static bool extract_tar_stream(const std::vector<uint8_t>& tar_data, const fs::path& dest_dir) {
    size_t offset = 0;
    std::string next_long_name;
    std::error_code ec;

    while (offset + 512 <= tar_data.size()) {
        const uint8_t* block = tar_data.data() + offset;
        if (is_tar_block_zero(block)) {
            offset += 512;
            continue;
        }

        const TarHeader* hdr = reinterpret_cast<const TarHeader*>(block);
        if (!verify_tar_checksum(hdr)) {
            break;
        }

        uint64_t file_size = parse_tar_octal(hdr->size, sizeof(hdr->size));
        size_t data_offset = offset + 512;
        size_t block_padding = ((file_size + 511) / 512) * 512;

        if (data_offset + file_size > tar_data.size()) {
            return false;
        }

        std::string entry_name;
        if (!next_long_name.empty()) {
            entry_name = next_long_name;
            next_long_name.clear();
        } else {
            if (std::memcmp(hdr->magic, "ustar", 5) == 0 && hdr->prefix[0] != '\0') {
                std::string prefix(hdr->prefix, strnlen(hdr->prefix, sizeof(hdr->prefix)));
                std::string name(hdr->name, strnlen(hdr->name, sizeof(hdr->name)));
                entry_name = prefix + "/" + name;
            } else {
                entry_name = std::string(hdr->name, strnlen(hdr->name, sizeof(hdr->name)));
            }
        }

        if (hdr->typeflag == 'L') {
            next_long_name = std::string(reinterpret_cast<const char*>(tar_data.data() + data_offset), file_size);
            while (!next_long_name.empty() && (next_long_name.back() == '\0' || next_long_name.back() == '\n')) {
                next_long_name.pop_back();
            }
            offset = data_offset + block_padding;
            continue;
        }

        if (hdr->typeflag == 'x' || hdr->typeflag == 'g') {
            std::string pax_str(reinterpret_cast<const char*>(tar_data.data() + data_offset), file_size);
            std::istringstream stream(pax_str);
            std::string line;
            while (std::getline(stream, line)) {
                size_t space_pos = line.find(' ');
                if (space_pos != std::string::npos) {
                    size_t eq_pos = line.find('=', space_pos + 1);
                    if (eq_pos != std::string::npos) {
                        std::string key = line.substr(space_pos + 1, eq_pos - (space_pos + 1));
                        std::string val = line.substr(eq_pos + 1);
                        if (!val.empty() && val.back() == '\n') val.pop_back();
                        if (key == "path") {
                            next_long_name = val;
                        }
                    }
                }
            }
            offset = data_offset + block_padding;
            continue;
        }

        fs::path entry_rel_path(entry_name);
        if (!is_safe_relative_path(entry_rel_path)) {
            offset = data_offset + block_padding;
            continue;
        }

        fs::path target_path = dest_dir / entry_rel_path.lexically_normal();

        if (hdr->typeflag == '5' || (!entry_name.empty() && entry_name.back() == '/')) {
            fs::create_directories(target_path, ec);
        } else if (hdr->typeflag == '0' || hdr->typeflag == '\0') {
            if (target_path.has_parent_path()) {
                fs::create_directories(target_path.parent_path(), ec);
            }
            std::ofstream out(target_path, std::ios::binary);
            if (out.is_open()) {
                if (file_size > 0) {
                    out.write(reinterpret_cast<const char*>(tar_data.data() + data_offset), file_size);
                }
            }
        }

        offset = data_offset + block_padding;
    }
    return true;
}

static bool decompress_xz_stream(const std::string& input_path, std::vector<uint8_t>& out_buffer) {
    std::ifstream in(input_path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        return false;
    }

    const size_t in_buf_size = 65536;
    const size_t out_buf_size = 65536;
    std::vector<uint8_t> in_buf(in_buf_size);
    std::vector<uint8_t> temp_out_buf(out_buf_size);

    lzma_action action = LZMA_RUN;
    strm.next_in = nullptr;
    strm.avail_in = 0;

    bool success = false;
    while (true) {
        if (strm.avail_in == 0 && !in.eof()) {
            in.read(reinterpret_cast<char*>(in_buf.data()), in_buf.size());
            strm.avail_in = in.gcount();
            strm.next_in = in_buf.data();
            if (in.eof()) {
                action = LZMA_FINISH;
            }
        }

        strm.next_out = temp_out_buf.data();
        strm.avail_out = temp_out_buf.size();

        ret = lzma_code(&strm, action);

        if (strm.avail_out < temp_out_buf.size()) {
            size_t produced = temp_out_buf.size() - strm.avail_out;
            out_buffer.insert(out_buffer.end(), temp_out_buf.data(), temp_out_buf.data() + produced);
        }

        if (ret == LZMA_STREAM_END) {
            success = true;
            break;
        }
        if (ret != LZMA_OK) {
            std::cerr << "Error: lzma decoding failure (" << ret << ")\n";
            break;
        }
    }

    lzma_end(&strm);
    return success;
}

bool extract_xz(const std::string& archive_path, const std::string& destination_dir) {
    std::vector<uint8_t> decompressed_data;
    if (!decompress_xz_stream(archive_path, decompressed_data)) {
        std::cerr << "Error: failed to decompress xz archive: " << archive_path << "\n";
        return false;
    }

    fs::path dest(destination_dir);
    std::error_code ec;
    fs::create_directories(dest, ec);

    if (is_tar_archive(decompressed_data)) {
        return extract_tar_stream(decompressed_data, dest);
    }

    // Standalone file decompressed from xz
    fs::path p(archive_path);
    std::string stem_name = p.stem().string();
    fs::path target_file = dest / stem_name;
    if (target_file.has_parent_path()) {
        fs::create_directories(target_file.parent_path(), ec);
    }

    std::ofstream out(target_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: failed to write decompressed file: " << target_file << "\n";
        return false;
    }

    if (!decompressed_data.empty()) {
        out.write(reinterpret_cast<const char*>(decompressed_data.data()), decompressed_data.size());
    }
    return true;
}

int dearchive(const std::string& extension_file, const std::string& destination_dir) {
    ArchiveFormat format = detect_archive_format(extension_file);
    switch (format) {
        case ArchiveFormat::Zip:
            return extract_zip(extension_file, destination_dir) ? 0 : 1;
        case ArchiveFormat::Xz:
            return extract_xz(extension_file, destination_dir) ? 0 : 1;
        default:
            std::cerr << "Error: unsupported archive format for " << extension_file << "\n";
            return 2;
    }
}

// orbit_hide:end

#ifndef EXTENSIONPARSER_NO_MAIN
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: extensionparser <archive_file> [destination_dir]\n";
        std::cout << "Supported formats: .zip, .xz, .tar.xz\n";
        return 1;
    }

    std::string archive_file;
    std::string dest_dir = ".";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: extensionparser <archive_file> [destination_dir]\n";
            std::cout << "Options:\n";
            std::cout << "  -o, --output <dir>   Set output destination directory\n";
            std::cout << "  -h, --help           Display help information\n";
            return 0;
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            dest_dir = argv[++i];
        } else if (archive_file.empty() && arg[0] != '-') {
            archive_file = arg;
        } else if (dest_dir == "." && arg[0] != '-') {
            dest_dir = arg;
        }
    }

    if (archive_file.empty()) {
        std::cerr << "Error: no archive file specified\n";
        return 1;
    }

    return dearchive(archive_file, dest_dir);
}
#endif
