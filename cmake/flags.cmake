cmake_minimum_required(VERSION 2.8)

set( SUPPRESS_MESSAGES "ON" CACHE BOOL "suppress compilation messages")
set( VERBOSE "OFF" CACHE BOOL "set verbosity for output")
set( BOOST_FUSION_MAX_SIZE 20 CACHE STRING "max sizes of boost fusion containers" )
set( ENABLE_PERFORMANCE_METERS "OFF" CACHE BOOL "If on, meters will be reported for each stencil")
set( USE_GPU "OFF" CACHE BOOL "Compile with GPU support (CUDA)" )
set( COMPILE_TO_PTX "OFF" CACHE BOOL "Compile to intermediate representation" )
set( SINGLE_PRECISION OFF CACHE BOOL "Option determining number of bytes used to represent the floating poit types (see defs.hpp for configuration)" )
set( STRUCTURED_GRIDS "ON" CACHE BOOL "compile for rectangular grids" )
set( USE_MPI "OFF" CACHE BOOL "Compile with MPI support" )
set( GCL_MPI "${USE_MPI}" )
set( GCL_GPU "${USE_GPU}" )
set( GCL_ONLY "OFF" CACHE BOOL "If on only library is build but not the examples and tests" )
set( USE_MPI_COMPILER "OFF" CACHE BOOL "On rosa turn this flag off since compiler takes care of mpi already" )
set( HOST_SPECIFIC_OPTIONS "" CACHE STRING "Options passed only to HOST COMPILER and not ACCELERATOR COMPILER" )
set( TEST_SCRIPT ${CMAKE_BINARY_DIR}/run_tests.sh )
set( TEST_MPI_SCRIPT ${CMAKE_BINARY_DIR}/run_mpi_tests.sh )
set( TEST_CUDA_MPI_SCRIPT ${CMAKE_BINARY_DIR}/run_cuda_mpi_tests.sh )
set( ENABLE_CACHING "ON" CACHE BOOL "Enable caching functionality" )
set( NVCC_CLANG_SPECIFIC_OPTIONS "" CACHE STRING "Options passed to NVCC when compiling with clang as host compiler" )
set( WERROR "OFF" CACHE BOOL "Treat warnings as errors" )
set( DISABLE_TESTING "OFF" CACHE BOOL "Disables all unit tests/examples")
