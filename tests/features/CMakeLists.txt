set(local_tests
    features_types
    features_metadata
    features_nacm)
set(local_tests_wraps
    " "
    " "
    " ")
set(tests ${tests} ${local_tests} PARENT_SCOPE)
set(tests_wraps ${tests_wraps} ${local_tests_wraps} PARENT_SCOPE)
