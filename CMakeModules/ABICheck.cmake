cmake_minimum_required(VERSION 2.8.12)

# generate API/ABI report
macro(LIBYANG_ABICHECK)
    find_program(ABI_DUMPER abi-dumper)
    find_package_handle_standard_args(abi-dumper DEFAULT_MSG ABI_DUMPER)
    if(ABI_DUMPER)
        set(PUBLIC_HEADERS ${headers})
        string(PREPEND PUBLIC_HEADERS "${CMAKE_SOURCE_DIR}/")
        string(REPLACE ";" "\n${CMAKE_SOURCE_DIR}/" PUBLIC_HEADERS "${PUBLIC_HEADERS}")
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/public_headers CONTENT "${PUBLIC_HEADERS}")
        add_custom_target(abi-dump
                COMMAND ${ABI_DUMPER} ./libyang${CMAKE_SHARED_LIBRARY_SUFFIX} -o libyang.${LIBYANG_SOVERSION_FULL}.dump
                -lver ${LIBYANG_SOVERSION_FULL} -public-headers ${CMAKE_BINARY_DIR}/public_headers
                DEPENDS yang
                BYPRODUCTS ${CMAKE_BINARY_DIR}/libyang.${LIBYANG_SOVERSION_FULL}.dump
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT Dumping ABI information for abi-check.)
    endif()

    # check correctness of the SO version according to the API/ABI changes
    find_program(ABI_CHECKER abi-compliance-checker)
    find_package_handle_standard_args(abi-compliance-checker DEFAULT_MSG ABI_CHECKER)
    if(ABI_DUMPER AND ABI_CHECKER)
        set(ABIBASE_HASH "" CACHE STRING "GIT hash for the commit to compare current ABI to. If not set, base SO version commit is found.")
        if (NOT ABIBASE_HASH)
            # check that we have some ABI base version already and get its hash in GIT
            set(ABIBASE_VERSION "${LIBYANG_MAJOR_VERSION}.0.0")
            execute_process(COMMAND bash "-c" "git log --pretty=oneline --grep=\"SOVERSION .* ${ABIBASE_VERSION}\$\" | cut -d' ' -f1"
                    OUTPUT_VARIABLE ABIBASE_HASH)
        else()
            string(SUBSTRING ${ABIBASE_HASH} 0 8 ABIBASE_HASH_SHORT)
            set(ABIBASE_VERSION "git-${ABIBASE_HASH_SHORT}")
    endif()
        if(ABIBASE_HASH)
            # we have the ABI base version, so generate script for abi-check target
            string(REPLACE "\n" "" ABIBASE_HASH ${ABIBASE_HASH})
            file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/abibase.sh CONTENT "#!/bin/sh
if [ ! -d abibase ]; then mkdir abibase; fi
cd abibase
if [ ! -f build/libyang.*.dump ]; then
    if [ -d .git ] && [ \"${ABIBASE_HASH}\" != \"`git log --pretty=oneline | cut -d' ' -f1`\" ]; then rm -rf .* 2> /dev/null; fi
    if [ ! -d .git ]; then
        git init
        git remote add origin https://github.com/CESNET/libyang
        git fetch origin --depth 1 ${ABIBASE_HASH}
        git reset --hard FETCH_HEAD
    fi
    if [ ! -d build ]; then mkdir build; fi
    cd build
    cmake -DCMAKE_BUILD_TYPE=ABICheck ..
    make abi-dump
fi
")
            # abi-check target itself using abi-compliance-checker
            add_custom_target(abi-check
                    COMMAND bash ./abibase.sh
                    COMMAND ${ABI_CHECKER} -l libyang${CMAKE_SHARED_LIBRARY_SUFFIX} -old abibase/build/libyang.*.dump
                    -new ./libyang.${LIBYANG_SOVERSION_FULL}.dump -s
                    DEPENDS yang abi-dump
                    BYPRODUCTS ${CMAKE_BINARY_DIR}/compat_reports/libyang${CMAKE_SHARED_LIBRARY_SUFFIX}/*_to_${LIBYANG_SOVERSION_FULL}/compat_report.html
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT Checking ABI compatibility with the ${ABIBASE_VERSION} version.)
        else()
            add_custom_target(abi-check
                    COMMENT Nothing to check - missing base SOVERSION commit for ${ABIBASE_VERSION} version.)
        endif()
    endif()
endmacro()
