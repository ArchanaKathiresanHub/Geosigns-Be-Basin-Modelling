#
# Automatically generated from ${CMAKE_CURRENT_SOURCE_DIR}/envsetup.csh.cmake
#

setenv PATH ${MPI_ROOT}/bin:${HDF5_ROOT}/bin:/usr/bin:/bin:/usr/local/bin:${CMAKE_COMMAND_PATH}:$PATH
setenv IBS_FIELDWIDTH 24
setenv IBS_LIMITEDBUILDANDINSTALL true

source /nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.3.163/bin/compilervars.csh intel64
source ${MPI_ROOT}/bin/mpivars.csh
source /nfs/rvl/apps/3rdparty/intel/itac/8.0.3.007/bin/itacvars.csh
