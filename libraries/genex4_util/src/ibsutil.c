/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsutil.c,v 25.0 2005/07/05 08:03:57 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: ibsutil.c,v $
 * Revision 25.0  2005/07/05 08:03:57  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:48  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:19  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:51  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:56  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:45:17  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:42  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:53:05  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:29  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:39  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:57  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:44  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:33  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:45  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:07:12  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:26  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:43:04  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:09:11  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		28-SEP-1993

Description:	Utility routines used within the IBS implementation of GENEX-3
		functionality. ibsutil.c contains functions which manipulate data retrieved
		from the GENEX kernel and the IBS extensions to that kernel.

		A list of the available routines and their function
		prototypes are present in the file ibsutil.h.

Usage notes:	To be able to use the functions of this file ibsutil.h
		must be included.

History:
--------
28-SEP-1993	ErKo	initial version: IbsGetAvg, IbsGetSum, IbsGetCum
30-SEP-1993	ErKo	added IbsGetFullTimeRange and IbsGetOutputTimeRange
 1-OCT-1993	ErKo	In the IbsGet... routines the calculations are not
			performed anymore in case of an error in one of the
			arguments
 7-OCT-1993	ErKo	- added IbsGetVal
			- IbsGetOutputTimeRange is not used to determine the
			  actual output time values. The routine is called with
			  a 0.0 value as OutputDeltaTime. The functionality of
			  the routine has not been tested.
			- IbsGetSum, IbsGetAvg and IbsGetCum are not used
			  (their function might be incorrect - they have not
			  been tested)
17-DEC-1993     ErKo    Updates in IbsGEtVal
                        - removed the BETWEEN statement
                        - if SearchDouble returns -1: use NOutputTimeValues
                          for endIx
------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxnumer.h"
#include  "gxutils.h"
#include  "ibsgxkernel.h"
#include  "ibsgx.h"
#include  "ibsutil.h"

/* Prototypes of local functions */
/* ----------------------------- */

/* Local variables */
/* --------------- */

/*
------------------------------------------------------------------------------
IbsGetAvg -	Get average corresponding to the requested interval.
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
GxErrorCode IbsGetAvg
   (double   StartTime,
    double   EndTime,
    int      NTimeValues,
    double * TimeValues,
    double * Values,
    double * Result)
{
   GxErrorCode rc;
   double      localResult;
   int         startIx;
   int         endIx;
   int         i;

   rc = GXE_NOERROR;

   if   ((! TimeValues)
      || (! Values)
      || (! Result))
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* swap interval boundaries if required */
      if (EndTime > StartTime)
      {
         localResult = EndTime;
         EndTime = StartTime;
         StartTime = localResult;
      }
   }

   /* Get sum of values */
   rc = IbsGetSum (StartTime,
                   EndTime,
                   NTimeValues,
                   TimeValues,
                   Values,
                   & localResult);
   if (rc == GXE_NOERROR)
   {
      if (StartTime == EndTime)
      {
         *Result = localResult / (StartTime - EndTime);
      }
      else
      {
         *Result = localResult;
      }
   }

   return (rc);
}

/*
------------------------------------------------------------------------------
IbsGetCum -	Get cumulative value from the end of requested interval.
------------------------------------------------------------------------------

IbsGetCum takes the cumulative value corresponding to the specified
time interval. It is assumed that the array with values contains cumulative
values. Due to this assumption the value corresponding with the last period
for which values are available is used the requested cumulative value.
Interpolation is done on the requested interval however in stead of time
interval related to the last cumulative value:
Fullrange time values:   30 - 25 - 20 - 15 - 10 - 5 - 0
Requested time interval:         22 ------ 12
Interpolation is done
1) for 22 on the sub interval 25 - 20 to get an accurate value for 22
2) for 12 on the sub interval 22 - 10 (using the accurate value for 22) to
   get an accurate value for 12
------------------------------------------------------------------------------
*/
GxErrorCode IbsGetCum
   (double   StartTime,
    double   EndTime,
    int      NTimeValues,
    double * TimeValues,
    double * Values,
    double * Result)
{
   GxErrorCode rc;
   double      localResult;
   int         startIx;
   int         endIx;
   double      startResult;

   rc = GXE_NOERROR;
   
   if   ((! TimeValues)
      || (! Values)
      || (! Result))
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* swap interval boundaries if required */
      if (EndTime > StartTime)
      {
         localResult = EndTime;
         EndTime = StartTime;
         StartTime = localResult;
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Get location in time Values array */
      startIx = MAX (0, GxSearchDouble (NTimeValues, TimeValues, StartTime));
      endIx   = MAX (0, GxSearchDouble (NTimeValues, TimeValues, EndTime));

      if (startIx == 0)
      {
         startResult = Values [startIx];
      }
      else
      {
         /* Get (partial) start value */
         startResult = GxLinearInterpolate (StartTime,
                                            TimeValues [startIx -1],
                                            TimeValues [startIx],
                                            Values [startIx -1],
                                            Values [startIx]);
      }
      /* Get (partial) end total value (using the found start value) */
      localResult = GxLinearInterpolate (EndTime,
                                         StartTime,
                                         TimeValues [endIx],
                                         startResult,
                                         Values [endIx]);

      /* Return the value for the requested time interval */
      *Result = localResult;
   }

   return (rc);
}

/*
------------------------------------------------------------------------------
IbsGetFullTimeRange -	Get time range for GENEX has calculated results.
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
GxErrorCode IbsGetFullTimeRange
   (double *  StartTime,
    double *  EndTime,
    int *     NTimeValues,
    double ** TimeValues)
{
   GxErrorCode rc;

   rc = GXE_NOERROR;

   rc = GxGetGeologicalTimeRange (StartTime, EndTime);

   if (rc == GXE_NOERROR)
   {
      rc = GxGetGeologicalTimesteps (*StartTime, *EndTime, NTimeValues); 
   }

   /* Create array to store full range time Values */
   if (rc == GXE_NOERROR)
   {
      rc = GxCreateModelHistory (*NTimeValues, TimeValues);
   }

   /* Fill FullRange time Values */
   if (rc == GXE_NOERROR)
   {
      rc = GxGetGeologicalTimes (*StartTime, *EndTime, *TimeValues);
   }

   return (rc);
}

/*
------------------------------------------------------------------------------
IbsGetOutputTimeRange -	Get time values based on start-, end- and deltatime.
------------------------------------------------------------------------------
IbsGetOutputTimeRange determines the values for which output is requested.

The output time values can be determined in two ways:
- store the desired time values in array OutputTimeValues and store the number
  of time values in NOutputTimeValues; specify 0.0 as delta-time value
- specify a value > 0.0 for OutputDeltaTime and make sure OutputStartTime,
  OutputEndTime and OutputDeltaTime are filled according to the desired range
  of time values.
  Note that the determined time values and the number of time values are
  returned in the arguments OutputTimeValues and NOutputTimeValues respectivily.
  The input values in these arguments are overwritten!

Since the output time values are always determined by burtop.f this routine
does not determine the output time values (OutputDeltaTime is equal to 0.0).
That part of the routine has not been tested.
------------------------------------------------------------------------------
*/
GxErrorCode IbsGetOutputTimeRange
   (int *   NOutputTimeValues, /* I/O */
    float   OutputStartTime,   /* I   */
    float   OutputEndTime,     /* I   */
    float   OutputDeltaTime,   /* I   */
    float * OutputTimeValues)  /*   O */
{
   GxErrorCode rc;
   int         i;

   rc = GXE_NOERROR;

   if (OutputDeltaTime != 0.0)
   {
      /* Use the begin- and end time and the DeltaTime to fill
         an array with requested time Values.
      */
      *NOutputTimeValues = 1 + ROUND ((OutputStartTime - OutputEndTime) /
                                      OutputDeltaTime);
      OutputTimeValues [0] = OutputStartTime;
      for (i = 1; i < *NOutputTimeValues; i++)
      {
         OutputTimeValues [i] = OutputTimeValues [i -1] - OutputDeltaTime;
      }
   }
   else
   {
      /* The requested time values are specied by the caller:
         there is nothing that has to be done now. 
      */
      ;
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
IbsGetSum -	Get sum of values corresponding to the requested interval.
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
GxErrorCode IbsGetSum
   (double   StartTime,
    double   EndTime,
    int      NTimeValues,
    double * TimeValues,
    double * Values,
    double * Result)
{
   GxErrorCode rc;
   double      localResult;
   int         startIx;
   int         endIx;
   int         i;

   rc = GXE_NOERROR;
   
   if   ((! TimeValues)
      || (! Values)
      || (! Result))
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* swap interval boundaries if required */
      if (EndTime > StartTime)
      {
         localResult = EndTime;
         EndTime = StartTime;
         StartTime = localResult;
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Get locations in time Values array */
      startIx = MAX (0, GxSearchDouble (NTimeValues, TimeValues, StartTime));
      endIx   = MAX (0, GxSearchDouble (NTimeValues, TimeValues, EndTime));

      localResult = 0.0;

      if (StartTime > TimeValues [0])
      {
         /* Start time is before start of time array.
         */
         localResult += Values [0] * (StartTime - TimeValues [0]);
      }
      else 
      {
         /* Get first (partial) value */
         localResult += GxLinearInterpolate (StartTime,
                                             TimeValues [MAX (startIx -1, 0)],
                                             TimeValues [startIx],
                                             Values [MAX (startIx -1, 0)],
                                             Values [startIx]);
      }

      /* Get sum of values for which information is available */
      for (i = startIx; i < endIx; i++)
      {
         localResult += Values [i];
      }

      if (EndTime < TimeValues [NTimeValues -1])
      {
         /* End time is after end of time array.
            Get value for time interval after the interval for which data is
            available: multiply 'after' time interval with last value
         */
         localResult += Values [NTimeValues -1] *
                           (TimeValues [NTimeValues -1] - EndTime);
      }
      else
      {
         /* Get last (partial) value */
         localResult += GxLinearInterpolate (EndTime,
                                             TimeValues [MAX (endIx -1, 0)],
                                             TimeValues [endIx],
                                             Values [MAX (endIx -1, 0)],
                                             Values [endIx]);
      }
      /* Return the sum of Values for the specified time interval */
      *Result = localResult;
   }

   return (rc);
}

/*
------------------------------------------------------------------------------
IbsGetVal -	Get value from the end of requested interval.
------------------------------------------------------------------------------

IbsGetVal takes the value corresponding to the specified time interval. It is
assumed that the array with values contains values which are representative
for the sub intervals. Due to this assumption the value corresponding with the
last sub interval (corresponding with the end-time of te requested interval)
is used as the requested  value. Interpolation is done using that sub
interval.
------------------------------------------------------------------------------
*/
GxErrorCode IbsGetVal
   (double   StartTime,
    double   EndTime,
    int      NTimeValues,
    double * TimeValues,
    double * Values,
    double * Result)
{
   GxErrorCode rc;
   double      localResult;
   int         endIx;

   rc = GXE_NOERROR;
   
   if   ((! TimeValues)
      || (! Values)
      || (! Result))
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* swap interval boundaries if required */
      if (EndTime > StartTime)
      {
         localResult = EndTime;
         EndTime = StartTime;
         StartTime = localResult;
      }

      /* Get location in time Values array */
      endIx = GxSearchDouble (NTimeValues, TimeValues, EndTime);
      if (endIx == -1)
      {
         endIx = NTimeValues - 1;
      }

      if (endIx == 0)
      {
         localResult = Values [0];
      }
      else
      {
         /* Get (partial) value */
         localResult = GxLinearInterpolate (EndTime,
                                            TimeValues [endIx -1],
                                            TimeValues [endIx],
                                            Values     [endIx -1],
                                            Values     [endIx]);
      }

      /* Return the value for the requested time interval */
      *Result = localResult;
   }

   return (rc);
}
