# The variable 'machine' must have been set to
# either Linux32, Silicon32, Silicon64, Sun or SunOS64

# The variable 'HDF5LIBS' must refer to the directory where 'libhdf5.a' can be found.
# The HDF5 library can be downloaded from 'http://hdf.ncsa.uiuc.edu/HDF5'.
# You will need the serial version.

# This (ad hoc) Makefile has been put together to make these applications and libraries
# compilable outside the Geocase/Cauldron environment.

include ../files/Make.versions

export PROCESSORCOUNT = $(shell /glb/home/ksaho3/bin/countprocs)

LN = /apps/oss/bin/ln -sf

#$(MAKE) SerialDataAccess OPT=$@
optimized profile debug clean doc depend scalasca:
	 $(MAKE) SerialDataAccess OPT=$@

veryclean:
	rm -rf `find . -name doc`
	rm -rf `find . -name obj.\*`
	rm -rf `find . -name lib`

TableIO: always

mpich2_lock: always

DataAccess: TableIO mpich2_lock CBMGenerics Serial_Hdf5 utilities always

SerialDataAccess: TableIO mpich2_lock CBMGenerics Serial_Hdf5 utilities DataAccess always

cauldron2eclipse: SerialDataAccess always

bpaclient: SerialDataAccess always

DistributedDataAccess: DataAccess TableIO CBMGenerics always

GeoPhysics: TableIO utilities DataAccess EosPack OTGC_kernel6 genex6 genex6_kernel Interpolation LinearAlgebra always


Parallel_Hdf5: always

Serial_Hdf5: mpich2_lock always

Utilities_Petsc: always

#genex4_kernel: always

genex5_kernel: CBMGenerics always

genex6_kernel: CBMGenerics always

OTGC_kernel6: CBMGenerics always

genex6: CBMGenerics always

OTGC_kernel: CBMGenerics always

calibration_kernel: CBMGenerics always

CrustalThickness: always

#genex4_util: always

EosPack: CBMGenerics always

functions: always

CBMGenerics: always

utilities: always

FiniteElements : always

ThreadUtilities: always

LinearAlgebra: ThreadUtilities always

Interpolation: ThreadUtilities LinearAlgebra always

DataMining: Interpolation ThreadUtilities LinearAlgebra GeoPhysics always

TableIO mpich2_lock DataAccess  SerialDataAccess cauldron2eclipse TestTableIO TestDataAccess \
   DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 Utilities_Petsc \
   genex5_kernel genex6_kernel genex6 OTGC_kernel6 GeoPhysics calibration_kernel CBMGenerics OTGC_kernel FiniteElements \
   LinearAlgebra Interpolation DataMining ThreadUtilities EosPack utilities functions CrustalThickness :
	cd $@; $(MAKE) --makefile=../../files/Makefile.library --jobs $(PROCESSORCOUNT) $(OPT)

bpaclient:
	cd $@; $(MAKE) $@

always:

