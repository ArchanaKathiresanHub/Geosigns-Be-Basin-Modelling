#!/bin/bash
work_dir=$SCRATCH/hdf5/5D
mkdir -p $work_dir
rm -Rf $work_dir/*

module load phdf5

N="{ 512, 512, 64, 256 }"
#N="{ 616, 535, 63, 200 }"
#N="{616, 535, 60, 1}"
D=1
ALIGN=$((2 ** 26 ))
STRIPE_COUNT=16
#Ts=26
#Ps=100
#N_x=616
#N_y=535
#N_z=63
#N_t=(60*100)

#PROCS="1 2"
PROCS="16 32 64 128 256 512 1024"
C="{ 32, 32, 64, 256 }"
HAVE_CHUNKING=true

function compile()
{
        output_file=$2
        CFLAGS=" -g "
        CFLAGS+="-I$TACC_HDF5_INC "
        LDFLAGS+="-Wl,-rpath,$TACC_HDF5_LIB -L$TACC_HDF5_LIB -lhdf5 -lz "
        SOURCES=$1
        mpicxx $SOURCES $CFLAGS $LDFLAGS -o $output_file "-DN=$N" "-DC=$C" -DD=$D -DALIGN=$ALIGN -DF=1 -DHAVE_CHUNKING=$HAVE_CHUNKING
}

for P in $PROCS
do

        jobdir=`printf '%s/procs-%04d' $work_dir $P`
        jobname="hdf5-5d"
        write_exe=$jobdir/write-bench.exe
        read_exe=$jobdir/read-bench.exe
        job=$jobdir/job

        mkdir -p $jobdir
        lfs setstripe --size $ALIGN --count $STRIPE_COUNT $jobdir
        compile write-bench.C $write_exe || { echo "Error: Couldn't compile write benchmark for P=$P" ; continue; }
        compile read-bench.C $read_exe || { echo "Error: Couldn't compile read benchmark for P=$P" ; continue; }

        cat > $job <<EOF
#!/bin/bash
#SBATCH -p normal
#SBATCH -N $(( ( P + 16 - 1)/16 ))
#SBATCH -n $P
#SBATCH -t 5
#SBATCH -o output.log
#SBATCH -D $jobdir
#SBATCH -J hdf5-5d
#SBATCH -d singleton
export TIMEFORMAT="%R"
ibrun -n $P -o 0 $write_exe
echo
ibrun -n $P -o 0 $read_exe
EOF

        echo Job $job is ready for submission
done
 
