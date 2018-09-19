set(local_tests
    src_set
    src_common)
set(local_tests_wraps
    " "
    "-Wl,--wrap=realloc")
set(tests ${tests} ${local_tests} PARENT_SCOPE)
set(tests_wraps ${tests_wraps} ${local_tests_wraps} PARENT_SCOPE)
