set(local_tests
    src_set
    src_common
    src_context
    src_hash_table
    src_xml
    src_parser_yang
    src_tree_schema
    src_tree_schema_compile
    src_tree_schema_helpers
    src_printer_yang
    src_tree_data
    src_parser_xml
    src_printer_xml)
set(local_tests_wraps
    " "
    "-Wl,--wrap=realloc"
    " "
    " "
    " "
    " "
    " "
    " "
    " "
    " "
    " "
    " "
    " ")
set(tests ${tests} ${local_tests} PARENT_SCOPE)
set(tests_wraps ${tests_wraps} ${local_tests_wraps} PARENT_SCOPE)
