add_library( genex4_util
	src/gxexit.c src/gxggrs.c src/gxgloc.c src/gxgpar.c src/gxgsrs.c src/gxinit.c 
	src/gxkerr.c src/gxrun.c src/gxsloc.c src/gxspar.c src/ibsgetcnvrs.c 
	src/ibsgetexpapi.c src/ibsgetexpmc.c src/ibsgetexpr.c src/ibsgetgenmc.c 
	src/ibsgetgenr.c src/ibsgetvre.c src/ibsgxcfgprj.c src/ibsgxkernel.c 
	src/ibsutil.c
)

include_directories(../genex4_kernel/src)

install(TARGETS genex4_util
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

