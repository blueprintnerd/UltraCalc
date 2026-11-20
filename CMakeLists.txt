cmake_minimum_required(VERSION 3.12)
project(UltraCalc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(THIRD_PARTY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/include")

# 1. ultracalc-cli
add_executable(ultracalc-cli cli.cpp math.cpp extensionparser.cpp)
target_compile_definitions(ultracalc-cli PRIVATE EXTENSIONPARSER_NO_MAIN)
target_include_directories(ultracalc-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${THIRD_PARTY_INCLUDE_DIR})

# 2. ultracalc-ext
add_executable(ultracalc-ext extensionparser.cpp)
target_include_directories(ultracalc-ext PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${THIRD_PARTY_INCLUDE_DIR})

# 3. ultracalc-server
add_executable(ultracalc-server serve.cpp math.cpp)
target_include_directories(ultracalc-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${THIRD_PARTY_INCLUDE_DIR})
target_link_libraries(ultracalc-server PRIVATE Threads::Threads)

# 4. ultracalc-gui (native C++ launcher wrapper for guiqt.py)
add_executable(ultracalc-gui gui_launcher.cpp)

# 5. ultracalc-tui (FTXUI TUI calculator)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/CMakeLists.txt")
    add_subdirectory(FTXUI)

    add_executable(ultracalc-tui math.cpp tui.cpp)
    target_link_libraries(ultracalc-tui PRIVATE ftxui::screen ftxui::component ftxui::dom)
    target_include_directories(ultracalc-tui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include)

    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(example_source ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_source} NAME_WE)
        add_executable(${example_name} ${example_source} math.cpp)
        target_link_libraries(${example_name} PRIVATE ftxui::screen ftxui::component ftxui::dom)
        target_include_directories(${example_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/FTXUI/include)
    endforeach()
endif()

# Testing
enable_testing()

add_executable(test_math tests/test_math.cpp math.cpp)
target_include_directories(test_math PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_math COMMAND test_math)

add_executable(test_extensionparser tests/test_extensionparser.cpp extensionparser.cpp)
target_compile_definitions(test_extensionparser PRIVATE EXTENSIONPARSER_NO_MAIN)
target_include_directories(test_extensionparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${THIRD_PARTY_INCLUDE_DIR})
add_test(NAME test_extensionparser COMMAND test_extensionparser)

add_test(
    NAME test_cli
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cli.sh" "$<TARGET_FILE:ultracalc-cli>"
)

add_test(
    NAME test_server
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_server.sh" "$<TARGET_FILE:ultracalc-server>"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
