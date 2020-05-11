<a href="https://GridTools.github.io/gridtools"><img src="docs/_static/logo.svg"/></a>
<br/><br/>
<a target="_blank" href="https://opensource.org/licenses/BSD-3-Clause">![License: BSD][BSD.License]</a>
![](https://github.com/GridTools/gridtools/workflows/CI/badge.svg?branch=master)
![](https://github.com/GridTools/gridtools/workflows/CMake-config/badge.svg?branch=master)
<a target="_blank" href="https://gridtools-slack.herokuapp.com"><img src="https://gridtools-slack.herokuapp.com/badge.svg"></a>
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/GridTools/gridtools) 

The GridTools framework is a set of libraries and utilities to develop performance portable applications in the area of weather and climate. To achieve the goal of performance portability, the user-code is written in a generic form which is then optimized for a given architecture at compile-time. The core of GridTools is the stencil composition module which implements a DSL embedded in C++ for stencils and stencil-like patterns. Further, GridTools provides modules for halo exchanges, boundary conditions, data management and bindings to C and Fortran.

GridTools is successfully used to accelerate the dynamical core of the [COSMO model](http://cosmo-model.org/) with improved performance on CUDA-GPUs compared to the current official version, demonstrating production quality and feature-completeness of the library for models on lat-lon grids.

Although GridTools was developed for weather and climate applications it might be applicable for other domains with a focus on stencil-like computations.

A detailed introduction can be found in the [documentation](https://GridTools.github.io/gridtools).

### Installation instructions

```
git clone https://github.com/GridTools/gridtools.git
cd gridtools
mkdir -p build && cd build
cmake ..
make -j8
make test
```

For choosing the compiler, use the standard CMake techniques, e.g. setting the environment variables
```
CXX=`which g++` # full path to the C++ compiler
CC=`which gcc` # full path to theC compiler
FC=`which gfortran` # full path to theFortran compiler
CUDACXX=`which nvcc` # full path to NVCC
CUDAHOSTCXX=`which g++` # full path to the C++ compiler to be used as CUDA host compiler
```

##### Requirements

- Boost (1.65.1 or later)
- CMake (3.14.5 or later)
- CUDA Toolkit (9.0 or later, optional)
- MPI (optional, CUDA-aware MPI for the GPU communication module `gcl_gpu`)

### Supported compilers

The GridTools libraries are currently nightly tested with the following compilers on [CSCS supercomputers](https://www.cscs.ch/computers/overview/).

| Compiler | Backend | Tested on | Comments
| --- | --- | --- | --- |
| Cray clang version 9.0.2 | all backends | Piz Daint | with flags `-fno-cray-gpu -fno-cray`, P100 GPU
| GNU 7.3.0 + NVCC 10.1 | all backends | Piz Daint | P100 GPU |
| Clang 7.0.1 + NVCC 10.1 | all backends | Piz Daint | GPU compilation in NVCC-CUDA mode, P100 GPU |
| GNU 8.3.0 + NVCC 10.1 | all backends | Tsa | V100 GPU |
| HIP Clang (TODO) | all backends | Ault | TODO GPU |

##### Officially not supported (no workarounds implemented and planned)

| Compiler | Backend | Date | Comments
| --- | --- | --- | --- |
| Cray without Clang frontend| cpu_kfirst |  | no effort to fix compilation
| NVCC <= 9.1 with GNU 6.x | gpu | 2018-10-16 | similar to [this tuple bug](https://devtalk.nvidia.com/default/topic/1028112/cuda-setup-and-installation/nvcc-bug-related-to-gcc-6-lt-tuple-gt-header-/)
| PGI 18.5 | cpu_kfirst | 2018-12-06 | no effort to fix compilation
| Intel 19.0.1.144 | all backends | 2020-05-11 | Intel workarounds removed in GridTools 2.0 (goal would be to support Intel with `-gnextgen`)
| Intel 19.1.0.166 | all backends | 2020-05-11 | even with `-qnextgen`, no effort to fix compilation

### Contributing

Contributions to the GridTools framework are welcome. Please open an issue for any bugs that you encounter or provide a fix or enhancement as a PR. External contributions to GridTools require us a signed copy of a copyright release form to ETH Zurich. We will contact you on the PR.

[BSD.License]: https://img.shields.io/badge/License-BSD--3--Clause-blue.svg
