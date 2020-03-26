function(gridtools_enable_fortran_openacc_on_target target)
    if(CMAKE_Fortran_COMPILER_ID STREQUAL "Cray")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:Fortran>:-h acc>)
    elseif(CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:Fortran>:-fopenacc>)
        set_target_properties(${target} PROPERTIES APPEND_STRING PROPERTY LINK_FLAGS -fopenacc)
    else()
        message(FATAL_ERROR "OpenACC is not configured for this compiler.")
    endif()
endfunction()

function(gridtools_enable_fortran_preprocessing_on_target target)
    if (CMAKE_Fortran_COMPILER_ID STREQUAL "Cray")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:Fortran>:-eF>)
    else()
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:Fortran>:-cpp>)
    endif()
endfunction()

# DEPRECATED: remove in GT 3.0 or after the COSMO dycore is updated with a GT 2.0 prerelease
function(gt_enable_fortran_openacc_on_target target)
    message(WARNING "gt_enable_fortran_openacc_on_target() is deprecated, use gridtools_enable_fortran_openacc_on_target()")
    gridtools_enable_fortran_openacc_on_target(${target})
endfunction()

# DEPRECATED: remove in GT 3.0 or after the COSMO dycore is updated with a GT 2.0 prerelease
function(gt_enable_fortran_preprocessing_on_target target)
    message(WARNING "gt_enable_fortran_preprocessing_on_target() is deprecated, use gridtools_enable_fortran_preprocessing_on_target()")
    gridtools_enable_fortran_preprocessing_on_target(${target})
endfunction()
