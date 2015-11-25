#!/bin/bash 
#SBATCH -p debug
#SBATCH -N 3
#SBATCH -t 00:15:00

date
scratch=$SCRATCH/$SLURM_JOB_ID
mkdir -p $scratch
cp -v /project/projectdirs/cosmo/staging/decam-public/2014-05-27/*.fits.fz $scratch/.
date

srun -n 70 ./bb-cfitsio-read bb-cfitsio-read-from-sc $scratch/*.fits.fz
