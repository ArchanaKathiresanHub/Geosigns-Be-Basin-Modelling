#!/bin/bash
src=${SRC_DIR:-`dirname $0`/../../../..}
build=${BUILD_DIR:-`mktemp -d`}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
tfs_version=${TFS_VERSION:-YYYY}
simulator_options=${SIMULATOR_OPTIONS:--itcoupled}
nprocs=${PROCESSORS:-1}
tabledir=${TABLE_DIR:-.}
tablefile=${TABLE_FILE:-table}

# Set some code to execute when shell script exits
function onExit()
{
  # remove the temporary directory made for the build directory
  if [ "x${BUILD_DIR}" = x ]; then
     rm -Rf $build
  fi
}

trap onExit EXIT

pushd ${build} > /dev/null
build=`pwd -P`
popd > /dev/null

pushd ${src} > /dev/null
src=`pwd -P`
popd > /dev/null

pushd ${tabledir} > /dev/null
tabledir=`pwd -P`
popd > /dev/null

# Fastcauldron is already built and available in the build direction.
current_time=$(date +'%Y-%m-%d %H:%M:%S')

# Build Cauldron
build_success=ok
GEOCASE=False Deploy=False BUILD_DIR=${build} ${src}/development/BuildProjects/Linux-BuildAndInstall.sh \
   || { echo "warning: Build has not completed succesfully"; build_success=failing; }

# Generate the projects first
echo ">>> Generating benchmark projects <<<"
cp -R ${src}/development/applications/hpc/intel_benchmark_projects/Barrowlands ${build}
pushd ${build}/Barrowlands > /dev/null
  bash generate.sh
popd  > /dev/null

# Now run fastcauldron on a selection of the generated projects
pushd ${build}
source envsetup.sh
for size in micro small
do
   for difficulty in easy medium hard salt
   do
      pushd Barrowlands/$size-$difficulty > /dev/null
      logfile=output.log
      export TIMEFORMAT="%R"
      echo ">>> Running fastcauldron $simulator_options on Barrowlands of size $size and difficulty $difficulty <<<"
      T_sim=$( ( time mpirun -np $nprocs ${build}/applications/fastcauldron/fastcauldron -project Project.project3d -verbose $simulator_options > $logfile 2>&1 ; ) 2>&1 )

      echo ">>> Adding results to table ${tabledir}/${tablefile} <<<"
      # Change the changeset number into a number (it's of the form C1234)
      version=$(echo $tfs_version | sed -e 's/^C//')

      # Total system assembly time (sec)
      T_assem=$(sed -n -e 's/^ total System_Assembly_Time\s*//p' $logfile | tail -n 1)

      # Total system solve time (sec)
      T_solve=$( sed -n -e 's/^ total System_Solve_Time\s*//p' $logfile | tail -n 1)

      # Total 
      T_other=$( echo "scale=15; $T_sim - $T_assem - $T_solve" | bc )


      ( echo -ne "${current_time}\t"
        echo -ne "${version}\t" 
        echo -ne "${build_success}\t"
        echo -ne "${platform}\t"
        echo -ne "${configuration}\t"
        echo -ne "${nprocs}\t"
        echo -ne "Barrowlands\t"
        echo -ne "${size}\t"
        echo -ne "${difficulty}\t"
        echo -ne "${T_sim}\t"
        echo -ne "${T_assem}\t"
        echo -ne "${T_solve}\t"
        echo -ne "${T_other}\t"
        echo -ne "\n"
      ) >>  ${tabledir}/${tablefile}

      popd > /dev/null
   done
done


popd

