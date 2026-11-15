#include "extensionparser.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static void run_command(const std::string& cmd) {
    int res = std::system(cmd.c_str());
    assert(res == 0);
}

int main() {
    fs::path test_dir = fs::temp_directory_path() / "extensionparser_tests";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);

    fs::path src_dir = test_dir / "src";
    fs::create_directories(src_dir / "layout");
    fs::create_directories(src_dir / "photos");

    const std::string valid_manifest =
        "{\"name\":\"TestExtension\",\"version\":\"2.0.0\",\"description\":\"Test extension\",\"main\":\"layout/calc.json\"}";

    {
        std::ofstream f(src_dir / "manifest.json");
        f << valid_manifest;
    }
    {
        std::ofstream f(src_dir / "layout" / "calc.json");
        f << "{\"theme\":\"dark\"}";
    }
    {
        std::ofstream f(src_dir / "photos" / "logo.png");
        f << "fake_image_bytes";
    }

    fs::path zip_file = test_dir / "bundle.uce";
    fs::path non_zip_file = test_dir / "bundle.txt";
    fs::path invalid_manifest_zip = test_dir / "invalid_manifest.zip";
    fs::path missing_manifest_zip = test_dir / "missing_manifest.zip";
    fs::path traversal_zip = test_dir / "traversal.zip";

    {
        std::ofstream f(non_zip_file);
        f << "not a zip file";
    }

    // Generate valid zip test archive using python
    std::string gen_cmd = "python3 -c \""
        "import zipfile; "
        "zf = zipfile.ZipFile('" + zip_file.string() + "', 'w'); "
        "zf.write('" + (src_dir / "manifest.json").string() + "', 'manifest.json'); "
        "zf.write('" + (src_dir / "layout" / "calc.json").string() + "', 'layout/calc.json'); "
        "zf.write('" + (src_dir / "photos" / "logo.png").string() + "', 'photos/logo.png'); "
        "zf.close();\"";
    run_command(gen_cmd);

    // Generate zip with incomplete manifest (missing required 'main' and 'description')
    std::string gen_invalid_cmd = "python3 -c \""
        "import zipfile; "
        "zf = zipfile.ZipFile('" + invalid_manifest_zip.string() + "', 'w'); "
        "zf.writestr('manifest.json', '{\\\"name\\\":\\\"Broken\\\",\\\"version\\\":\\\"1.0\\\"}'); "
        "zf.close();\"";
    run_command(gen_invalid_cmd);

    // Generate zip without manifest.json
    std::string gen_missing_cmd = "python3 -c \""
        "import zipfile; "
        "zf = zipfile.ZipFile('" + missing_manifest_zip.string() + "', 'w'); "
        "zf.writestr('readme.txt', 'hello'); "
        "zf.close();\"";
    run_command(gen_missing_cmd);

    // Generate zip with directory traversal entry
    std::string gen_traversal_cmd = "python3 -c \""
        "import zipfile; "
        "zf = zipfile.ZipFile('" + traversal_zip.string() + "', 'w'); "
        "zf.writestr('manifest.json', '" + valid_manifest + "'); "
        "zf.writestr('../evil.txt', 'bad'); "
        "zf.close();\"";
    run_command(gen_traversal_cmd);

    // Test 1: detect_archive_format
    std::cout << "Running Test 1: detect_archive_format..." << std::endl;
    assert(detect_archive_format(zip_file.string()) == ArchiveFormat::Zip);
    assert(detect_archive_format(non_zip_file.string()) == ArchiveFormat::Unknown);
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
    assert(zip_manifest_content == valid_manifest);
    std::cout << "Test 2 passed." << std::endl;

    // Test 3: unsupported archive format error handling
    std::cout << "Running Test 3: Error handling on unsupported format..." << std::endl;
    int ret_unsupported = dearchive(non_zip_file.string(), out_zip.string());
    assert(ret_unsupported != 0);
    std::cout << "Test 3 passed." << std::endl;

    // Test 4: non-existent file error handling
    std::cout << "Running Test 4: Error handling on nonexistent file..." << std::endl;
    int ret_err = dearchive((test_dir / "missing.zip").string(), out_zip.string());
    assert(ret_err != 0);
    std::cout << "Test 4 passed." << std::endl;

    // Test 5: validate_extension (valid, missing fields, missing manifest, path traversal)
    std::cout << "Running Test 5: validate_extension..." << std::endl;
    ExtensionMetadata meta;
    std::string err;
    assert(validate_extension(zip_file.string(), &meta, &err));
    assert(meta.valid);
    assert(meta.name == "TestExtension");
    assert(meta.version == "2.0.0");
    assert(meta.description == "Test extension");
    assert(meta.entry_point == "layout/calc.json");

    assert(!validate_extension(invalid_manifest_zip.string(), nullptr, &err));
    assert(!validate_extension(missing_manifest_zip.string(), nullptr, &err));
    assert(!validate_extension(traversal_zip.string(), nullptr, &err));
    std::cout << "Test 5 passed." << std::endl;

    // Test 6: install_extension and list_installed_extensions
    std::cout << "Running Test 6: install_extension and list_installed_extensions..." << std::endl;
    fs::path install_dir = test_dir / "installed";
    assert(install_extension(zip_file.string(), install_dir.string()) == 0);
    assert(fs::exists(install_dir / "TestExtension" / "manifest.json"));
    auto installed = list_installed_extensions(install_dir.string());
    assert(installed.size() == 1);
    assert(installed[0].name == "TestExtension");
    assert(installed[0].version == "2.0.0");
    std::cout << "Test 6 passed." << std::endl;

    // Clean up temporary test files
    fs::remove_all(test_dir);
    std::cout << "All extensionparser tests passed successfully!" << std::endl;
    return 0;
}
