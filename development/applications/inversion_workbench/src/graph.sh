#!/bin/bash

#defaults
INPUT_PROJECT=Project.project3d
OUTPUT_TABLE=table
PARAMETER=TopCrustHeatProd
VARIABLE=Temperature
X=500
Y=500
DEPTHS="0 100 200 300"
VALUES="0.0 0.5 1.0 1.5 2.0 2.5 3.0 3.5"
CAULDRON_MODE="temperature"
NPROCS=`grep processor /proc/cpuinfo | wc -l`

# Parsing command line parameters
while [ $# -gt 1 ]; 
do
  case $1 in
    -project) 
    	INPUT_PROJECT="$2"
	shift
	shift
	;;

    -table)
        OUTPUT_TABLE="$2"
	shift
	shift
	;;

    -parameter)
        PARAMETER="$2"
	shift
	shift
	;;

    -variable)
        VARIABLE="$2"
	shift
	shift
	;;

    -x)
        X="$2"
	shift
	shift
	;;

    -y)
    	Y="$2"
	shift
	shift
	;;

    -depths)
        DEPTHS="$2"
	shift
	shift
	;;

    -values)
    	VALUES="$2"
	shift
	shift
	;;

    -mode)
    	CAULDRON_MODE=$2
	shift
	shift
	;;

    -nprocs)
        NPROCS=$2
	shift
	shift
	;;

    *)
        echo Unrecognized option: $1
	exit 1
	;;

  esac
done

if [ -e $OUTPUT_TABLE ]; then
  echo The destination file $OUTPUT_TABLE already exists. 
  exit 1;
fi

# Print variable settings
echo Input project file: $INPUT_PROJECT
echo Output table file: $OUTPUT_TABLE
echo Varying parameter \'$PARAMETER\' with values $VALUES
echo Measuring \'$VARIABLE\' at location X: $X, Y: $Y, Zs: $DEPTHS
echo Cauldron mode: $CAULDRON_MODE
echo Number of available processors: $NPROCS


# Check environment
if [ x$INVERSION_ROOT = x ]; then
  echo Environment has not been set-up correctly. Please source the 
  echo setenv.sh file in the root of the inversion project directory first.
  exit 1;
fi

CAULDRON=$CAULDRON_DEV/applications/fastcauldron/obj.Linux64/fastcauldron
PROBE=$INVERSION_ROOT/src/c++/inversion/probe
ADJUST=$INVERSION_ROOT/src/c++/inversion/adjust

if [ ! -e $PROBE ]; then
  echo The executable $PROBE does not exist. Please build it first:
  echo do a \'make\' in the $(dirname $PROBE) directory
  exit 1
fi

if [ ! -e $ADJUST ]; then
  echo The executable $ADJUST does not exist. Please build it first:
  echo do a \'make\' in the $(dirname $ADJUST) directory
  exit 1
fi


# Copy files to temporary dir
tmpdir=`mktemp -d` || { echo Could not create temporary directory; exit 1; }
cp $INPUT_PROJECT $tmpdir/Project.project3d


ZS_PARAM=
for DEPTH in $DEPTHS
do
  ZS_PARAM="$ZS_PARAM -z $DEPTH"
done

VECS=
for V in $VALUES
do
  VECS="$VECS $V.vec"
done

# This make file below is going to help parallel processing 
cat > $tmpdir/Makefile <<EOF

%.project3d : Project.project3d
	cp Project.project3d \$@
	${ADJUST} --project \$@ --set-basement-property ${PARAMETER}=\$*

%.vec : %.project3d
	${CAULDRON} -project \$< -${CAULDRON_MODE} -onlyat 0 > /dev/null
	echo -n \$*" " > \$@
	${PROBE} -project \$< -property ${VARIABLE} -age 0 -x ${X} -y ${Y} ${ZS_PARAM} >> \$@

table : ${VECS}
	echo NAN ${DEPTHS} > table
	cat ${VECS} >> table
EOF

# Execute the makefile 
make -C $tmpdir -j $NPROCS

# Save results
cp $tmpdir/table $OUTPUT_TABLE

# Clean up
echo Cleaning up
rm -Rf $tmpdir

# End
echo DONE
