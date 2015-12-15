TOP = ../..
CUR = moreslibs/SUMlib

include $(TOP)/config/all.config.mk

ifeq (1,$(MAKE_ACTION_BUILD))

BUILD_TYPE     = lib
BUILD_TARGET   = SUMlib

INCLUDEPATH += -Iinclude

DEFINES += -DSUMLIB_EXPORT

vpath %.cpp src
CXXSOURCE = $(wildcard src/*.cpp)

include $(CONFIG_DIR)/all.dobuild.mk

endif
