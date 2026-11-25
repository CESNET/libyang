add_test(NAME headers
    COMMAND ${CMAKE_SOURCE_DIR}/compat/check_includes.sh ${CMAKE_SOURCE_DIR}/src/ ${CMAKE_SOURCE_DIR}/tools/lint/ ${CMAKE_SOURCE_DIR}/tools/re/)

if(${SOURCE_FORMAT_ENABLED})
    add_test(NAME format WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND cmake --build ${CMAKE_BINARY_DIR} --target format-check)
endif()

# just compile
add_executable(cpp_compat cpp_compat.c)
target_include_directories(cpp_compat BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cpp_compat yang)
target_compile_options(cpp_compat PUBLIC "-Werror=c++-compat")
