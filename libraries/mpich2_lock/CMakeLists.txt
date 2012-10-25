if (BM_USE_OWN_MPICH_LOCK)

	add_library(mpich2_lock STATIC
		src/lock.c
	)

	set(flags)
	foreach( flag -O2 -DFORTRANUNDERSCORE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_ROMIOCONF_H)
		set(flags "${flags} ${flag}")
	endforeach(flag)

	foreach( dir 
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpi/romio/adio/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpl/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/openpa/src
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/ch3/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/common/datatype
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/common/locks
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/ch3/channels/nemesis/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/ch3/channels/nemesis/nemesis/include
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/mpid/ch3/channels/nemesis/nemesis/utils/monitor
	  /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/src/util/wrappers
	)
		set(flags "${flags} -I${dir}")
	endforeach(dir) 

	set_source_files_properties( src/lock.c PROPERTIES COMPILE_FLAGS "${flags}")
	set_directory_properties( PROPERTIES INCLUDE_DIRECTORIES "")

endif(BM_USE_OWN_MPICH_LOCK)
