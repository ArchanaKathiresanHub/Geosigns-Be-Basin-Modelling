#!/bin/bash -lx
export PATH=:$PATH
source setupEnv.sh
bsub <"./T2Z_step2/run.sh"
