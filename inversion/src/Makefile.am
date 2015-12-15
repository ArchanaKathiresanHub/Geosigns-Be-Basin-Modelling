#Compiler options
AM_CPPFLAGS = -g -Wall -O3 -funroll-loops -Wno-reorder 
#List of programs to compile
bin_PROGRAMS= wellmodelcalibration thermalcalibration cloneproject3d

cloneproject3d_SOURCES= cloneproject3d.cpp 
cloneproject3d_CPPFLAGS= $(AM_CPPFLAGS)

thermalcalibration_SOURCES= thermalcalibration.cpp
thermalcaliration_CPPFLAGS= $(AM_CPPFLAGS)

runtest_SOURCES= runtest.cc test.cc
runtest_CPPFLAGS= $(AM_CPPFLAGS)

wellmodelcalibration_SOURCES= wellmodelcalibration.cpp
wellmodelcalibration_CPPFLAGS= $(AM_CPPFLAGS)
wellmodelcalibration_LDFLAGS = $(AM_LDFLAGS)  -L/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Boost/boost_1_47_0/libs -pthread -lboost_thread
#de_SOURCES=de4_0.cpp de.h eval.cpp sort.cpp
#de_CPPFLAGS= $(AM_CPPFLAGS)

#coupledmodel_SOURCES=usual_functions.cpp wellmodel.cpp coupledmodel.cpp
#coupledmodel_CPPFLAGS= $(AM_CPPFLAGS) 


#cleanup:
#	rm -rf *_xml_data tmp/* *.xml *.dfirst* *.odi *.oli *.log *.edi *.fad  *.sdi *.int1 *.sli


#gcc -Wall -fopenmp de4_0.cpp de.h eval.cpp sort.cpp 
