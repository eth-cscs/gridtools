macro(detect_cuda)
    if (NOT DEFINED GT_ENABLE_BACKEND_CUDA)
        # detect CUDA support
        include(CheckLanguage)
        check_language (CUDA)

        if (CMAKE_CUDA_COMPILER)
            enable_language (CUDA)
            message (STATUS "CUDA found")
            set (CUDA_AVAILABLE ON)
        else ()
            message (STATUS "CUDA NOT found")
            set (CUDA_AVAILABLE OFF)
        endif ()
    elseif (GT_ENABLE_BACKEND_CUDA)
        if (NOT GT_USE_HIP)
            enable_language (CUDA)
        endif ()
        message (STATUS "CUDA found")
        set (CUDA_AVAILABLE ON)
    else ()
        message (STATUS "CUDA NOT found")
        set (CUDA_AVAILABLE OFF)
    endif ()

    if (CUDA_AVAILABLE AND NOT GT_USE_HIP)
        include(workaround_cudart)
        _fix_cudart_library()
    endif()
endmacro(detect_cuda)

macro(detect_openmp)
    if (NOT DEFINED GT_ENABLE_BACKEND_X86 AND NOT DEFINED GT_ENABLE_BACKEND_MC AND NOT DEFINED GT_ENABLE_BACKEND_NAIVE)
        if (GT_USE_CLANG_CUDA)
            # Disable OpenMP due to bug https://bugs.llvm.org/show_bug.cgi?id=42061
            set (OPENMP_AVAILABLE OFF)
            message (STATUS "OpenMP not avaiable with Clang CUDA")
        else ()
            find_package( OpenMP COMPONENTS CXX)
            if (TARGET OpenMP::OpenMP_CXX)
                set (OPENMP_AVAILABLE ON)
                message (STATUS "OpenMP found")
            else ()
                set (OPENMP_AVAILABLE OFF)
                message (STATUS "OpenMP NOT found")
            endif ()
        endif ()

    elseif (GT_ENABLE_BACKEND_X86 OR GT_ENABLE_BACKEND_MC OR GT_ENABLE_BACKEND_NAIVE)
        if (GT_USE_CLANG_CUDA)
            message (FATAL_ERROR "OpenMP backends can not be used in combination with Clang CUDA due to bug https://bugs.llvm.org/show_bug.cgi?id=42061")
        endif ()
        find_package( OpenMP REQUIRED COMPONENTS CXX)
        set (OPENMP_AVAILABLE ON)
        message (STATUS "OpenMP found")

    else ()
        set (OPENMP_AVAILABLE OFF)
        message (STATUS "OpenMP NOT found")
    endif ()
endmacro(detect_openmp)

macro(detect_mpi)
    if (NOT DEFINED GT_USE_MPI)
        find_package(MPI COMPONENTS CXX)
        include(workaround_mpi)
        _fix_mpi_flags()
        if (MPI_FOUND)
            message (STATUS "MPI found")
            set (MPI_AVAILABLE ON)
        else ()
            message (STATUS "MPI NOT found")
            set (MPI_AVAILABLE OFF)
        endif()
    elseif (GT_USE_MPI)
        find_package(MPI REQUIRED COMPONENTS CXX)
        include(workaround_mpi)
        _fix_mpi_flags()
        message (STATUS "MPI found")
        set (MPI_AVAILABLE ON)
    else ()
        message (STATUS "MPI NOT found")
        set (MPI_AVAILABLE OFF)
    endif()

    if (TARGET MPI::MPI_CXX)
        include(workaround_mpiexec)
        _fix_mpi_exec()
    endif()
endmacro(detect_mpi)
