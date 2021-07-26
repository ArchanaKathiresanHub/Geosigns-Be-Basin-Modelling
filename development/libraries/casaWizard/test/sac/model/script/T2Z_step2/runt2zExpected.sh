#!/bin/bash -lx
cat > runt2zCluster.sh << EOF
#BSUB -P cldrn
#BSUB -We 3:00
#BSUB -J "Fastcauldron T2Z conversion run"
#BSUB -n 1
#BSUB -o output.log
#BSUB -cwd ./T2Z_step2
source setupEnv.sh
mpirun_wrap.sh -n 1 -outfile-pattern 'fastdepthconversion-output-rank-%r.log' fastdepthconversion -project Project.project3d -temperature -onlyat 0 -referenceSurface 0 -endSurface 10 -noofpp -preserveErosion -noCalculatedTWToutput
EOF

bsub -Is < runt2zCluster.sh
