#
# Automatically generated from ${CMAKE_CURRENT_SOURCE_DIR}/envsetup.csh.cmake
#

setenv PATH ${MPI_ROOT}/bin:${HDF5_ROOT}/bin:/usr/bin:/bin:/usr/local/bin:${CMAKE_COMMAND_PATH}
setenv IBS_FIELDWIDTH 24
setenv IBS_LIMITEDBUILDANDINSTALL true

source /apps/3rdparty/intel/ics2011/composerxe-2011.3.174/bin/compilervars.csh intel64
source ${MPI_ROOT}/bin/mpivars.csh
source /apps/3rdparty/intel/ics2011/itac/8.0.1.009/bin/itacvars.csh
