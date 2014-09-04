#!/bin/bash

LSF_PROJECT=cldrn
PLATFORM=$1
shift

# Source LSF command line tools
source /glb/apps/hpc/lsfprod/conf/profile.lsf


case $PLATFORM in
  RHEL6*) 
      # Submit command to cluster
      bsub -P $LSF_PROJECT -Is -q default.q -R "select[ostype=$PLATFORM]" "$@" 

      # Print that we ended the job
      echo "<<Exited from Job>>"

      ;;

  RHEL5.10)
      # Submit command to TAW (Thin Any Where) cluster
      bsub -P $LSF_PROJECT -Is -q taw.q -R "select[ostype=RHEL5.10]" "$@" 

      # Print that we ended the job
      echo "<<Exited from Job>>"

      ;;

  RHEL5.8)
      # Execute on local machine
      if [ x`/apps/sss/share/getos2 --ver` != x58WS ]; then
         echo Cannot find a RHEL5.8 machine
         exit 1
      fi
      
      echo "<<Running job locally>>"

      # Execute the command
      "$@"

      # Print that we ended the job
      echo "<<Exited from Job>>"

      ;;

  *)
      echo "Platform $PLATFORM is not available"
      ;;
esac

