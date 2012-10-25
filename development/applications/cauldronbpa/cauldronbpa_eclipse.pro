TEMPLATE	= app
LANGUAGE	= C++
MOC_DIR = .moc
OBJECTS_DIR = obj.$(machine)
DESTDIR = obj.$(machine)

CONFIG	+= release
# CONFIG	+= debug
CONFIG	+= thread

INCLUDEPATH	+= .

INCLUDEPATH += $(QWTDIR)/include
INCLUDEPATH += ../../libraries/SerialDataAccess/src
INCLUDEPATH += ../../applications/MakeLeads/src

# CCLDIR points to the BPA CauldronClientLib directory
# CCLDIR = /glb/home/ksaho3/CauldronClientLib.3.4.0
# LIBS += $$(CCLDIR)/lib.$(machine)/libCauldronClientLib.a
# INCLUDEPATH += $$(CCLDIR)/include

INCLUDEPATH += ${BPA_HOME}/include
LIBS += ${BPA_HOME}/lib/libCauldronClientLib.a

LIBS += ../../libraries/SerialDataAccess/Release/libSerialDataAccess.a
LIBS += ../../libraries/Serial_Hdf5/Release/libSerial_Hdf5.a
LIBS += ../../libraries/CBMGenerics/Release/libCBMGenerics.a
LIBS += ../../libraries/utilities/Release/libutilities.a
LIBS += ../../libraries/TableIO/libTableIO.a
# LIBS += -L$(HDF5LIBS) -lhdf5
# LIBS += $(HDF5LIBS)/libhdf5.a
LIBS += $(HDF5_HOME)/lib/libhdf5.a

LIBS += -L$(QWTDIR)/lib -lqwt
# LIBS += ../../applications/MakeLeads/obj.$(machine)/libLeads.a
LIBS += $(soardir)/libPVTPACK.a

# child execution related files
# HEADERS += process.h
# SOURCES += process.cpp

# logindialog related files
INTERFACES += logindialogbase.ui
HEADERS += logindialog.h
SOURCES += logindialog.cpp

#rasterplot related files, used by gridmapview
HEADERS += rasterplot.h
SOURCES += rasterplot.cpp

# gridmapview related files, used by projectview
INTERFACES += gridmapviewbase.ui
HEADERS += gridmapview.h
SOURCES += gridmapview.cpp
HEADERS += scrollbar.h scrollzoomer.h
SOURCES += scrollbar.cpp scrollzoomer.cpp

# bpa object lists related files
HEADERS += infobase.h loaderthread.h
SOURCES += infobase.cpp loaderthread.cpp

# projectview related files
INTERFACES += projectviewbase.ui
HEADERS	+= projectview.h
SOURCES	+= projectview.cpp
HEADERS += maplistviewitem.h
SOURCES += maplistviewitem.cpp


# bpaclient related files
INTERFACES += bpaclientbase.ui
HEADERS += bpaclientbase.ui.h
HEADERS += bpaclient.h
HEADERS += iteminfo.h
SOURCES += bpaclient.cpp
SOURCES += iteminfo.cpp

# BPA Connectivity related files
# OBJECTS += ../CauldronClientLib/*.o
# SOURCES += ../CauldronClientLib/BPAClientLibrary.cpp
# SOURCES += ../CauldronClientLib/soapC.cpp
# SOURCES += ../CauldronClientLib/soapClient.cpp
# SOURCES += ../CauldronClientLib/stdsoap2.cpp
# OBJECTS += ../CauldronClientLib/BPAClientLibrary.o
# OBJECTS += ../CauldronClientLib/soapC.o
# OBJECTS += ../CauldronClientLib/soapClient.o
# OBJECTS += ../CauldronClientLib/stdsoap2.o

# exportwizard related files
INTERFACES += exportwizardbase.ui
SOURCES += exportwizard.cpp
HEADERS += exportwizard.h

SOURCES += main.cpp

SOURCES += misc.cpp
HEADERS += misc.h

QMAKE_CXXFLAGS += -DUSEINTERFACE

# compile flags
UNAME = $$system(uname -s)
message (this is a $$UNAME machine)
contains(UNAME,IRIX64) {
   QMAKE_LIBS_QT_THREAD += -lSM -lICE -ldl
   QMAKE_CXXFLAGS += -LANG:std
   QMAKE_CXXFLAGS += -woff 3201,1110,1107
   QMAKE_LFLAGS += -LANG:std
   QMAKE_LFLAGS += -woff 15,1110,1107
   LIBS += -lftn -lm
}

contains(UNAME,SunOS) {
   QMAKE_LIBS_QT_THREAD += -lSM -lICE -ldl
   LIBS +=  -lF77 -lM77 -lsunmath
   QMAKE_CXX = /opt/SUNWspro/bin/CC
   QMAKE_LINK = /opt/SUNWspro/bin/CC
}

contains(UNAME,Linux) {
   QMAKE_CXX = /usr/bin/g++
   QMAKE_LINK = /usr/bin/g++
   QMAKE_CXXFLAGS += -m64
   QMAKE_LIBS_QT += -lSM -lICE
   QMAKE_LIBS_QT_THREAD += -lSM -lICE -lXrender -lXrandr -lXcursor -lXft -lXinerama
   LIBS += -lm -lc -lg2c
}

