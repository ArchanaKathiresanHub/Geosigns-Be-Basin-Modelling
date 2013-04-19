if (BM_USE_OWN_MPICH_LOCK)

	add_library(mpich2_lock STATIC
		src/lock.c
	)

	add_definitions(-DFORTRANUNDERSCORE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_ROMIOCONF_H)
	include_directories(
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

endif(BM_USE_OWN_MPICH_LOCK)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: