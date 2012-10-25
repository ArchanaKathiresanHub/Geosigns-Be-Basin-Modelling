######################### BEGIN: PREDEFINED CACHE VARIABLES #########################
set( MPI_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/impi/4.0.1.007/intel64" CACHE PATH "Path to MPI library" )

set( HDF5_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5-intel-parallel/1.8.7/LinuxRHEL64_x86_64_57WS" CACHE PATH "Path to HDF5 library" )

set( PETSC_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/petsc-debug/3.1.8/LinuxRHEL64_x86_64_57WS" CACHE PATH "Path to Petsc library" )

set( BLA_VENDOR "Intel10_64lp_seq" CACHE STRING "Choice of BLAS/LAPACK vendor. Set to 'All' if any vendor isok. Vendors are: Goto, ATLAS PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, Intel10_32 (intel mkl v10 32 bit), Intel10_64lp (intel mkl v10 64 bit,lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v10 64 bit,sequential code, lp64 model), Intel( older versions of mkl 32 and 64 bit),ACML, ACML_MP, ACML_GPU, Apple, NAS, Generic" )
set( BLAS_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl" CACHE PATH "Path to BLAS library" )
set( BLAS_FOUND "ON" CACHE BOOL "Whether a BLAS library is available" )
set( BLAS_LINKER_FLAGS "" CACHE STRING "Linker flags required to link with BLAS" )
set( BLAS_LIBRARIES "-Wl,--start-group;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_intel_lp64.a;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_sequential.a;/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl/lib/intel64/libmkl_core.a;-Wl,--end-group" CACHE STRING "List of libraries that have to be linked to use BLAS" )
set( BLAS95_FOUND "OFF" CACHE BOOL "Whether Fortran 95 interface for BLAS is provided" )

set( LAPACK_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/composerxe-2011.3.174/mkl" CACHE PATH "Path to LAPACK library" )

set( FLEXLM_ROOT "/apps/3rdparty/EPTFlexLm/v9.2" CACHE PATH "Path to FlexLM directory" )
set( FLEXLM_LIBRARIES "/apps/3rdparty/EPTFlexLm/v9.2/LinuxRHEL_i686_30WS/lib64/EPTFlexLm.o" CACHE PATH "Path of FlexLM libraries" )
set( FLEXLM_INCLUDE_DIRS "/apps/3rdparty/EPTFlexLm/v9.2/include" CACHE PATH "Path of FlexLM include files" )

set( FP_ROOT "/glb/home/ksaho3/FunctionParser/src" CACHE PATH "Path to Function Parser library (see also http://warp.povusers.org/FunctionParser/ )" )
set( FP_LIBRARIES "/glb/home/ksaho3/FunctionParser/src/libparser.a" CACHE PATH "Path to Function Parser library" )
set( FP_INCLUDE_DIRS "/glb/home/ksaho3/FunctionParser/src" CACHE PATH "Path to Function Parser library include directory" )
set( TS_INCLUDE_DIRS "/glb/home/ksaho3/TsLib3.1.1_64/Lib" CACHE PATH "Path to Geocosm's TsLib include files" )
set( TS_LIBRARIES "/glb/home/ksaho3/TsLib3.1.1_64/Lib/libTsLib.a" CACHE PATH "Path to Geocosm's TsLib library" )
set( TS7_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlvam0/opt/tslib" CACHE PATH "Path to Geocosm's TsLib 7.0" )
set( TS7_LIBRARY_DIR "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlvam0/opt/tslib/bin/x86_64_linux/Release" CACHE PATH "Path to Geocosm's TsLib library" )

set( XSD_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlvam0/opt/tslib/3rdparty/xsd/xsd-3.3.0.b3-i686-linux-gnu" CACHE PATH "Path to Codesynthesis XSD library" )

set( XERCES_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlvam0/opt/tslib/3rdparty/Xerces/xerces-c-3.1.0-x86_64-linux-gcc-3.4" CACHE PATH "Path to Xerces-C library" )

set( QWT_ROOT "/nfs/rvl/users/ibs/IBS/EXTERNALS/QWT/curr/LinuxRHEL64_x86_64/64/OPTIMIZED" CACHE PATH "Path to QWT library" )

######################### END: PREDEFINED CACHE VARIABLES #########################
