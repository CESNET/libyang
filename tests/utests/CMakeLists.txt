set(local_tests
    utest:test_common
    utest:test_set
    utest:test_hash_table
    utest:test_inout
    utest:test_context
    utest:test_xml
    utest:test_xpath
    utest:test_yanglib
    utest:schema/test_schema
    utest:schema/test_parser_yang
    utest:schema/test_parser_yin
    utest:schema/test_tree_schema_compile
    utest:schema/test_printer_yang
    utest:schema/test_printer_yin
    utest:data/test_tree_data
    utest:data/test_new
    utest:data/test_parser_xml
    utest:data/test_printer_xml
    utest:data/test_validation
    utest:data/test_types
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
    " "
    " "
    " "
    " ")
set(tests ${tests} ${local_tests} PARENT_SCOPE)
set(tests_wraps ${tests_wraps} ${local_tests_wraps} PARENT_SCOPE)
