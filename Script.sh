#!/bin/bash
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=24:00:00
#SBATCH --mem=0
#SBATCH --job-name=SA-Sun-Day
#SBATCH --output=/globalhome/cvb652/HPC/Output/-%A.out
#SBATCH --account=hpc_c_giws_spiteri

module load StdEnv/2023
module load gcc/12.3
module load netcdf-fortran
module load tbb
module load openmpi

summa_exe=/globalhome/cvb652/HPC/Summa-Actors/bin/summa_actors.exe
config_summa=/globalhome/cvb652/HPC/Summa-Actors/bin/config.json
$summa_exe -g 1 500000 -c $config_summa 