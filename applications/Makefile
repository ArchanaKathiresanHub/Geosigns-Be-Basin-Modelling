# The variable 'machine' must have been set to
# either Linux32, Silicon32, Silicon64, Sun or SunOS64

# The variable 'HDF5LIBS' must refer to the directory where 'libhdf5.a' can be found.
# The HDF5 library can be downloaded from 'http://hdf.ncsa.uiuc.edu/HDF5'.
# You will need the serial version.

# This (ad hoc) Makefile has been put together to make these applications and libraries
# compilable outside the Geocase/Cauldron environment.
#
# fastmiglogger, cauldronbpa and geomorph added. Now with Linux (all 64 bits) and normal (32 and 64) build.
#
include ../files/Make.versions

export PROCESSORCOUNT = $(shell /glb/home/ksaho3/bin/countprocs)
LN = /apps/oss/bin/ln -sf

optimized debug profile clean doc depend scalasca:
	 $(MAKE) fastcauldron OPT=$@

veryclean:
	rm -rf `find . -name doc`
	rm -rf `find . -name obj.\*`
	rm -rf `find . -name lib`

releaseLinux64: \
	fastcauldron fastmig fastgenex5 fastgenex6 fastreq fasttouch fasttouch7 cauldron2eclipse cauldron2voxet datadriller deviatedwell track1d datamining isosurface cauldron2bhf xcauldron trapper gempis cauldronbpa cauldronmaps fastmiglogger geomorph gempis_recv gempis_send upgrade setupView4D fastctc

release64: \
	fastcauldron fastmig fastgenex5 fastgenex6 fastreq fasttouch fasttouch7 cauldron2eclipse cauldron2voxet datadriller deviatedwell track1d datamining isosurface cauldron2bhf xcauldron trapper gempis geomorph gempis_recv gempis_send upgrade setupView4D fastctc

release32: \
	cauldronmaps trapper gempis cauldronbpa fastmiglogger

fastcauldron: \
	FiniteElements Parallel_Hdf5 Serial_Hdf5 Utilities_Petsc CBMGenerics TableIO DistributedDataAccess GeoPhysics \
	utilities ThreadUtilities LinearAlgebra Interpolation calibration_kernel CrustalThickness

testDistribution: always

testDiffusionLeakage: always


testInterpolation: always

testInterpolation: ThreadUtilities LinearAlgebra Interpolation


TestTrapArea TestDistributedDataAccess: \
	TableIO DataAccess DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 genex5_kernel CBMGenerics \
	Utilities_Petsc EosPack utilities

fasttouch : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 \
	Utilities_Petsc utilities CBMGenerics

fasttouch7 : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 \
	Utilities_Petsc utilities CBMGenerics

fastgenex5 : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 genex5_kernel \
	Utilities_Petsc utilities EosPack CBMGenerics OTGC_kernel

fastgenex6 : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 genex6 genex6_kernel \
	Utilities_Petsc utilities EosPack CBMGenerics OTGC_kernel6

fastmig : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 genex6 genex6 \
	Utilities_Petsc EosPack utilities CBMGenerics functions OTGC_kernel6

fastctc : \
	TableIO DistributedDataAccess Parallel_Hdf5 Serial_Hdf5 CrustalThickness Utilities_Petsc 

pargrid: Parallel_Hdf5 Serial_Hdf5 Utilities_Petsc

Parallel_Hdf5 Serial_Hdf5 Utilities_Petsc DataAccess DistributedDataAccess \
		SerialDataAccess TableIO genex5_kernel genex6_kernel genex6 OTGC_kernel OTGC_kernel6 EosPack utilities CBMGenerics GeoPhysics functions \
		ThreadUtilities LinearAlgebra Interpolation DataMining calibration_kernel FiniteElements CrustalThickness:
	cd ../libraries; $(MAKE) $@ OPT=${OPT}

TestDataAccess: \
	DataAccess SerialDataAccess TableIO CBMGenerics

cauldron2bhf: \
	SerialDataAccess TableIO CBMGenerics always

cauldron2eclipse: \
	SerialDataAccess TableIO CBMGenerics always

PreferredDirectionUnitTest: FiniteElements utilities

cauldron2voxet: \
	SerialDataAccess TableIO CBMGenerics always

datadriller: \
	SerialDataAccess TableIO CBMGenerics Interpolation EosPack DataMining always

deviatedwell: \
	SerialDataAccess TableIO CBMGenerics Interpolation DataMining FiniteElements GeoPhysics EosPack Utilities_Petsc OTGC_kernel6 always

track1d: \
	SerialDataAccess TableIO CBMGenerics Interpolation DataMining always

datamining: \
	SerialDataAccess TableIO CBMGenerics Interpolation DataMining always

isosurface: \
	SerialDataAccess TableIO CBMGenerics always

fastmiglogger: \
	EosPack SerialDataAccess TableIO CBMGenerics always

xcauldron: \
	TableIO always

TestTableIO: TableIO always

fastreq: \
	SerialDataAccess TableIO Serial_Hdf5 CBMGenerics utilities

fastcompare: \
	SerialDataAccess TableIO Serial_Hdf5 CBMGenerics utilities

fastreport: \
	SerialDataAccess TableIO Serial_Hdf5 CBMGenerics utilities

geomorph: \
	SerialDataAccess TableIO Serial_Hdf5 utilities ThreadUtilities LinearAlgebra Interpolation

gempis_recv:

gempis_send:

upgrade:

setupView4D:

UnitTestGenex5: \
	genex5_kernel CBMGenerics

UnitTestOTGC: \
	OTGC_kernel CBMGenerics

calibrationUnitTest: \
	calibration_kernel 

testFissionTracks: \
        CBMGenerics calibration_kernel 

EosPackDemo: \
	EosPack CBMGenerics

trapper: \
	EosPack TableIO Serial_Hdf5 CBMGenerics

elfen: always

gempis: always

ListProperties: always

TestTableIO: TableIO

subscale: TableIO

MakeLeads: always

tracktraps: always

cauldronbpa: always

cauldronmaps: always

fastcauldron: always

trapper: always

fastmig TestTrapArea TestDistributedDataAccess fastgenex5 fastgenex6 fasttouch fasttouch7: always

geomorph: always

gempis_recv: always

gempis_send: always

upgrade: always

setupView4D: always

testmpi: always

pargrid: always

fastmig fastgenex5 fastgenex6 fasttouch fasttouch7 UnitTestGenex5 TestTrapArea TestDistributedDataAccess fastcauldron pargrid cauldron2bhf cauldron2eclipse cauldron2voxet datadriller deviatedwell track1d datamining isosurface fastmiglogger xcauldron TestTableIO TestDataAccess fastreq fastreport subscale ListProperties testmpi EosPackDemo testDistribution testDiffusionLeakage calibrationUnitTest UnitTestOTGC geomorph gempis_recv gempis_send upgrade setupView4D testInterpolation testFissionTracks PreferredDirectionUnitTest fastctc:
	@mkdir -p $@/obj.${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) --jobs $(PROCESSORCOUNT) $(OPT)

gempis:
	@mkdir -p $@/obj.${machine}
	@mkdir -p $@/tmp.${machine}
	@mkdir -p $@/bin.${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) $(OPT)

elfen:
	@mkdir -p $@/obj
	@mkdir -p $@/tmp
	@mkdir -p $@/bin
	@cd $@/obj; $(LN) ../Makefile.$@ Makefile
	cd $@/obj; $(MAKE) $(OPT)

trapper:
	@mkdir -p $@/obj.${machine}
	@mkdir -p $@/tmp.${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) $(OPT) BOPT=O_c++

MakeLeads: SerialDataAccess TableIO
	@mkdir -p $@/obj.${machine}
	@mkdir -p lib/${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) $(OPT)
	@cd lib/${machine}; $(LN) ../../$@/obj.${machine}/*.so .
	@cd lib/${machine}; $(LN) ../../$@/obj.${machine}/*.a .

tracktraps: EosPack CBMGenerics SerialDataAccess TableIO
	@mkdir -p $@/obj.${machine}
	@mkdir -p lib/${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) $(OPT)
	@cd lib/${machine}; $(LN) ../../$@/obj.${machine}/*.so .
	@cd lib/${machine}; $(LN) ../../$@/obj.${machine}/*.a .

cauldronbpa: SerialDataAccess TableIO
	cd $@; qmake; $(MAKE)

cauldronmaps: SerialDataAccess TableIO
	cd $@; qmake; $(MAKE)

always:

