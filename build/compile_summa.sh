#!/bin/bash

#### load modules if using Compute Canada or Copernicus ####
module load gcc/9.3.0
module load netcdf-fortran
module load openblas
module load caf

#### Specifiy Master Directory, parent of build directory
export ROOT_DIR=/globalhome/kck540/HPC/SummaProjects/Summa-Actors

#### Specifiy Compilers ####
export FC=gfortran
export CC=g++

#### Includes and Libraries ####

export INCLUDES="-I$EBROOTNETCDFMINFORTRAN/include"
export LIBRARIES="-L$EBROOTNETCDFMINFORTRAN/lib64\
    -L$EBROOTOPENBLAS/lib\
    -lnetcdff -lopenblas"

# INCLUDES FOR Actors Component
export ACTORS_INCLUDES="-I$EBROOTCAF/include\
    -I$EBROOTNETCDFMINFORTRAN/include"

export ACTORS_LIBRARIES="-L$EBROOTCAF/lib\
    -L$EBROOTCAF/lib64\
    -L$EBROOTNETCDFMINFORTRAN/lib64\
    -L$EBROOTOPENBLAS/lib\
    -L$ROOT_DIR/bin\
    -lcaf_core -lcaf_io -lsumma -lopenblas -lnetcdff"

#### Compile with the Makefile ####
make -f ${ROOT_DIR}/build/makefile all


export LD_LIBRARY_PATH=${ROOT_DIR}/bin