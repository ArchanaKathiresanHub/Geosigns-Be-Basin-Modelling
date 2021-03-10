#!/bin/bash -lx
#BSUB -P cldrn
#BSUB -W 0:30
#BSUB -J "Fastcauldron T2Z conversion run"
#BSUB -n 1
#BSUB -o output.log
#BSUB -x
#BSUB -cwd .T2Z_step2
mpirun_wrap.sh -n 1 -outfile-pattern 'fastdepthconversion-output-rank-%r.log' fastdepthconversion -project Project.project3d -temperature -onlyat 0 -referenceSurface 0 -endSurface 6 -noofpp
