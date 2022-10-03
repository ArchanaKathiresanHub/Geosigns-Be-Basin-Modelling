#!/bin/bash -lx
export PATH=:$PATH
source setupEnv.sh
sbatch "./T2Z_step2/run.sh"
