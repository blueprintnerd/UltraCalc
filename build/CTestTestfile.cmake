# CMake generated Testfile for 
# Source directory: /vercel/sandbox/UltraCalc
# Build directory: /vercel/sandbox/UltraCalc/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_math "/vercel/sandbox/UltraCalc/build/test_math")
set_tests_properties(test_math PROPERTIES  _BACKTRACE_TRIPLES "/vercel/sandbox/UltraCalc/CMakeLists.txt;53;add_test;/vercel/sandbox/UltraCalc/CMakeLists.txt;0;")
add_test(test_extensionparser "/vercel/sandbox/UltraCalc/build/test_extensionparser")
set_tests_properties(test_extensionparser PROPERTIES  _BACKTRACE_TRIPLES "/vercel/sandbox/UltraCalc/CMakeLists.txt;58;add_test;/vercel/sandbox/UltraCalc/CMakeLists.txt;0;")
add_test(test_cli "bash" "/vercel/sandbox/UltraCalc/tests/test_cli.sh" "/vercel/sandbox/UltraCalc/build/ultracalc-cli")
set_tests_properties(test_cli PROPERTIES  _BACKTRACE_TRIPLES "/vercel/sandbox/UltraCalc/CMakeLists.txt;60;add_test;/vercel/sandbox/UltraCalc/CMakeLists.txt;0;")
add_test(test_server "bash" "/vercel/sandbox/UltraCalc/tests/test_server.sh" "/vercel/sandbox/UltraCalc/build/ultracalc-server")
set_tests_properties(test_server PROPERTIES  WORKING_DIRECTORY "/vercel/sandbox/UltraCalc" _BACKTRACE_TRIPLES "/vercel/sandbox/UltraCalc/CMakeLists.txt;65;add_test;/vercel/sandbox/UltraCalc/CMakeLists.txt;0;")
