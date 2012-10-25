#
#
#

# For this module we use a for Cauldron specifically adjusted version
# of some GeoCase templates. Therefore we need to adjust the TNGPATH in order
# to precede the GeoCase version.

set(TNGPATH . $(TNGPATH))
set(target_name RubyUtilities)

file(GLOB MODSRCC "${BB_ROOT}/RubyUtilities/src/*.C")
file(GLOB MODSRCH "${BB_ROOT}/RubyUtilities/src/*.h")

set(MODSRCD 
  RBYutilities.def
  IBSfile.def
  IBSpss.def
  IBSstrng.def
  IBSv.def
  IBScfunc.def
  IBSmain.def
  IBSpvec.def
  IBSsrc.def
  IBSwldxy.def
  IBSdistr.def 
  IBSpolyf.def
  IBSshw.def
  IBSshwui.def
  IBSss.def
  IBStman.def
  IBSpatom.def
  IBSpbhan.def
)

set(MODSRCD )
set(MODSRCI )
set(MODSRCV )
set(MODSRCL )
set(MODSRCY )

set(MODSRCO )

#   [5] Additional include directories.
# We need to specify these features explicitly here, since we have "plain"
# C++ source files that make use of these toolkits. If we do not specify
# these features here, dependency generation for these C++ source files will
# fail, since the include paths for the related toolkits (DDS and GOSPEL)
# are not passed on to the compiler.
set(FEATURES DDS GOSPEL UNITSYS UI NOGL)

set(MODINC )

#   [6] Additional libraries (before and after the local library).
#set(MODLIB )
# OPTIONAL:
#   [6b] Debug versions of MODLIB and MODLIBA
set(MODLIBD )
#   [7] Reference Directories.
#set(MODDIR ${BB_ROOT}/${target_name})
#set(MODSDIR ${MODDIR}/src)
#set(MODDDIR = $(MODDIR)/def)
##   [8] Files which can be deleted after a compile.
#MODREM  =
#   [9] Installation directories.
#MODLDIR = 
#MODBDIR = 
#MODIDIR = 
#  [10] Compiler and Linker flags.
#UCOPTION = $(DOPTION)
#UOPTIM = -DNOTRACE
#ULOPTION = $(DLOPTION)
#  [11] Module version
#MODVER = $(RM_RELEASE)
#  [12] File that contains the main().
#MODEXEC = 
#  [13] Code Generator version.
set(NORCS TRUE)

#===============================================================
# generate code on module basis, uncomment next statement if
# code generating on class level is preferred
#TNGOPT = -macro MERGEOUTPUT TRUE
#===============================================================


include_directories(${GEOCASE_BASE_DIR}/include/nogl)
include_directories(${GEOCASE_BASE_DIR}/include/usys)
include_directories(${GEOCASE_BASE_DIR}/include/ui)
include_directories(${GEOCASE_BASE_DIR}/include/gospel)
include_directories(${GEOCASE_BASE_DIR}/include/dds)
include_directories(${GEOCASE_BASE_DIR}/include/dds)
include_directories(${GEOCASE_BASE_DIR}/include/noxint)
include_directories(/nfs/rvl/users/ibs/geocase_64/utilities/Linux64g/xpm/include)

add_library(${target_name} STATIC ${MODSRCC} ${MODSRCH})
