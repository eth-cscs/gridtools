#!/bin/sh

source tave.sh

module load PrgEnv-intel
module load gcc

export CXX=$(which icpc)
export CC=$(which icc)
export FC=$(which ifort)

export KMP_AFFINITY='balanced'
