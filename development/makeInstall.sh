#!/bin/bash

source_directory=$(dirname ${BASH_SOURCE[0]})

source "${source_directory}/setupEnv.sh"
echo "Running Make install"

make install -j $1
