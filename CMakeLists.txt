cmake_minimum_required (VERSION 3.2)
project (perf_aware)
add_executable(perf_aware main.cc)
set_target_properties(perf_aware PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "../")
