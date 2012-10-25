/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/RubyUtilities/src/IBSinclude.h,v 24.1 2004/12/23 15:53:51 ibs Stable $ */
/* ================================================================== */
/*
 * $Log: IBSinclude.h,v $
 * Revision 24.1  2004/12/23 15:53:51  ibs
 * BIM report 7478 Partly.
 * Upped the IBSLINESIZE from 500 to 650 to allow for extension of the LithoTableHeader.
 * The Soil mechanics properties have to fit in...
 *
 * Revision 24.0  2004/04/15 13:02:42  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:19:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.1  2003/01/17 12:14:53  ksaho3
 * - speed up of Cauldron during startup
 * - some code refactoring
 *
 * Revision 22.0  2002/06/28 11:45:03  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:32:40  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:44:14  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:20:52  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 12:49:50  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.1  1999/09/23 09:42:28  ksaho3
 * New, revised, better maintainable implementation of migration
 *
 * Revision 18.0  1999/08/01 14:08:58  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 13:37:42  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 21:58:46  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.1  1998/05/28 13:48:14  ibs
 * The log keyword put into another format to make RCS happy.
 *
 * Revision 15.0  1998/03/05 15:16:51  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.1  1997/11/18  15:00:22  cs319
 * Better function to compare snapshots
 *
 * Revision 14.0  1997/09/16  17:28:17  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  14:26:58  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:09:22  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 11.0  1996/07/15  13:37:00  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:57:51  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/12/20  09:01:07  cs319
 * RCS - header added
 * */
/* ================================================================== */

#define IBSNULLVALUE       		99999
#define IBSLINESIZE			650
#define IBSNUMBERLEN			50
#define IBSMINVALUE			1.0E-30
#define IBSMAXVALUE			1.0E+30
// #define IBSBETWEEN(a,x,b)		(((a)-(x))*((x)-(b))>=0?TRUE:FALSE)
#define IBSBETWEEN(a,x,b)		(((a) <= (x)) == ((b) >= (x)))
#define IBSPI				3.1415927

#define PA_TO_MPA                        0.000001  // conversion from Pa to MPa
#define MPA_TO_PA                        1000000.0 // conversion from MPa to Pa



#define PROPSYS_PRESENTDAY		0.0
#define PROPSYS_MAXSTRING		256
#define PROPSYS_NOSNAPSHOT		-9999
#define PROPSYS_STRING_NULL		""
#define PROPSYS_INTEGER_NULL		-9999
#define PROPSYS_SMALLINT_NULL		-9999
#define PROPSYS_BOOLEAN_NULL		-9999
#define PROPSYS_FLOAT_NULL		-9999.
#define PROPSYS_DOUBLE_NULL		-9999.
#define PROPSYS_CHARACTER_NULL	'?'
#define PROPSYS_CHARACTER_VALUE	'@'
#define PROPSYS_DEFAULTFILE		0
#define PROPSYS_PROJECTFILE		1
#define PROPSYS_IMPORTFILE		2
#define PROPSYS_EXPORTFILE		3
#define PROPSYS_MISCFILE		4
#define PROPSYS_SEARCH		"Propsrc"
#define PROPSYS_VALUE			"Propv"
#define PROPSYS_SNAPSHOT		"Propss"
#define PROPSYS_PROPSNAPSHOT		"Proppss"
#define PROPSYS_A_DAY                 (double)2.7397e-09
#define PROPSYS_SAMESNAPSHOT(x,y)	(ABS((x) - (y)) < PROPSYS_A_DAY)
