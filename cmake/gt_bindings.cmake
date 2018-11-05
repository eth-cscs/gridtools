# Create a library with c- and Fortran-bindings
#
# Usage of this module:
#
#  add_bindings_library(<library-name> SOURCES <sources>[...] [FORTRAN_OUTPUT_DIR fortran_dir] [C_OUTPUT_DIR c_dir] [FORTRAN_MODULE_NAME name])
#
#  Arguments:
#   SOURCES: sources of the library
#   FORTRAN_OUTPUT_DIR: destination for generated Fortran files (default: ${CMAKE_CURRENT_LIST_DIR})
#   C_OUTPUT_DIR: destination for generated C files (default: ${CMAKE_CURRENT_LIST_DIR})
#   FORTRAN_MODULE_NAME: name for the Fortran module (default: <library-name>)
#
# Variables used by this module:
#
#  GT_BINDINGS_CROSS_COMPILATION:
#  If GT_BINDINGS_CROSS_COMPILATION=ON, bindings will not be generated, but expected to be provided,
#  as part of the user source code, e.g. by updating bindings with the bindings generator during development.
#  If GT_BINDINGS_CROSS_COMPILATION is not defined already it will be made available after including this file.
#  Including this module will make the cached CMake variable GT_BINDINGS_CROSS_COMPILATION available.
#
# In the default case (GT_BINDINGS_CROSS_COMPILATION=OFF), the bindings files are generated in the directory
# where the CMakeLists.txt with the call to add_bindings_library() is located.
#
# Targets generated by add_bindings_library(<library-name> ...):
#  - <library_name> library build from <Sources...> without bindings (ususally this target is not used)
#  - <library_name>_declarations will run the generator for this library
#  - <library_name>_c the C-bindings with <library_name> linked to it
#  - <library_name>_fortran the Fortran-bindings with <library_name> linked to it


option(GT_BINDINGS_CROSS_COMPILATION "If turned on, bindings will not be generated." OFF)

if(DEFINED GRIDTOOLS_LIBRARIES_DIR)
    set(bindings_generator_path_to_src ${GRIDTOOLS_LIBRARIES_DIR}/../src)
else()
    set(bindings_generator_path_to_src ${CMAKE_SOURCE_DIR}/src)
endif()

add_library(c_bindings_generator ${bindings_generator_path_to_src}/c_bindings/generator.cpp ${bindings_generator_path_to_src}/c_bindings/generator_main.cpp)

macro(add_bindings_library target_name)
    set(options)
    set(one_value_args FORTRAN_OUTPUT_DIR C_OUTPUT_DIR FORTRAN_MODULE_NAME) # TODO use OUTPUT_DIR!!!
    set(multi_value_args SOURCES)
    cmake_parse_arguments(ARG "${options}" "${one_value_args};" "${multi_value_args}" ${ARGN})

    if(NOT DEFINED ARG_FORTRAN_MODULE_NAME)
        set(ARG_FORTRAN_MODULE_NAME ${target_name}) # default value
    endif()

    add_library(${target_name} ${ARG_SOURCES})
    target_link_libraries(${target_name} ${binding_libs} c_bindings_generator)

    set(bindings_c_decl_filename ${CMAKE_CURRENT_LIST_DIR}/${target_name}.h)
    set(bindings_fortran_decl_filename ${CMAKE_CURRENT_LIST_DIR}/${target_name}.f90)

    if(NOT GT_BINDINGS_CROSS_COMPILATION)
        # generator
        add_executable(${target_name}_decl_generator
            ${bindings_generator_path_to_src}/c_bindings/generator_main.cpp)
        target_link_libraries(${target_name}_decl_generator c_bindings_generator)
        set_target_properties(${target_name}_decl_generator PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${GPU_SPECIFIC_FLAGS}")
    
        if (${APPLE})
            target_link_libraries(${target_name}_decl_generator
                -Wl,-force_load ${target_name})
        else()
            target_link_libraries(${target_name}_decl_generator
                -Xlinker --whole-archive ${target_name}
                -Xlinker --no-whole-archive)
        endif()
    
        add_custom_target(${target_name}_declarations
            COMMAND ${CMAKE_COMMAND}
                -DGENERATOR=${CMAKE_CURRENT_BINARY_DIR}/${target_name}_decl_generator
                -DBINDINGS_C_DECL_FILENAME=${bindings_c_decl_filename}
                -DBINDINGS_FORTRAN_DECL_FILENAME=${bindings_fortran_decl_filename}
                -DFORTRAN_MODULE_NAME=${ARG_FORTRAN_MODULE_NAME}
                -P ${bindings_generator_path_to_src}/../cmake/gt_bindings_generate.cmake #TODO FIXME
            BYPRODUCTS ${bindings_c_decl_filename} ${bindings_fortran_decl_filename}
            DEPENDS $<TARGET_FILE:${target_name}_decl_generator>)
    else()
        if(EXISTS ${bindings_c_decl_filename} AND (EXISTS ${bindings_fortran_decl_filename}))
            add_custom_target(${target_name}_declarations) # noop, the dependencies are satisfied if the files exist
        else()
            message(FATAL_ERROR "Cross-compilation for bindings is enabled: no bindings will be generated, but "
                "${bindings_c_decl_filename} and/or "
                "${bindings_fortran_decl_filename} "
                "are missing. Generate the bindings and consider making them part of your repository.") 
        endif()
    endif()

    # bindings c library
    add_library(${target_name}_c INTERFACE)
    target_link_libraries(${target_name}_c INTERFACE ${target_name})
    add_dependencies(${target_name}_c ${target_name}_declarations)

    # bindings Fortran library
    add_library(${target_name}_fortran ${bindings_fortran_decl_filename})
    target_link_libraries(${target_name}_fortran ${target_name} ${binding_f90_libs})
    add_dependencies(${target_name}_fortran ${target_name}_declarations)
endmacro()

