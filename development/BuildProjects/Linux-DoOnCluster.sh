#!/bin/bash

LSF_PROJECT=cldrn

# Source LSF command line tools
source /glb/apps/hpc/lsfprod/conf/profile.lsf

# Submit command to cluster
bsub -P $LSF_PROJECT -Is "$@"

# Print that we ended the job
echo "<<Exited from Job>>"
