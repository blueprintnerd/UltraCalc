cmake_minimum_required(VERSION 3.12)
project(UltraCalc)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LibLZMA REQUIRED)

# Extension Parser executable
add_executable(extensionparser extensionparser.cpp miniz.c)
target_include_directories(extensionparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(extensionparser PRIVATE LibLZMA::LibLZMA)

# Testing
enable_testing()
add_executable(test_extensionparser tests/test_extensionparser.cpp extensionparser.cpp miniz.c)
target_compile_definitions(test_extensionparser PRIVATE EXTENSIONPARSER_NO_MAIN)
target_include_directories(test_extensionparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_extensionparser PRIVATE LibLZMA::LibLZMA)
add_test(NAME test_extensionparser COMMAND test_extensionparser)

# Add FTXUI as a subdirectory if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/CMakeLists.txt")
    add_subdirectory(FTXUI)

    # Main UltraCalc executable
    add_executable(ultra_calc math.cpp tui.cpp)
    target_link_libraries(ultra_calc PRIVATE ftxui::screen ftxui::component ftxui::dom)
    target_include_directories(ultra_calc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include)

    # Example executables
    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(example_source ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source} math.cpp)
        target_link_libraries(${example_name} PRIVATE ftxui::screen ftxui::component ftxui::dom)
        target_include_directories(${example_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include)
    endforeach()
endif()
