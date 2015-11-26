#!/bin/bash 
#SBATCH -p debug
#SBATCH -N 3
#SBATCH -t 00:15:00
#DW jobdw capacity=100GB access_mode=striped type=scratch

date
cp -v /project/projectdirs/cosmo/staging/decam-public/2014-05-27/*.fits.fz $DW_JOB_STRIPED/.
date

srun -n 70 ./parallel-cfitsio-read from-burst-buffer $DW_JOB_STRIPED/*.fits.fz
