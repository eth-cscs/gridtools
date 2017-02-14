cmake_minimum_required(VERSION 2.8)

set( SUPPRESS_MESSAGES "ON" CACHE BOOL "suppress compilation messages")
set( VERBOSE "OFF" CACHE BOOL "set verbosity for output")
set( BOOST_FUSION_MAX_SIZE 40 CACHE STRING "max sizes of boost fusion containers" )
set( ENABLE_PERFORMANCE_METERS "OFF" CACHE BOOL "If on, meters will be reported for each stencil")
set( ENABLE_CXX11 "ON" CACHE BOOL "Enable examples and tests featuring C++11 features" )
set( ENABLE_PYTHON "OFF" CACHE BOOL "Enable Python front-end and tests. Requires Python >=3.0" )
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
set( ENABLE_CACHING "ON" CACHE BOOL "Enable caching functionality" )
set( NVCC_CLANG_SPECIFIC_OPTIONS "" CACHE STRING "Options passed to NVCC when compiling with clang as host compiler" )
set( WERROR "OFF" CACHE BOOL "Treat warnings as errors" )
