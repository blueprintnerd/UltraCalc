#include "extensionparser.hpp"

#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

static void run_command(const std::string& cmd) {
    int res = std::system(cmd.c_str());
    assert(res == 0);
}

int main() {
    fs::path test_dir = fs::temp_directory_path() / "extensionparser_tests";
    fs::create_directories(test_dir);

    fs::path src_dir = test_dir / "src";
    fs::create_directories(src_dir / "layout");
    fs::create_directories(src_dir / "photos");

    {
        std::ofstream f(src_dir / "manifest.json");
        f << "{\"name\":\"TestExtension\",\"version\":\"2.0\"}";
    }
    {
        std::ofstream f(src_dir / "layout" / "calc.json");
        f << "{\"theme\":\"dark\"}";
    }
    {
        std::ofstream f(src_dir / "photos" / "logo.png");
        f << "fake_image_bytes";
    }

    fs::path zip_file = test_dir / "bundle.zip";
    fs::path tar_xz_file = test_dir / "bundle.tar.xz";
    fs::path single_xz_file = test_dir / "config.json.xz";

    // Generate zip and xz test archives using python
    std::string gen_cmd = "python3 -c \""
        "import zipfile, tarfile, lzma; "
        "zf = zipfile.ZipFile('" + zip_file.string() + "', 'w'); "
        "zf.write('" + (src_dir / "manifest.json").string() + "', 'manifest.json'); "
        "zf.write('" + (src_dir / "layout" / "calc.json").string() + "', 'layout/calc.json'); "
        "zf.write('" + (src_dir / "photos" / "logo.png").string() + "', 'photos/logo.png'); "
        "zf.close(); "
        "tf = tarfile.open('" + tar_xz_file.string() + "', 'w:xz'); "
        "tf.add('" + (src_dir / "manifest.json").string() + "', arcname='manifest.json'); "
        "tf.add('" + (src_dir / "layout" / "calc.json").string() + "', arcname='layout/calc.json'); "
        "tf.add('" + (src_dir / "photos" / "logo.png").string() + "', arcname='photos/logo.png'); "
        "tf.close(); "
        "f = lzma.open('" + single_xz_file.string() + "', 'wb'); "
        "f.write(b'{\\\"single\\\": true}'); "
        "f.close();\"";
    run_command(gen_cmd);

    // Test 1: detect_archive_format
    std::cout << "Running Test 1: detect_archive_format..." << std::endl;
    assert(detect_archive_format(zip_file.string()) == ArchiveFormat::Zip);
    assert(detect_archive_format(tar_xz_file.string()) == ArchiveFormat::Xz);
    assert(detect_archive_format(single_xz_file.string()) == ArchiveFormat::Xz);
    assert(detect_archive_format((src_dir / "manifest.json").string()) == ArchiveFormat::Unknown);
    std::cout << "Test 1 passed." << std::endl;

    // Test 2: dearchive zip
    std::cout << "Running Test 2: dearchive ZIP..." << std::endl;
    fs::path out_zip = test_dir / "out_zip";
    int ret_zip = dearchive(zip_file.string(), out_zip.string());
    assert(ret_zip == 0);
    assert(fs::exists(out_zip / "manifest.json"));
    assert(fs::exists(out_zip / "layout" / "calc.json"));
    assert(fs::exists(out_zip / "photos" / "logo.png"));

    std::ifstream zip_manifest(out_zip / "manifest.json");
    std::string zip_manifest_content((std::istreambuf_iterator<char>(zip_manifest)),
                                     std::istreambuf_iterator<char>());
    assert(zip_manifest_content == "{\"name\":\"TestExtension\",\"version\":\"2.0\"}");
    std::cout << "Test 2 passed." << std::endl;

    // Test 3: dearchive tar.xz
    std::cout << "Running Test 3: dearchive TAR.XZ..." << std::endl;
    fs::path out_tar_xz = test_dir / "out_tar_xz";
    int ret_tar_xz = dearchive(tar_xz_file.string(), out_tar_xz.string());
    assert(ret_tar_xz == 0);
    assert(fs::exists(out_tar_xz / "manifest.json"));
    assert(fs::exists(out_tar_xz / "layout" / "calc.json"));
    assert(fs::exists(out_tar_xz / "photos" / "logo.png"));

    std::ifstream tar_manifest(out_tar_xz / "manifest.json");
    std::string tar_manifest_content((std::istreambuf_iterator<char>(tar_manifest)),
                                     std::istreambuf_iterator<char>());
    assert(tar_manifest_content == "{\"name\":\"TestExtension\",\"version\":\"2.0\"}");
    std::cout << "Test 3 passed." << std::endl;

    // Test 4: dearchive standalone .xz
    std::cout << "Running Test 4: dearchive standalone XZ..." << std::endl;
    fs::path out_single_xz = test_dir / "out_single_xz";
    int ret_single_xz = dearchive(single_xz_file.string(), out_single_xz.string());
    assert(ret_single_xz == 0);
    assert(fs::exists(out_single_xz / "config.json"));

    std::ifstream single_f(out_single_xz / "config.json");
    std::string single_content((std::istreambuf_iterator<char>(single_f)),
                               std::istreambuf_iterator<char>());
    assert(single_content == "{\"single\": true}");
    std::cout << "Test 4 passed." << std::endl;

    // Test 5: non-existent file error handling
    std::cout << "Running Test 5: Error handling on nonexistent file..." << std::endl;
    int ret_err = dearchive((test_dir / "missing.zip").string(), out_zip.string());
    assert(ret_err != 0);
    std::cout << "Test 5 passed." << std::endl;

    // Clean up temporary test files
    fs::remove_all(test_dir);
    std::cout << "All extensionparser tests passed successfully!" << std::endl;
    return 0;
}
