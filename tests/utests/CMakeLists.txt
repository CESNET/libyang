set(local_tests
    utest:test_common
    utest:test_set
    utest:test_hash_table
    utest:test_context
    utest:test_xml
    utest:test_parser_yang
    utest:test_parser_yin
    utest:test_tree_schema
    utest:test_tree_schema_compile
    utest:test_tree_schema_helpers
    utest:test_printer_yang
    utest:test_printer_yin
    utest:test_tree_data
    utest:test_parser_xml
    utest:test_printer_xml
    utest:test_validation
    utest:test_types
    utest:extensions/test_metadata
    utest:extensions/test_nacm)
set(local_tests_wraps
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
