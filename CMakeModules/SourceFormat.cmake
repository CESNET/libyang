# format source files with uncrustify
# files are expected to be a list and relative paths are resolved wtih respect to CMAKE_SOURCE DIR
macro(SOURCE_FORMAT)
    if(NOT ${ARGC})
        message(FATAL_ERROR "source_format() needs a list of files to format!")
    endif()

    find_package(Uncrustify 0.71)
    if(UNCRUSTIFY_FOUND)
        add_custom_target(format
                COMMAND ${UNCRUSTIFY} -c ${CMAKE_SOURCE_DIR}/uncrustify.cfg --no-backup --replace ${ARGN}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Formating sources with ${UNCRUSTIFY} ...")

        add_custom_target(format-check
                COMMAND ${UNCRUSTIFY} -c ${CMAKE_SOURCE_DIR}/uncrustify.cfg --check ${ARGN}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Checking format of the sources with ${UNCRUSTIFY} ...")
    endif()
endmacro()
