#!/bin/bash

LSF_PROJECT=cldrn
PLATFORM=$1
shift

PROCS=$1

#
# Format for PROCS
#  - "1", "2", ...  => the number of slots
#  - "x1", "x2", ... => a number of slots on a exclusively allocated nodes
if echo "$PROCS" | grep -q '^x[[:digit:]]\+$' ;  then
  P=`echo $PROCS | sed -e 's/^x\([[:digit:]]\+\)$/\1/'`
  LSF_PROCS="-x -n $P"
  shift
elif echo "$PROCS" | grep -q '^[[:digit:]]\+$' ; then
  LSF_PROCS="-n $PROCS" 
  shift
else
  LSF_PROCS=""
fi

# Source LSF command line tools
source /glb/apps/hpc/lsfprod/conf/profile.lsf

# Clean-up after exiting
trash=
function onExit()
{
   rm -rf $trash
}
trap onExit EXIT


# Execute on available platforms
exit_status=0
case $PLATFORM in
  RHEL6.8) 
      # Submit command to cluster
      bsub -P $LSF_PROJECT -x -Is -q default.q -R "select[ostype=$PLATFORM]" $LSF_PROCS "$@" 
      exit_status=$?

      # Print that we ended the job
      echo "<<Exited from Job>>"

      ;;

  RHEL5.10)
      # Submit command to TAW (Thin Any Where) cluster
      bsub -P $LSF_PROJECT -Is -q taw.q -R "select[ostype=RHEL5.10]" $LSF_PROCS "$@" 
      exit_status=$?

      # Print that we ended the job
      echo "<<Exited from Job>>"

      ;;

  RHEL6.6)
      # Execute on local machine
      if [ x`/apps/sss/share/getos2 --ver` != x66WS ]; then
         echo Cannot find a RHEL6.6 machine
         exit 1
      fi
      
      # Set-up TMPDIR so that it points to scratch
      tmpdir=/scratch/do-on-platform.$$
      trash="$tmpdir $trash"
      mkdir -p $tmpdir

      echo "<<Running job locally>>"

      # Execute the command
      TMPDIR=$tmpdir "$@"
      exit_status=$?

      # Print that we ended the job
      echo "<<Exited from Job>>"

      # Remove temporary files
      rm -rf $tmpdir

      ;;

  *)
      echo "Platform $PLATFORM is not available"
      exit_status=1
      ;;
esac

exit $exit_status
