# The variable 'machine' must have been set to
# either Linux32, Silicon32, Silicon64, Sun or SunOS64

# The variable 'HDF5LIBS' must refer to the directory where 'libhdf5.a' can be found.
# The HDF5 library can be downloaded from 'http://hdf.ncsa.uiuc.edu/HDF5'.
# You will need the serial version.

# This (ad hoc) Makefile has been put together to make these applications and libraries
# compilable outside the Geocase/Cauldron environment.

LN = /apps/oss/bin/ln -sf

optimized debug profile clean doc depend regression:
	 $(MAKE) pvtpack OPT=$@

pvtpack: always

veryclean:
	rm -rf `find . -name doc`
	rm -rf `find . -name obj.\*`
	rm -rf `find . -name lib` 

pvtpack: 
	@mkdir -p $@/obj.${machine}
	@cd $@/obj.${machine}; $(LN) ../Makefile.$@ Makefile
	cd $@/obj.${machine}; $(MAKE) $(OPT)

always:

