# Sharp C language support for CMake
#
# Include this module after project() and after defining targets.
# Usage:
#   include(cmake/SharpC.cmake)
#   add_executable(myapp main.c util.ce)
#   sharp_enable_c(myapp)
#
# This registers all .ce files in the target as C source files.

function(sharp_enable_c _target)
    get_target_property(_srcs ${_target} SOURCES)
    if(NOT _srcs)
        return()
    endif()
    foreach(_src IN LISTS _srcs)
        get_filename_component(_ext "${_src}" LAST_EXT)
        if(_ext STREQUAL ".ce")
            set_source_files_properties("${_src}" PROPERTIES LANGUAGE C)
        endif()
    endforeach()
endfunction()