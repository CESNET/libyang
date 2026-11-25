set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/perf.c
    PARENT_SCOPE)

add_executable(ly_perf ${CMAKE_CURRENT_SOURCE_DIR}/perf.c)
set_target_properties(ly_perf PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
target_link_libraries(ly_perf yang)
