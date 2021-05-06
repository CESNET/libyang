include(CheckSymbolExists)
include(TestBigEndian)

# defines "compatsrc" with source(s) of this small library
macro(USE_COMPAT)
    # compatibility checks
    set(CMAKE_REQUIRED_DEFINITIONS -D_POSIX_C_SOURCE=200809L)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D__BSD_VISIBLE=1)
    check_symbol_exists(vdprintf "stdio.h;stdarg.h" HAVE_VDPRINTF)
    check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
    check_symbol_exists(vasprintf "stdio.h" HAVE_VASPRINTF)

    check_symbol_exists(strndup "string.h" HAVE_STRNDUP)
    check_symbol_exists(strnstr "string.h" HAVE_STRNSTR)
    check_symbol_exists(getline "stdio.h" HAVE_GETLINE)

    check_symbol_exists(get_current_dir_name "unistd.h" HAVE_GET_CURRENT_DIR_NAME)

    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

    # header and source file (adding the source directly allows for hiding its symbols)
    configure_file(${PROJECT_SOURCE_DIR}/compat/compat.h.in ${PROJECT_BINARY_DIR}/compat/compat.h @ONLY)
    include_directories(${PROJECT_BINARY_DIR}/compat)
    set(compatsrc ${PROJECT_SOURCE_DIR}/compat/compat.c)
endmacro()
