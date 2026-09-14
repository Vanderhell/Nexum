# CMake generated Testfile for 
# Source directory: C:/Users/vande/Desktop/NETWORK
# Build directory: C:/Users/vande/Desktop/NETWORK/build-fuzz
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(primitive "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_primitive.exe")
set_tests_properties(primitive PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(graph "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_graph.exe")
set_tests_properties(graph PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(topology "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_topology.exe")
set_tests_properties(topology PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(determinism "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_determinism.exe")
set_tests_properties(determinism PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(expressivity "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_expressivity.exe")
set_tests_properties(expressivity PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(expressivity_regression "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_expressivity_regression.exe")
set_tests_properties(expressivity_regression PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(fsm "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_fsm.exe")
set_tests_properties(fsm PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(generated "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_generated.exe")
set_tests_properties(generated PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(serialization "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_serialization.exe")
set_tests_properties(serialization PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(analysis "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_analysis.exe")
set_tests_properties(analysis PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(retry "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_retry.exe")
set_tests_properties(retry PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
add_test(freeze "C:/Users/vande/Desktop/NETWORK/build-fuzz/test_freeze.exe")
set_tests_properties(freeze PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;25;add_test;C:/Users/vande/Desktop/NETWORK/CMakeLists.txt;0;")
