#!/bin/bash
src=${SRC_DIR:-`dirname $0`/../../../..}
build=${BUILD_DIR:-`mktemp -d`}
installdir=${INSTALL_DIR:-${build}}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
tfs_version=${TFS_VERSION:-YYYY}
simulator_options=${SIMULATOR_OPTIONS:--itcoupled}
nprocs=${PROCESSORS:-1}
tabledir=${TABLE_DIR:-.}
tablefile=${TABLE_FILE:-table}


# Make temporary files for scripts
selection=`mktemp`
plot=`mktemp`

# Set some code to execute when shell script exits
function onExit()
{
  # remove the temporary directory made for the build directory
  if [ "x${BUILD_DIR}" = x ]; then
     rm -Rf $build $selection $plot
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


# Define project
project=Barrowlands

# Fastcauldron is already built and available in the build direction.
current_time=$(date +'%Y-%m-%d %H:%M:%S')

# Build Cauldron
build_success=ok
GEOCASE=False DEPLOY=False BUILD_DIR=${build} ${src}/development/BuildProjects/Linux-BuildAndInstall.sh \
   || { echo "warning: Build has not completed succesfully"; build_success=failing; }

# Generate the projects first
echo ">>> Generating benchmark projects <<<"
cp -R ${src}/development/applications/hpc/intel_benchmark_projects/$project ${build}
pushd ${build}/$project > /dev/null
  bash generate.sh
popd  > /dev/null

# Now run fastcauldron on a selection of the generated projects
pushd ${build}
source envsetup.sh
for size in micro small
do
   for difficulty in easy medium hard salt
   do
      pushd $project/$size-$difficulty > /dev/null
      logfile=output.log
      export TIMEFORMAT="%R"
      echo ">>> Running fastcauldron $simulator_options on $project of size $size and difficulty $difficulty <<<"
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
        echo -ne "${project}\t"
        echo -ne "${size}\t"
        echo -ne "${difficulty}\t"
        echo -ne "${T_sim}\t"
        echo -ne "${T_assem}\t"
        echo -ne "${T_solve}\t"
        echo -ne "${T_other}\t"
        echo -ne "\n"
      ) >>  ${tabledir}/${tablefile}

      # Extract data from the graph
      sed -ne "/ok\\t${platform}\\t${configuration}\\t${nprocs}\\t${project}\\t${size}\\t${difficulty}/p" < ${tabledir}/${tablefile} > $selection
       
      # Write the Gnuplot script
      Xaxis=2
cat > $plot <<EOF
set terminal png
set output "${installdir}/perfhist-$project-$size-$difficulty-on-$platform-$configuration-$nprocs.png"
set datafile separator "\t"
set xlabel "TFS Changeset"
set ylabel "Running time (seconds)"
set yrange [0:]
set title "The $size sized $difficulty difficulty project $project simulated\n with $simulator_options on $platform built as $configuration on $nprocs processors" 
plot "$selection" using ${Xaxis}:10 title "Total time" with linespoints,\
     "$selection" using ${Xaxis}:11 title "System assembly time" with linespoints,\
     "$selection" using ${Xaxis}:12 title "System solve time" with linespoints,\
     "$selection" using ${Xaxis}:13 title "Other" with linespoints
EOF

      gnuplot < $plot

      popd > /dev/null
   done
done



popd

