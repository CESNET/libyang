set(local_tests
    src_set
    src_common
    src_context
    src_hash_table
    src_xml
    src_parser_yang
    src_tree_schema_compile)
set(local_tests_wraps
    " "
    "-Wl,--wrap=realloc"
    "-Wl,--wrap=ly_set_add"
    " "
    " "
    " "
    " ")
set(tests ${tests} ${local_tests} PARENT_SCOPE)
set(tests_wraps ${tests_wraps} ${local_tests_wraps} PARENT_SCOPE)
