#!/bin/bash

source_directory=$(dirname ${BASH_SOURCE[0]})

source "${source_directory}/setupEnv.sh"
echo "Checking Locale"
locale
echo "Running Make all"
make all -j $1
