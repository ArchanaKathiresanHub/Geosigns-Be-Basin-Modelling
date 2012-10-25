/*******************************************************************************
* Class:       FtSample
*
* File:        ftsample.C
*
* Copyright:   (c) 1997 Shell International Exploration and Production
*
* GeoCase rev: v1.0
*
* Description: 

 ~ ===========================================================================
 ~
 ~  Class Name:       FtSample
 ~  Subclass of:  
 ~  Description:      Appatite fission track sample
 ~  Attributes:
 ~    Name              type             Description
 ~    -------------------------------------------------------------------------
 ~    SAMPLECODE        STRING           Sample identifier
 ~    DEPTH             float            Depth from where the sample was taken from
 ~    DEPOAGE           float            Depositional age of the location where the sampl
 ~                                       was taken from 
 ~    TOPNODEID         int              Identifier of node just above sample location 
 ~    BOTTOMNODEID      int              Identifier of node just below sample location 
 ~    LENGTHFACTOR      float            factor to locate sample location in layer
 ~    SAMPLELOCATIONSET BOOLEAN_t        Flag that is set when sample location is set
 ~    TMPBINCNT         PropDoubleVector Vector used in track length calculation
 ~    AFTCALC           AftCalc          Pointer to the parent AftCalc
 ~    ObsTracklengths   DaList           List of observed track length distributions
 ~    PredTracklengths  DaList           List of predicted track length distributions
 ~    GRAINFTAGEDIST    Histogram        Grain age histogram
 ~
 ~ ============================================================================


*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                             FTSAMPLE Methods                              //
///////////////////////////////////////////////////////////////////////////////
// This object is a member of the module RUBYPROJECT in project RBYPROJECT   //
///////////////////////////////////////////////////////////////////////////////
#include   <stdio.h>
#include   <string.h>
#include   "DDSall.h"
#define  ftsample__file__name  1
#include   "rbyproject.h"
#include   "ftsample.h"
#include   "ftsampleP.h"

#include "aftcalc.h"
#include "tempcalc.h"
#include "project.h"
#include "calcengine.h"

#include "glgmanager.h"
#include "needlenodeiterator.h"
#include "segment.h"
#include "node.h"
#include "histogram.h"
#include "propdoublevector.h"

#include "RBYconstants.h"


///////////////////////////////////////////////////////////////////////////////
//                  Private and Static method declarations.                  //
///////////////////////////////////////////////////////////////////////////////

void FtSample_Add_FtGrainAges (FtSample_c ftsample, Histogram_c member)
{
   ftsample->FtSample::Add_FtGrainAges (member);
}
void FtSample_Add_ObsTracklengths (FtSample_c ftsample, Histogram_c member)
{
   ftsample->FtSample::Add_ObsTracklengths (member);
}
void FtSample_Add_PredTracklengths (FtSample_c ftsample, Histogram_c member)
{
   ftsample->FtSample::Add_PredTracklengths (member);
}
void FtSample_BinFtGrainAge (FtSample_c ftsample, float ClPerc, float
grainAge)
{
   ftsample->FtSample::BinFtGrainAge (ClPerc, grainAge);
}
void FtSample_BinObsTracklengths (FtSample_c ftsample, float tracklength, float
ClPerc)
{
   ftsample->FtSample::BinObsTracklengths (tracklength, ClPerc);
}
void FtSample_CalcFtLengthChi2 (FtSample_c ftsample, float *Chi2)
{
   ftsample->FtSample::CalcFtLengthChi2 (Chi2);
}
BOOLEAN_t FtSample_CalcPredFtAge (FtSample_c ftsample, float *PredFtAge)
{
   return ftsample->FtSample::CalcPredFtAge (PredFtAge);
}
void FtSample_CalcPredTracklengths (FtSample_c ftsample, double timestep, double
                                    temperature, int ClIndex)
{
   ftsample->FtSample::CalcPredTracklengths (timestep, temperature, ClIndex);
}
FtSample_c FtSample_Create (BOOLEAN_t alloc)
{
   return FtSample::Create (alloc);
}
void FtSample_Delete (FtSample_c ftsample)
{
   ftsample->FtSample::Delete ();
}
double DensityFromR (double r)
{
   return FtSample::DensityFromR (r);
}
void FtSample_ForAll_FtGrainAges (FtSample_c ftsample, ActionMethod Action, long
*arg)
{
   ftsample->FtSample::ForAll_FtGrainAges (Action, arg);
}
void FtSample_ForAll_ObsTracklengths (FtSample_c ftsample, ActionMethod
                                      Action, long *arg)
{
   ftsample->FtSample::ForAll_ObsTracklengths (Action, arg);
}
void FtSample_ForAll_PredTracklengths (FtSample_c ftsample, ActionMethod
                                       Action, long *arg)
{
   ftsample->FtSample::ForAll_PredTracklengths (Action, arg);
}
BOOLEAN_t FtSample_FtGrainAges_GtFunc (Histogram_c histogram1, Histogram_c
histogram2)
{
   return FtSample::FtGrainAges_GtFunc (histogram1, histogram2);
}
void FtSample_GeotrackCorrPredTracklengthDists (FtSample_c ftsample)
{
   ftsample->FtSample::GeotrackCorrPredTracklengthDists ();
}
float FtSample_GetClContFraction (FtSample_c ftsample, int ClBin,
                                  FtGrainIoTbl_c FtGrainIoTbl)
{
   return ftsample->FtSample::GetClContFraction (ClBin, FtGrainIoTbl);
}
void FtSample_GetFrom_FtGrainAges (FtSample_c ftsample, Histogram_c * member, long
key)
{
   ftsample->FtSample::GetFrom_FtGrainAges (member, key);
}
void FtSample_GetFrom_ObsTracklengths (FtSample_c ftsample, Histogram_c *
                                       member, long key)
{
   ftsample->FtSample::GetFrom_ObsTracklengths (member, key);
}
void FtSample_GetFrom_PredTracklengths (FtSample_c ftsample, Histogram_c *
                                        member, long key)
{
   ftsample->FtSample::GetFrom_PredTracklengths (member, key);
}
void FtSample_GetGrainAgeDist (FtSample_c ftsample, int ChlorideIx,
                               HistogramTbl_c HistogramTbl)
{
   ftsample->FtSample::GetGrainAgeDist (ChlorideIx, HistogramTbl);
}
void FtSample_GetMaxBinCount (FtSample_c ftsample, double *MaxTracknum, double
*MaxGrainnum)
{
   ftsample->FtSample::GetMaxBinCount (MaxTracknum, MaxGrainnum);
}
void FtSample_GetObsTracklengthDist (FtSample_c ftsample, int ChlorideIx,
                                     HistogramTbl_c HistogramTbl)
{
   ftsample->FtSample::GetObsTracklengthDist (ChlorideIx, HistogramTbl);
}
void FtSample_GetPredTracklengthDist (FtSample_c ftsample, int ChlorideIx,
                                      HistogramTbl_c HistogramTbl)
{
   ftsample->FtSample::GetPredTracklengthDist (ChlorideIx, HistogramTbl);
}
void FtSample_GetSumGrainAgeDist (FtSample_c ftsample, HistogramTbl_c
                                  HistogramTbl, double *MaxBinCnt)
{
   ftsample->FtSample::GetSumGrainAgeDist (HistogramTbl, MaxBinCnt);
}
void FtSample_GetSumObsTracklengthDist (FtSample_c ftsample, HistogramTbl_c
                                        HistogramTbl, double *MaxBinCnt)
{
   ftsample->FtSample::GetSumObsTracklengthDist (HistogramTbl, MaxBinCnt);
}
void FtSample_GetSumPredTracklengthDist (FtSample_c ftsample, HistogramTbl_c
                                         HistogramTbl, double *MaxBinCnt)
{
   ftsample->FtSample::GetSumPredTracklengthDist (HistogramTbl, MaxBinCnt);
}
BOOLEAN_t FtSample_GetTemperature (FtSample_c ftsample, double *temperature)
{
   return ftsample->FtSample::GetTemperature (temperature);
}
int IndexFromClPerc (float ClPerc)
{
   return FtSample::IndexFromClPerc (ClPerc);
}
void FtSample_Init (FtSample_c ftsample)
{
   ftsample->FtSample::Init ();
}
void FtSample_Initialize (FtSample_c ftsample, STRING sampleId, float depth,
                          FtGrainIoTbl_c FtGrainIoTbl, FtLengthIoTbl_c
                          FtLengthIoTbl, StratIoTbl_c StratIoTbl)
{
   ftsample->FtSample::Initialize (sampleId, depth, FtGrainIoTbl,
                                   FtLengthIoTbl, StratIoTbl);
}
FtSample_c FtSample_Make (void)
{
   return FtSample::Make ();
}
void FtSample_MakeDistributions (FtSample_c ftsample, FtGrainIoTbl_c
                                 FtGrainIoTbl, FtLengthIoTbl_c FtLengthIoTbl)
{
   ftsample->FtSample::MakeDistributions (FtGrainIoTbl, FtLengthIoTbl);
}
void FtSample_MergeTracklengthDists (FtSample_c ftsample)
{
   ftsample->FtSample::MergeTracklengthDists ();
}
BOOLEAN_t FtSample_ObsTracklengths_GtFunc (Histogram_c histogram1, Histogram_c
histogram2)
{
   return FtSample::ObsTracklengths_GtFunc (histogram1, histogram2);
}
BOOLEAN_t FtSample_PredTracklengths_GtFunc (Histogram_c histogram1,
                                            Histogram_c histogram2)
{
   return FtSample::PredTracklengths_GtFunc (histogram1, histogram2);
}
void FtSample_Print (FtSample_c ftsample, long *arg)
{
   ftsample->FtSample::Print (arg);
}
void FtSample_PrintFtGrainAges (FtSample_c ftsample)
{
   ftsample->FtSample::PrintFtGrainAges ();
}
void FtSample_PrintObsTracklengths (FtSample_c ftsample)
{
   ftsample->FtSample::PrintObsTracklengths ();
}
void FtSample_PrintPredTracklengths (FtSample_c ftsample)
{
   ftsample->FtSample::PrintPredTracklengths ();
}
void FtSample_Remove_FtGrainAges (FtSample_c ftsample, Histogram_c member)
{
   ftsample->FtSample::Remove_FtGrainAges (member);
}
void FtSample_Remove_ObsTracklengths (FtSample_c ftsample, Histogram_c member)
{
   ftsample->FtSample::Remove_ObsTracklengths (member);
}
void FtSample_Remove_PredTracklengths (FtSample_c ftsample, Histogram_c
member)
{
   ftsample->FtSample::Remove_PredTracklengths (member);
}
void FtSample_ScaleObsTracklengthDists (FtSample_c ftsample, int tracknum)
{
   ftsample->FtSample::ScaleObsTracklengthDists (tracknum);
}
void FtSample_ScalePredTracklenghtDists (FtSample_c ftsample, int tracknum)
{
   ftsample->FtSample::ScalePredTracklenghtDists (tracknum);
}
void FtSample_ScalePredTracklengthsToObserved (FtSample_c ftsample)
{
   ftsample->FtSample::ScalePredTracklengthsToObserved ();
}
BOOLEAN_t FtSample_SetFtSampleLocation (FtSample_c ftsample, StratIoTbl_c
StratIoTbl)
{
   return ftsample->FtSample::SetFtSampleLocation (StratIoTbl);
}
void FtSample_SetIn_FtGrainAges (FtSample_c ftsample, Histogram_c member, long
key)
{
   ftsample->FtSample::SetIn_FtGrainAges (member, key);
}
void FtSample_SetIn_ObsTracklengths (FtSample_c ftsample, Histogram_c member, long
key)
{
   ftsample->FtSample::SetIn_ObsTracklengths (member, key);
}
void FtSample_SetIn_PredTracklengths (FtSample_c ftsample, Histogram_c member, long
key)
{
   ftsample->FtSample::SetIn_PredTracklengths (member, key);
}
void FtSample_SumFtGrainAges (FtSample_c ftsample, Histogram_c sumHistogram)
{
   ftsample->FtSample::SumFtGrainAges (sumHistogram);
}
void FtSample_SumObsTracklengthDists (FtSample_c ftsample, Histogram_c
sumHistogram)
{
   ftsample->FtSample::SumObsTracklengthDists (sumHistogram);
}
void FtSample_SumPredTracklengthDists (FtSample_c ftsample, Histogram_c
sumHistogram)
{
   ftsample->FtSample::SumPredTracklengthDists (sumHistogram);
}
double equivalentTime (double temperature, double h)
{
   return FtSample::equivalentTime (temperature, h);
}
double hBinIndex (double h, int binnum, double L0, double c0, double c1)
{
   return FtSample::hBinIndex (h, binnum, L0, c0, c1);
}
void hBinPrevious (int ix, int binnum, double L0, double c0, double c1, double
                   temperature, double timestep, double *hPrev, int *ixPrev, double
*tPrev)
{
   FtSample::hBinPrevious (ix, binnum, L0, c0, c1, temperature, timestep,
                           hPrev, ixPrev, tPrev);
}
double hValue (int ix, double L0, double c0, double c1)
{
   return FtSample::hValue (ix, L0, c0, c1);
}
double prob (double z)
{
   return FtSample::prob (z);
}
///////////////////////////////////////////////////////////////////////////////
//                      Member function Implementation                       //
///////////////////////////////////////////////////////////////////////////////


void FtSample::Add_FtGrainAges (Histogram_c member)
{
   FtSample_c ftsample = this;

   DaListAppendItem ((ftsample->FtGrainAges), (AnyPntr_t) member);

}


void FtSample::Add_ObsTracklengths (Histogram_c member)
{
   FtSample_c ftsample = this;

   DaListAppendItem ((ftsample->ObsTracklengths), (AnyPntr_t) member);

}


void FtSample::Add_PredTracklengths (Histogram_c member)
{
   FtSample_c ftsample = this;

   DaListAppendItem ((ftsample->PredTracklengths), (AnyPntr_t) member);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            BinFtGrainAge
 ~ ACCESS:          Public
 ~ FUNCTION:        Bin the grain ages of the sample
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ grainAge              float        I    fission track age of a grain
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::BinFtGrainAge (float ClPerc, float grainAge)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   int ClIndex;

   ClIndex = FtSample::IndexFromClPerc (ClPerc);

   if (ClIndex < 0 || ClIndex >= FTBINNUMCLWGHTPERCDISTR)
   {
      showerror ("FtSample: BinObsTracklengthss - Cl Weigth Percentage %7.2f out "
		 "of range", ClPerc);
   }
   else
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), ClIndex);
      if (!histogram)
      {
         showerror ("FtSample: BinFtGrainAge - No grain age distribution for"
		    " Cl Percentage %7.2f",
                    ClPerc);
      }
      else
      {
         if (!histogram->FindAndIncrementBin (grainAge))
         {
            showerror ("FtSample: BinFtGrainAge - GrainAge %7.2f out of range",
		       grainAge);
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            BinObsTracklengths 
 ~ ACCESS:          Public
 ~ FUNCTION:        Bin the observed fission track lengths
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracklength           float        I    Track length of oberved track
 ~ ClPerc                float        I    Cl Weight percentage of the grain in which 
 ~                                         tracklength was measured 
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::BinObsTracklengths (float tracklength, float ClPerc)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int ClIndex;
   Histogram_c histogram;

   // Calculate index in array of observed histograms of this Cl weigth percentage
   ClIndex = FtSample::IndexFromClPerc (ClPerc);

   if (ClIndex < 0 || ClIndex >= FTBINNUMCLWGHTPERCDISTR)
   {
      showerror ("FtSample: BinObsTracklengthss - ClPerc %7.2f out of range", ClPerc);
   }
   else
   {
      // Get histogram from ClIndex, and increment the bincount in which tracklength lies

      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), ClIndex);
      if (!histogram)
      {
         showerror ("FtSample: BinObsTracklengthss - histogram %d not found", ClIndex);
      }
      else if (!histogram->FindAndIncrementBin (tracklength))
      {
         // showerror("FtSample: BinObsTracklengths - tracklength %7.2f out of range",tracklength);
         // discard the track, it must have been an erroneous measurement 
         // (according to geotrack)
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtLengthChi2
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculates the Chi squared of the summed predicted and observed 
 ~                  track length distributions, used for calibration
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ Chi2                  float*       O    Chi squared    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::CalcFtLengthChi2 (float *Chi2)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c predhist, obshist;
   int i;

   *Chi2 = .0;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      obshist = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      predhist = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (predhist && obshist)
      {
         *Chi2 += Histogram::CalcChi2 (predhist, obshist);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcPredFtAge
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculates the predicted fission track age of a sample from the 
 ~                  predicted track length distribution
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ PredFtAge             float*       O    predicted fission track age
 ~ 
 ~ RETURN VALUE: BOOLEAN_t -> TRUE on succes, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t FtSample::CalcPredFtAge (float *PredFtAge)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   double L, L0, bincount;
   double cumDensity = 0.;
   double histnum = 0.;
   double depoAge;
   int i, j;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         L0 = 16.0;             // this is what geotrack does
         L = FTSTARTLENGTHDISTR + 0.5 * FTSTEPLENGTHDISTR;

         for (j = 0; j < ((histogram)->BINNUM); j++)
         {
            bincount = histogram->BinCount (j);
            cumDensity += FtSample::DensityFromR (L / L0) * bincount;

            L += FTSTEPLENGTHDISTR;
         }
         histnum++;
      }
   }
   if (histnum > .0)
   {
      // calculate the predicted age

      // The total number of generated tracks == histnum*Depositional Age of the sample
      // Depositional age is therefore divided out of the equation.

      *PredFtAge = FTRENORMALIZATIONFACTOR * cumDensity / histnum;

      return (TRUE);
   }
   return (FALSE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcPredTracklengths
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculates the fission track length distribution of the sample 
 ~                  through time as a function of the temperature of a specific Cl
 ~                  weight percentage, defined by the index in the histogram array
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ timestep              double       I    time step
 ~ temperature           double       I    temperature
 ~ ClIndex               int          I    index of histogram 
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::CalcPredTracklengths (double timestep, double temperature, int ClIndex)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   double hBeg, hEnd;           // The original h value of the start and the end of a bin
   int iBeg, iEnd;              // Index of the bin for hBeg and hEnd
   double tBeg, tEnd;           // The original equivalent time of the start and end of a bin
   double teq, factor;
   double c0, c1, L0;
   int binnum, i;
   double *newCount;
   Histogram_c histogram;

   histogram = (Histogram_c) DaListValue (
                                            ((ftsample)->PredTracklengths), ClIndex);

   if (histogram)
   {
      // Cl weigth percentage is present in the sample

      ((ftsample)->TMPBINCNT)->Reset (.0);
      newCount = ((((ftsample)->TMPBINCNT))->VALUE);

      // Get the Cl weigth percentage dependent parameters
      c0 = ((ftsample)->AFTCALC)->GetC0 (ClIndex);
      c1 = ((ftsample)->AFTCALC)->GetC1 (ClIndex);
      L0 = ((ftsample)->AFTCALC)->GetL0 (ClIndex);
      //binnum = ( histogram ->BINNUM);
      histogram->FindBin (L0, &binnum);

      // Get the h, index and equivalent time of the start of the first bin at time t-timestep
      FtSample::hBinPrevious (0, binnum, L0, c0, c1, temperature, timestep,
                              &hBeg, &iBeg, &tBeg);

      // loop over all bins in the histogram
      for (i = 0; i < binnum; i++)
      {

         // Get the h, index and equivalent time of the end  of the current bin at 
         // time t-timestep
         FtSample::hBinPrevious (i + 1, binnum, L0, c0, c1, temperature, timestep,
                                 &hEnd, &iEnd, &tEnd);

         if (tBeg < .0)
         {
            if (tEnd < .0)
            {
               // hBeg and hEnd are both in a bin

               if (iEnd == iBeg)  // hBeg and hEnd fall in the same bin 
               {

                  if (iBeg == binnum - 1)  // take the whole bin
                  {
                     factor = 1.;
                  }
                  else          // take fractional part of the bin
                  {
                     factor = (hEnd - hBeg) /
                           (FtSample::hValue (iBeg + 1, L0, c0, c1) -
                            FtSample::hValue (iBeg, L0, c0, c1));
                  }
                  newCount[i] += factor * histogram->BinCount (iBeg);
               }
               else             // hBeg and hEnd fall in different bins 
               {
                  // take fractional part of first bin 
                  factor = (FtSample::hValue (iBeg + 1, L0, c0, c1) - hBeg) /
                        (FtSample::hValue (iBeg + 1, L0, c0, c1) -
                         FtSample::hValue (iBeg, L0, c0, c1));
                  newCount[i] += factor * histogram->BinCount (iBeg);

                  // last bin 
                  if (iEnd == binnum - 1)  // take the whole bin
                  {
                     factor = 1.;
                  }
                  else          // take fractional part of the bin
                  {
                     factor = (hEnd - FtSample::hValue (iEnd, L0, c0, c1)) /
                           (FtSample::hValue (iEnd + 1, L0, c0, c1) -
                            FtSample::hValue (iEnd, L0, c0, c1));
                  }
                  newCount[i] += factor * histogram->BinCount (iEnd);

                  // take whole of in between bins
                  newCount[i] += histogram->SumBins (iBeg + 1, iEnd - 1);
               }
            }
            else                // hBeg within h-range,  hEnd outside h-range 
            {
               // first bin 
               if (iBeg == binnum - 1)  // take the whole bin
               {
                  factor = 1.;
               }
               else             // take fractional part of the bin
               {
                  factor = (FtSample::hValue (iBeg + 1, L0, c0, c1) - hBeg) /
                        (FtSample::hValue (iBeg + 1, L0, c0, c1) -
                         FtSample::hValue (iBeg, L0, c0, c1));
               }
               newCount[i] += factor * histogram->BinCount (iBeg);

               // take whole of in between bins
               if (iBeg < binnum - 1)
                  newCount[i] += histogram->SumBins (iBeg + 1, binnum - 1);

               // part that is outside bins number of new tracks is "evenredig" with time

               // The total number of generated tracks per composition is 1 per Ma, 
               // therefore the total number of generated tracks is
               // number of compositions*the depositional age of the sample.
               // This assumption is used for the calculation of the predicted fission
               // track age, so don't change this! 
               newCount[i] += tEnd / SECS_IN_MA;
            }
         }
         else                   // hBeg and hEnd both outside h-range 
         {
            // number of new tracks is "evenredig" with time 

            // The total number of generated tracks per composition is 1 per Ma, 
            // therefore the total number of generated tracks is
            // number of compositions*the depositional age of the sample.
            // This assumption is used for the calculation of the predicted fission
            // track age, so don't change this! 
            newCount[i] += (tEnd - tBeg) / SECS_IN_MA;
         }

         // set start of bin to start of the next bin
         hBeg = hEnd;
         tBeg = tEnd;
         iBeg = iEnd;
      }
      // Copy the new counts into the track length distribution
      histogram->CopyBin (newCount);
   }

}


///////////////////////////////////////////////////////////////////////////////
//                          Create object instance.                          //
///////////////////////////////////////////////////////////////////////////////

FtSample_c FtSample::Create (BOOLEAN_t alloc)
{
   FtSample_c ftsample = (FtSample_c) 0;


   if (!(ftsample = new FtSample_t))
      return (FtSample_c) 0;

   return ((FtSample_c) ftsample);
}


///////////////////////////////////////////////////////////////////////////////
//                Delete object instance and any dependents.                 //
///////////////////////////////////////////////////////////////////////////////

void FtSample::Delete (void)
{
   FtSample_c ftsample = this;

   delete ftsample;
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            DensityFromR
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the track density from r (used for predicted fission
 ~                  track age calculation
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ r                     double       I    r value (L/L0)
 ~ 
 ~ RETURN VALUE: double -> density
 ~ ============================================================================


*******************************************************************************/

double FtSample::DensityFromR (double r)
{
// Source file:RBYaftcalc.def                                                //

   double density;

   if (r > .65)
      density = r;
   else if (r > 0.45 && r <= 0.65)
      density = 3.25 * (r - 0.45);
   else                         /* r <= .45 */
      density = .0;

   return (density);

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::ForAll_FtGrainAges (ActionMethod Action, long *arg)
{
   FtSample_c ftsample = this;

   {
      ITERATOR_t it;
      Histogram_c member;

      it = ContainerInitNextItem ((Container_c) (ftsample->FtGrainAges));
      while (member = (Histogram_c) Next (it))
         Action ((AnyPntr_t) member, arg);
      DaListIteratorClear (&it);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::ForAll_ObsTracklengths (ActionMethod Action, long *arg)
{
   FtSample_c ftsample = this;

   {
      ITERATOR_t it;
      Histogram_c member;

      it = ContainerInitNextItem ((Container_c) (ftsample->ObsTracklengths));
      while (member = (Histogram_c) Next (it))
         Action ((AnyPntr_t) member, arg);
      DaListIteratorClear (&it);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::ForAll_PredTracklengths (ActionMethod Action, long *arg)
{
   FtSample_c ftsample = this;

   {
      ITERATOR_t it;
      Histogram_c member;

      it = ContainerInitNextItem ((Container_c) (ftsample->PredTracklengths));
      while (member = (Histogram_c) Next (it))
         Action ((AnyPntr_t) member, arg);
      DaListIteratorClear (&it);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

BOOLEAN_t FtSample::FtGrainAges_GtFunc (Histogram_c histogram1, Histogram_c histogram2)
{

   return (TRUE);

}


///////////////////////////////////////////////////////////////////////////////
//                 Create object instance (C++ constructor).                 //
///////////////////////////////////////////////////////////////////////////////

FtSample::FtSample (BOOLEAN_t alloc)
{
   FtSample_c ftsample = this;

   ftsample->AFTCALC = AftCalc::GetCurrent();
   ftsample->SAMPLECODE = 0;
   if (!(ftsample->TMPBINCNT = PropDoubleVector::Create (alloc)))
   {
      ftsample->Delete ();
      return;
   }
   if (!(ftsample->ObsTracklengths = DaListCreate (FTBINNUMCLWGHTPERCDISTR)))
   {
      ftsample->Delete ();
      return;
   }
   DaListSetDestroyMethod (ftsample->ObsTracklengths, (DestroyMethod) Histogram_Delete);
   if (!(ftsample->PredTracklengths = DaListCreate (FTBINNUMCLWGHTPERCDISTR)))
   {
      ftsample->Delete ();
      return;
   }
   DaListSetDestroyMethod (ftsample->PredTracklengths, (DestroyMethod) Histogram_Delete);
   if (!(ftsample->GRAINFTAGEDIST = Histogram::Create (alloc)))
   {
      ftsample->Delete ();
      return;
   }
   if (!(ftsample->FtGrainAges = DaListCreate (FTBINNUMCLWGHTPERCDISTR)))
   {
      ftsample->Delete ();
      return;
   }
   DaListSetDestroyMethod (ftsample->FtGrainAges, (DestroyMethod) Histogram_Delete);
   ftsample->Init ();
   if (ftsample->AFTCALC && (alloc != 3))
      AftCalc_Add_FtSamples (ftsample->AFTCALC, ftsample);
}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::GeotrackCorrPredTracklengthDists (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int i, j, ClIndex, binnum;
   Histogram_c histogram, partdist, sum;
   double L0, tracklength, tl, sigma;
   double p, nextp, a, b, epx, bjval;
   PropDoubleVector_c bj;

   partdist = Histogram::Make ();
   sum = Histogram::Make ();
   bj = PropDoubleVector::Make ();

   for (ClIndex = 0; ClIndex < FTBINNUMCLWGHTPERCDISTR; ClIndex++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), ClIndex);
      if (histogram)
      {
         binnum = ((histogram)->BINNUM);
         partdist->Initialize (FTSTARTLENGTHDISTR, FTSTEPLENGTHDISTR, binnum);
         sum->Initialize (FTSTARTLENGTHDISTR, FTSTEPLENGTHDISTR, binnum);

         L0 = ((ftsample)->AFTCALC)->GetL0 (ClIndex);

         bj->Initialize (binnum);
         for (j = 0; j < binnum; j++)
         {
            a = -4.5;
            b = .65;
            if (j == binnum - 1)
               tracklength = (FTSTARTLENGTHDISTR + (double) j *
                              FTSTEPLENGTHDISTR + L0) / 2.;
            else
               tracklength = FTSTARTLENGTHDISTR + ((float) j + .5) * FTSTEPLENGTHDISTR;

            epx = exp (a + (b * tracklength));
            bj->SetValue (j, tracklength * epx / (1. + epx));
         }

         for (i = 0; i < binnum; i++)
         {
            if (i == binnum - 1)
               tracklength = (FTSTARTLENGTHDISTR + (double) i *
                              FTSTEPLENGTHDISTR + L0) / 2.;
            else
               tracklength = FTSTARTLENGTHDISTR + ((float) i + .5) * FTSTEPLENGTHDISTR;

            if (tracklength > 7.2)
            {
               if (tracklength > 11.)
                  sigma = tracklength * (-.0325) + 1.34;
               else
                  sigma = tracklength * (-.54) + 6.9;

               p = FtSample::prob (-tracklength / sigma);
               tl = FTSTARTLENGTHDISTR;
               for (j = 0; j < binnum; j++)
               {
                  tl += FTSTEPLENGTHDISTR;
                  nextp = FtSample::prob ((tl - tracklength) / sigma);
                  partdist->SetBinCount (j, p - nextp);
                  p = nextp;
               }

               // normalise partial distributions
               partdist->Scale (1);

               for (j = 0; j < binnum; j++)
               {
                  bj->GetValue (j, &bjval);
                  partdist->MulBinCount (j, 1. / bjval);
               }

               partdist->Scale (1);

               // debias and sum partial distribution

               for (j = 0; j < binnum; j++)
               {
                  sum->SumBinCount (j, partdist->BinCount (j) *
                                    histogram->BinCount (i));
               }
            }
         }

         // rebias final length distribution

         for (j = 0; j < binnum; j++)
         {
            bj->GetValue (j, &bjval);
            sum->MulBinCount (j, bjval);
         }

         sum->CopyContentsTo (histogram);
      }
   }
   partdist->Delete ();
   sum->Delete ();
   bj->Delete ();
}


/*******************************************************************************
Undocumented.
*******************************************************************************/

float FtSample::GetClContFraction (int ClBin, FtGrainIoTbl_c FtGrainIoTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c row;
   int grainNumSample, grainNumClbin;
   float ClPerc;
   float fraction;

   grainNumSample = 0;
   grainNumClbin = 0;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (row = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (((ftsample)->SAMPLECODE),
                   FtGrainIoTblRow_FtSampleId (row)))
      {
         grainNumSample++;

         ClPerc = FtGrainIoTblRow_FtClWeightPerc (row);
         if (ClPerc >= FTSTARTCLWGHTPERCDISTR + (float) ClBin *
             FTSTEPCLWGHTPERCDISTR &&
             ClPerc < FTSTARTCLWGHTPERCDISTR + (float) (ClBin + 1) * FTSTEPCLWGHTPERCDISTR)
         {
            grainNumClbin++;
         }
      }
   }
   if (grainNumSample == 0)
      fraction = 0.;
   else
      fraction = (float) grainNumClbin / (float) grainNumSample;

   return (fraction);

}


///////////////////////////////////////////////////////////////////////////////
//                    Retrieve element from set by index                     //
///////////////////////////////////////////////////////////////////////////////

void FtSample::GetFrom_FtGrainAges (Histogram_c * member, long key)
{
   FtSample_c ftsample = this;

   DaListGetItem ((ftsample->FtGrainAges), (int) key, (AnyPntr_t *) member);

}


///////////////////////////////////////////////////////////////////////////////
//                    Retrieve element from set by index                     //
///////////////////////////////////////////////////////////////////////////////

void FtSample::GetFrom_ObsTracklengths (Histogram_c * member, long key)
{
   FtSample_c ftsample = this;

   DaListGetItem ((ftsample->ObsTracklengths), (int) key, (AnyPntr_t *) member);

}


///////////////////////////////////////////////////////////////////////////////
//                    Retrieve element from set by index                     //
///////////////////////////////////////////////////////////////////////////////

void FtSample::GetFrom_PredTracklengths (Histogram_c * member, long key)
{
   FtSample_c ftsample = this;

   DaListGetItem ((ftsample->PredTracklengths), (int) key, (AnyPntr_t *) member);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetGrainAgeDist
 ~ ACCESS:          Public
 ~ FUNCTION:        Returns the grain age distribution in the HistogramTbl 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type           I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ HistogramTbl          HistogramTbl_c  O    Table for the grain age distribution
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::GetGrainAgeDist (int ChlorideIx, HistogramTbl_c HistogramTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;

   histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), ChlorideIx);
   if (histogram)
   {
      histogram->FillHistogramTbl (HistogramTbl);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::GetMaxBinCount (double *MaxTracknum, double *MaxGrainnum)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   int i;

   *MaxTracknum = .0;
   *MaxGrainnum = .0;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      if (histogram)
      {
         *MaxTracknum = MAX (*MaxTracknum, histogram->GetMaxBinCount ());
      }
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         *MaxTracknum = MAX (*MaxTracknum, histogram->GetMaxBinCount ());
      }
      histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), i);
      if (histogram)
      {
         *MaxGrainnum = MAX (*MaxGrainnum, histogram->GetMaxBinCount ());
      }

   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetObsTracklengthDist
 ~ ACCESS:          Public
 ~ FUNCTION:        Returns the summed observed tracklength distribution in the HistogramTbl 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type           I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ HistogramTbl          HistogramTbl_c  O    Table for the observed tracklength
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::GetObsTracklengthDist (int ChlorideIx, HistogramTbl_c HistogramTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;

   histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), ChlorideIx);
   if (histogram)
   {
      histogram->FillHistogramTbl (HistogramTbl);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetPredTracklengthDist
 ~ ACCESS:          Public
 ~ FUNCTION:        Returns the summed predicted tracklength distribution in the HistogramTbl 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type           I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ HistogramTbl          HistogramTbl_c  O    Table for the predicted tracklength
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::GetPredTracklengthDist (int ChlorideIx, HistogramTbl_c HistogramTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;

   histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), ChlorideIx);
   if (histogram)
   {
      histogram->FillHistogramTbl (HistogramTbl);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::GetSumGrainAgeDist (HistogramTbl_c HistogramTbl, double *MaxBinCnt)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram = Histogram::Make ();;

   ftsample->SumFtGrainAges (histogram);

   histogram->FillHistogramTbl (HistogramTbl);
   *MaxBinCnt = histogram->GetMaxBinCount ();

   histogram->Delete ();

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::GetSumObsTracklengthDist (HistogramTbl_c HistogramTbl, double *MaxBinCnt)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram = Histogram::Make ();

   ftsample->SumObsTracklengthDists (histogram);

   histogram->FillHistogramTbl (HistogramTbl);
   *MaxBinCnt = histogram->GetMaxBinCount ();

   histogram->Delete ();

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void FtSample::GetSumPredTracklengthDist (HistogramTbl_c HistogramTbl, double *MaxBinCnt)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram = Histogram::Make ();

   ftsample->SumPredTracklengthDists (histogram);

   histogram->FillHistogramTbl (HistogramTbl);
   *MaxBinCnt = histogram->GetMaxBinCount ();

   histogram->Delete ();

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetTemperature
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the current temperature of the sample
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ temperature           double*      I    temperature
 ~ 
 ~ RETURN VALUE: BOOLEAN_t -> TRUE on succes, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t FtSample::GetTemperature (double *temperature)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   CalcEngine_c calcengine = ((((ftsample)->AFTCALC))->CALCENGINE);
   GLGmanager_c glgmanager = CalcEngine_GLGMANAGER (calcengine);
   Tempcalc_c tempcalc = ((calcengine)->TEMPCALC);
   double *nodeStartTemp = tempcalc->NodeStartTemp ();
   double *nodeEndTemp = tempcalc->NodeEndTemp ();
   int topNodeId = ((ftsample)->TOPNODEID);
   int botNodeId = ((ftsample)->BOTTOMNODEID);
   float factor = ((ftsample)->LENGTHFACTOR);
   double startTemp, endTemp;

   if (!Ibscfuncs::DoubleEqual (nodeStartTemp[topNodeId], PROPSYS_DOUBLE_NULL) &&
       !Ibscfuncs::DoubleEqual (nodeEndTemp[topNodeId], PROPSYS_DOUBLE_NULL) &&
       !Ibscfuncs::DoubleEqual (nodeStartTemp[botNodeId], PROPSYS_DOUBLE_NULL) &&
       !Ibscfuncs::DoubleEqual (nodeEndTemp[botNodeId], PROPSYS_DOUBLE_NULL))
   {
      // Calculate the start and end temperature of the sample by interpolating the 
      // start and end tempertures of the nodes just above and below the sample

      startTemp = nodeStartTemp[topNodeId] +
            factor * (nodeStartTemp[botNodeId] - nodeStartTemp[topNodeId]);
      endTemp = nodeEndTemp[topNodeId] +
            factor * (nodeEndTemp[botNodeId] - nodeEndTemp[topNodeId]);

      // take the average temperature during the timestep, set in K
      *temperature = (startTemp + endTemp) / 2. + ABSOLUTEZERO;

      return (TRUE);
   }
   return (FALSE);

}

int FtSample::IndexFromClPerc (float ClPerc)
{
// Source file:RBYaftcalc.def                                                //

   return (int ((ClPerc - FTSTARTCLWGHTPERCDISTR) / FTSTEPCLWGHTPERCDISTR));

}


///////////////////////////////////////////////////////////////////////////////
//                        Init Object initialization.                        //
///////////////////////////////////////////////////////////////////////////////

void FtSample::Init (void)
{
   FtSample_c ftsample = this;

   (ftsample->SAMPLECODE) = 0;
   (ftsample->DEPTH) = -9999.000000;
   (ftsample->DEPOAGE) = -9999.000000;
   (ftsample->TOPNODEID) = -9999;
   (ftsample->BOTTOMNODEID) = -9999;
   (ftsample->LENGTHFACTOR) = -9999.000000;
   (ftsample->SAMPLELOCATIONSET) = FALSE;

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Initialize
 ~ ACCESS:          Public
 ~ FUNCTION:        Initialize FtSample object
  ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ sampleId            STRING             I    sample identifier
 ~ depth               float              I    sample depth
 ~ FtGrainIoTbl        FtGrainIoTbl_c     I    Fission track grain data
 ~ StratIoTbl          StratIoTbl_c       I    StratIoTbl, used to set location (depth) of samples
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::Initialize (STRING sampleId, float depth, FtGrainIoTbl_c
                           FtGrainIoTbl, FtLengthIoTbl_c FtLengthIoTbl,
                           StratIoTbl_c StratIoTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   STR_ASS (((ftsample)->SAMPLECODE), sampleId);
   ((ftsample)->DEPTH) = depth;

   // Identify  the location (depth) of the sample so that the temperature of the 
   // sample through time can be traced 
   ftsample->SetFtSampleLocation (StratIoTbl);

   // Make the histograms for the observerd and predicted tracklengths
   ftsample->MakeDistributions (FtGrainIoTbl, FtLengthIoTbl);

   // Initialize the grain age histogram
   // Histogram_Initialize( ( ftsample ->GRAINFTAGEDIST), FTSTARTGRAINAGEDISTR,
   //                      FTSTEPGRAINAGEDISTR, FTBINNUMGRAINAGEDISTR);

}


///////////////////////////////////////////////////////////////////////////////
//                       Create and initialize object.                       //
///////////////////////////////////////////////////////////////////////////////

FtSample_c FtSample::Make (void)
{
   FtSample_c ftsample = (FtSample_c) 0;

   BOOLEAN_t alloc = FALSE;

   if (!(ftsample = FtSample::Create (alloc)))
      return (FtSample_c) 0;

   return ((FtSample_c) ftsample);
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            MakeDistributions
 ~ ACCESS:          Private
 ~ FUNCTION:        Make track length histograms for all Cl weigth percentages
 ~                  in the sample, both observed and predicted
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl        FtGrainIoTbl_c     I    Fission track grain data
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::MakeDistributions (FtGrainIoTbl_c FtGrainIoTbl, FtLengthIoTbl_c FtLengthIoTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int ClIndex;
   Histogram_c histogram;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (((ftsample)->SAMPLECODE),
                   FtGrainIoTblRow_FtSampleId (gRow))  /* &&
                                                          FtGrainHasMeasuredFtLengths(gRow,FtLengthIoTbl)  */ )
      {
         // Calculate the index in the array of histograms for the Cl weight perc.
         ClIndex = FtSample::IndexFromClPerc (FtGrainIoTblRow_FtClWeightPerc (gRow));

         if (ClIndex >= 0 && ClIndex < FTBINNUMCLWGHTPERCDISTR)
         {
            histogram = (Histogram_c) DaListValue
                                                   (((ftsample)->ObsTracklengths), ClIndex);

            if (!histogram)
            {
               /* make observed tracklength distribution for this Cl weight perc. */

               histogram = Histogram::Make ();
               histogram->Initialize (FTSTARTLENGTHDISTR,
                                      FTSTEPLENGTHDISTR, FTBINNUMLENGTHDISTR);
               DaListSetItem (((ftsample)->ObsTracklengths),
                              ClIndex, (AnyPntr_t) histogram);
            }

            histogram = (Histogram_c) DaListValue
                                                   (((ftsample)->PredTracklengths), ClIndex);

            if (!histogram)
            {
               /* make predicted tracklength distribution for this Cl weight perc. */

               histogram = Histogram::Make ();
               histogram->Initialize (FTSTARTLENGTHDISTR,
                                      FTSTEPLENGTHDISTR, FTBINNUMLENGTHDISTR);
               DaListSetItem (((ftsample)->PredTracklengths),
                              ClIndex, (AnyPntr_t) histogram);
            }

            histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), ClIndex);

            if (!histogram)
            {
               /* make grain age distribution for this Cl weight perc. */

               histogram = Histogram::Make ();
               histogram->Initialize (FTSTARTGRAINAGEDISTR,
                                      FTSTEPGRAINAGEDISTR, FTBINNUMGRAINAGEDISTR);
               DaListSetItem (((ftsample)->FtGrainAges),
                              ClIndex, (AnyPntr_t) histogram);
            }

         }
      }
   }

   // Initialize the TMPBINCNT vector which is used for the calculation of the predicted
   // tracklenghts. 
   ((ftsample)->TMPBINCNT)->Initialize (FTBINNUMLENGTHDISTR);

}


void FtSample::MergeTracklengthDists (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int i;
   Histogram_c histogram;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         histogram->MergeBins (FTFACTOR);
      }
      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      if (histogram)
      {
         histogram->MergeBins (FTFACTOR);
      }
   }

}


BOOLEAN_t FtSample::ObsTracklengths_GtFunc (Histogram_c histogram1,
                                            Histogram_c histogram2)
{

   return (TRUE);

}


BOOLEAN_t FtSample::PredTracklengths_GtFunc (Histogram_c histogram1,
                                             Histogram_c histogram2)
{

   return (TRUE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Print
 ~ ACCESS:          Private
 ~ FUNCTION:        Print all FtSample data
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ dummy                 long*             dummy argument
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::Print (long *arg)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   printf ("\n\n");
   printf ("Samplecode:     %12s\n", ((ftsample)->SAMPLECODE));
   printf ("Depth:          %12.2f\n", ((ftsample)->DEPTH));
   printf ("TopNodeId:      %12d\n", ((ftsample)->TOPNODEID));
   printf ("BotNodeId:      %12d\n", ((ftsample)->BOTTOMNODEID));
   printf ("Length factor:  %12.2f\n", ((ftsample)->LENGTHFACTOR));
   printf ("DepoAge:        %12.2f\n\n", ((ftsample)->DEPOAGE));

   printf ("\n[Predicted Tracklengths distribution]\n\n");
   ftsample->PrintPredTracklengths ();

   printf ("\n\n[Observed Tracklengths distribution]\n\n");
   ftsample->PrintObsTracklengths ();

   printf ("\n\n[Observed grain age distribution]\n\n");
   ftsample->PrintFtGrainAges ();

   printf ("\n\n");


}


void FtSample::PrintFtGrainAges (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   float ClPerc;
   int i;
   BOOLEAN_t first = TRUE;

   ClPerc = FTSTARTCLWGHTPERCDISTR;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), i);
      if (histogram)
      {
         if (first)
         {
            printf ("     ");
            histogram->PrintBinHeader ();
            first = FALSE;
         }
         printf ("\n%3.1f  ", ClPerc);
         histogram->PrintBins ();
      }
      ClPerc += FTSTEPCLWGHTPERCDISTR;
   }

   printf ("\n\n");

   histogram = Histogram::Make ();
   ftsample->SumFtGrainAges (histogram);
   printf ("Sum: ");
   histogram->PrintBins ();
   printf ("   %8.2f", histogram->SumBins (0, ((histogram)->BINNUM) - 1));


   printf ("\n\n");
   histogram->Delete ();

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            PrintObsTracklengths
 ~ ACCESS:          Private
 ~ FUNCTION:        Print the observed track length distributions
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~                                    I    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::PrintObsTracklengths (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram = NULL;
   float ClPerc;
   int i;
   BOOLEAN_t first = TRUE;

   ClPerc = FTSTARTCLWGHTPERCDISTR;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      if (histogram)
      {
         if (first)
         {
            printf ("     ");
            histogram->PrintBinHeader ();
            first = FALSE;
         }
         printf ("\n%3.1f  ", ClPerc);
         histogram->PrintBins ();
      }
      ClPerc += FTSTEPCLWGHTPERCDISTR;
   }

   printf ("\n\n");

   histogram = Histogram::Make ();
   ftsample->SumObsTracklengthDists (histogram);

   printf ("Sum: ");
   histogram->PrintBins ();
   printf ("\n\n");
   histogram->Delete ();

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            PrintPredTracklengths
 ~ ACCESS:          Private
 ~ FUNCTION:        Print the predicted track length distributions
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~                                    I    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::PrintPredTracklengths (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   float ClPerc;
   int i;
   BOOLEAN_t first = TRUE;

   ClPerc = FTSTARTCLWGHTPERCDISTR;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         if (first)
         {
            printf ("     ");
            histogram->PrintBinHeader ();
            first = FALSE;
         }
         printf ("\n%3.1f  ", ClPerc);
         histogram->PrintBins ();

         printf (" %6.2f", histogram->GetMean ());
         printf (" %6.2f", histogram->SumBins (0, ((histogram)->BINNUM) - 1));

      }
      ClPerc += FTSTEPCLWGHTPERCDISTR;
   }

   printf ("\n\n");

   histogram = Histogram::Make ();
   ftsample->SumPredTracklengthDists (histogram);
   printf ("Sum: ");
   histogram->PrintBins ();
   printf ("   %8.2f", histogram->SumBins (0, ((histogram)->BINNUM) - 1));


   printf ("\n\n");
   histogram->Delete ();

}


void FtSample::Remove_FtGrainAges (Histogram_c member)
{
   FtSample_c ftsample = this;

   int key;

   if ((key = DaListFindItem ((ftsample->FtGrainAges), (AnyPntr_t) member)) != -1)
   {
      DaListRemItem ((ftsample->FtGrainAges), key, FALSE);
   }

}


void FtSample::Remove_ObsTracklengths (Histogram_c member)
{
   FtSample_c ftsample = this;

   int key;

   if ((key = DaListFindItem ((ftsample->ObsTracklengths), (AnyPntr_t)
       member)) != -1)
   {
      DaListRemItem ((ftsample->ObsTracklengths), key, FALSE);
   }

}


void FtSample::Remove_PredTracklengths (Histogram_c member)
{
   FtSample_c ftsample = this;

   int key;

   if ((key = DaListFindItem ((ftsample->PredTracklengths), (AnyPntr_t)
       member)) != -1)
   {
      DaListRemItem ((ftsample->PredTracklengths), key, FALSE);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScaleObsTracklengthDists
 ~ ACCESS:          Public
 ~ FUNCTION:        Scale the observed length distributions to contain a total number
 ~                  of tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracknum              int          I    Number of tracks the histogram will contain
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::ScaleObsTracklengthDists (int tracknum)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   int i;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      if (histogram)
      {
         histogram->Scale (tracknum);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScalePredTracklenghtDists
 ~ ACCESS:          Public
 ~ FUNCTION:        Scale the predicted length distributions to contain a total number
 ~                  of tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracknum              int          I    Number of tracks the histogram will contain
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::ScalePredTracklenghtDists (int tracknum)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c histogram;
   int i;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         histogram->Scale (tracknum);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScalePredTracklengthsToObserved
 ~ ACCESS:          Public
 ~ FUNCTION:        Scale the predicted tracklength distributions so that each 
 ~                  distribution contains the same number of tracks as the 
 ~                  corresponding observed track length dist.
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracknum              int          I    Number of tracks the histogram will contain
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::ScalePredTracklengthsToObserved (void)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   Histogram_c predhist, obshist;
   double tracknum;
   int i;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      obshist = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      predhist = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (predhist && obshist)
      {
         tracknum = obshist->SumBins (0, ((obshist)->BINNUM) - 1);
         if (tracknum > 0)
         {
            predhist->Scale (tracknum);
         }
         else
         {
            predhist->Initialize (((predhist)->BINSTART),
                                  ((predhist)->BINWIDTH),
                                  ((predhist)->BINNUM));
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SetFtSampleLocation
 ~ ACCESS:          Private
 ~ FUNCTION:        Identify the sample location, so that the temperature of the 
 ~                  sample through time can be traced 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         This algorithm will probably change with current geology model
 ~ PARAMETERS:
 ~   
 ~ Name                Type             I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ StratIoTbl          StratIoTbl_c      I    StratIoTbl
 ~ 
 ~ RETURN VALUE: BOOLEAN_t -> TRUE on succes, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t FtSample::SetFtSampleLocation (StratIoTbl_c StratIoTbl)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   StratIoTblRow_c sRow;
   double sampleDepth;
   double topSurfdepth, botSurfdepth;
   double layerThickness;
   float sampleDepoage, topDepoage, botDepoage;
   double factor;
   int topNodeid, botNodeid;
   NeedleNodeIterator_c needleIt;
   Node_c node;
   Segment_c segment;
   CalcEngine_c calcengine = ((((ftsample)->AFTCALC))->CALCENGINE);
   GLGmanager_c glgmanager = CalcEngine_GLGMANAGER (calcengine);
   BOOLEAN_t bSegmentFound = TRUE;
   BOOLEAN_t bResult = TRUE;


   /* find the layer where the sample was taken from in the StratIoTbl */

   sampleDepth = ((ftsample)->DEPTH);

   it = StratIoTbl_IteratorInit_StratIoTblRows (StratIoTbl);
   while ((sRow = (StratIoTblRow_c) Next (it)) &&
          sampleDepth > StratIoTblRow_Depth (sRow))
   {
      // store depth and depoage of first surface above sample depth

      topSurfdepth = StratIoTblRow_Depth (sRow);
      topDepoage = StratIoTblRow_DepoAge (sRow);
   }

   if (!sRow)
   {
      showerror
                 ("FtSample: SetFtSampleLocation - Sample depth not in sediment range");
      bResult = FALSE;
   }
   else
   {
      // store depth and depoage of first surface below sample depth

      botSurfdepth = StratIoTblRow_Depth (sRow);
      botDepoage = StratIoTblRow_DepoAge (sRow);

      // calculate the depoAge of the sample, this is needed for the
      // calculation of the predicted fission track age

      layerThickness = botSurfdepth - topSurfdepth;
      if (Ibscfuncs::DoubleEqual (layerThickness, .0))
         sampleDepoage = topDepoage;
      else
         sampleDepoage = topDepoage + (botDepoage - topDepoage) *
               (sampleDepth - topSurfdepth) / layerThickness;

      // find the nodes just above and below the sample depth  
      needleIt = glgmanager->CreateNeedleNodeIterator ();
      if (node = needleIt->StartIteration (0, 0,
                                           GEOL_ITR_DOWN_DIRECTION,
                                           GEOL_ITR_SEDIMENTS_RANGE,
                                           FALSE, .0))
      {
         topNodeid = ((node)->IDENTIFIER);
         bSegmentFound = FALSE;
         while (!bSegmentFound)
         {
            if ((node = needleIt->next ()) &&
                (segment = Node_GetTopSegment (node)) &&
                (segment->HasFCThicknessHistory ()) &&
                (segment->GetStartDepoAge (botDepoage)) &&
                (sampleDepoage <= botDepoage))
            {
               bSegmentFound = TRUE;
            }
            else
            {
               topNodeid = ((node)->IDENTIFIER);
            }
         }
         if (bSegmentFound)
         {
            botNodeid = ((node)->IDENTIFIER);
            segment->GetEndDepoAge (topDepoage);
            if (Ibscfuncs::DoubleEqual (layerThickness, .0))
               factor = 0.;
            else
               factor = (sampleDepoage - topDepoage) / (botDepoage - topDepoage);

            // Set all attributes that define the sample location
            ((ftsample)->TOPNODEID) = topNodeid;
            ((ftsample)->BOTTOMNODEID) = botNodeid;
            ((ftsample)->LENGTHFACTOR) = factor;
            ((ftsample)->SAMPLELOCATIONSET) = TRUE;

            ((ftsample)->DEPOAGE) = sampleDepoage;
         }
         else
         {
            showerror ("FtSample: SetFtSampleLocation - Error in sample depth");
            bResult = FALSE;
         }
      }
      else
      {
         showerror ("FtSample: SetFtSampleLocation - Model not build");
         bResult = FALSE;
      }
      needleIt->Delete ();


   }
   return (bResult);

}


///////////////////////////////////////////////////////////////////////////////
//                     Insert element into set by index                      //
///////////////////////////////////////////////////////////////////////////////

void FtSample::SetIn_FtGrainAges (Histogram_c member, long key)
{
   FtSample_c ftsample = this;

   DaListSetItem ((ftsample->FtGrainAges), (int) key, (AnyPntr_t) member);

}


///////////////////////////////////////////////////////////////////////////////
//                     Insert element into set by index                      //
///////////////////////////////////////////////////////////////////////////////

void FtSample::SetIn_ObsTracklengths (Histogram_c member, long key)
{
   FtSample_c ftsample = this;

   DaListSetItem ((ftsample->ObsTracklengths), (int) key, (AnyPntr_t) member);

}


///////////////////////////////////////////////////////////////////////////////
//                     Insert element into set by index                      //
///////////////////////////////////////////////////////////////////////////////

void FtSample::SetIn_PredTracklengths (Histogram_c member, long key)
{
   FtSample_c ftsample = this;

   DaListSetItem ((ftsample->PredTracklengths), (int) key, (AnyPntr_t) member);

}


void FtSample::SumFtGrainAges (Histogram_c sumHistogram)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int i;
   Histogram_c histogram;
   BOOLEAN_t first = TRUE;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->FtGrainAges), i);
      if (histogram)
      {
         if (first)
         {
            sumHistogram->Initialize (((histogram)->BINSTART),
                                      ((histogram)->BINWIDTH), ((histogram)->BINNUM));
            first = FALSE;
         }
         sumHistogram->AddBins (histogram);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SumObsTracklengthDists
 ~ ACCESS:          Private
 ~ FUNCTION:        Sums the bins of the Cl weigth percentage dependent obsrved 
 ~                  tracklenth histograms 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ sumHistogram          Histogram_c  O    Summed histogram
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::SumObsTracklengthDists (Histogram_c sumHistogram)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int i;
   Histogram_c histogram;
   BOOLEAN_t first = TRUE;

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->ObsTracklengths), i);
      if (histogram)
      {
         if (first)
         {
            sumHistogram->Initialize (((histogram)->BINSTART),
                                      ((histogram)->BINWIDTH), ((histogram)->BINNUM));
            first = FALSE;
         }

         sumHistogram->AddBins (histogram);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SumPredTracklengthDists
 ~ ACCESS:          Private
 ~ FUNCTION:        Sums the bins of the Cl weigth percentage dependent predicted 
 ~                  tracklenth histograms
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ sumHistogram          Histogram_c  O    Summed histogram
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::SumPredTracklengthDists (Histogram_c sumHistogram)
{
   FtSample_c ftsample = this;

// Source file:RBYaftcalc.def                                                //

   int i;
   Histogram_c histogram;
   BOOLEAN_t first = TRUE;

   sumHistogram->Initialize (FTSTARTLENGTHDISTR,
                             FTSTEPLENGTHDISTR, FTBINNUMLENGTHDISTR);

   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      histogram = (Histogram_c) DaListValue (((ftsample)->PredTracklengths), i);
      if (histogram)
      {
         if (first)
         {
            sumHistogram->Initialize (((histogram)->BINSTART),
                                      ((histogram)->BINWIDTH), ((histogram)->BINNUM));
            first = FALSE;
         }

         sumHistogram->AddBins (histogram);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            equivalentTime
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the equivalent time as function of temperatur and h
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ temperature           double       I    temperature
 ~ h                     double       I    h
 ~ 
 ~ RETURN VALUE: double -> equivalent time
 ~ ============================================================================


*******************************************************************************/

double FtSample::equivalentTime (double temperature, double h)
{
// Source file:RBYaftcalc.def                                                //

   double a = (double) 1. / temperature - FTT0INV;
   double teq = FTt0 * exp (a * h);

   return (teq);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            hBinIndex
 ~ ACCESS:          Private
 ~ FUNCTION:        Find the index of the bin in which h falls
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ h                     double       I    h value to locate 
 ~ binnum                int          I    number of bins in histogram
 ~ c0                    double       I    Cl weight perc. dependent parameter
 ~ c1                    double       I    Cl weight perc. dependent parameter
~ 
 ~ RETURN VALUE: int -> index of bin in which h falls
 ~ ============================================================================


*******************************************************************************/

double FtSample::hBinIndex (double h, int binnum, double L0, double c0, double c1)
{
// Source file:RBYaftcalc.def                                                //

   int i;

   for (i = 1; i < binnum; i++)
   {
      if (h > FtSample::hValue (i, L0, c0, c1))
      {
         return (i - 1);
      }
   }
   return (binnum - 1);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            hBinPrevious
 ~ ACCESS:          Private
 ~ FUNCTION:        This routine calculates the h value, the equivalent time of this h
 ~                  and the index of the bin in which this h falls at time t-dt. 
 ~                  From the binindex ix the h value at time t is calculated. From this 
 ~                  h and the current temperature the equivalent time is calculated. 
 ~                  From this the hPrev, which is the h value at time t-dt 
 ~                  and the equivalent time of hPrev and current temperature is calculated
 ~                  Finally the bin index of hPrev is determined.
 ~ 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ ix                    int          I    index of bin
 ~ binnum                int          I    number of bins in histogram
 ~ c0                    double       I    Cl weight perc. dependent parameter
 ~ c1                    double       I    Cl weight perc. dependent parameter
 ~ temperature           double       I    temperature
 ~ hPrev                 double*      O    h value at time t-dt with temperture 
 ~ ixPrev                int*         O    index of bin in which hPrev lies 
 ~ tPrev                 double*      O    equivalent time of hPrev at temperature 
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void FtSample::hBinPrevious (int ix, int binnum, double L0, double c0, double
                             c1, double temperature, double timestep, double
                             *hPrev, int *ixPrev, double *tPrev)
{
// Source file:RBYaftcalc.def                                                //

   double teq;

   *ixPrev = -1;
   if (ix == binnum)
   {
      // equivalent time is zero ( l==L0 => r==1 => h== -inf)
      *tPrev = timestep;
   }
   else
   {
      teq = FtSample::equivalentTime (temperature, FtSample::hValue (ix, L0,
                                                                     c0, c1));
      *tPrev = timestep - teq;

      if (*tPrev < .0)
      {
         // previous equivalent time is in time range
         *hPrev = log ((teq - timestep) / (double) FTt0) /
               ((double) 1. / temperature - (double) FTT0INV);
         *ixPrev = FtSample::hBinIndex (*hPrev, binnum, L0, c0, c1);
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            hValue
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate h from r (where is calculated from i) of a specific
 ~                  composition (defined by c0 and c1)
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ i                     int          I    index of bin in histogram 
 ~ binnum                int          I    number of bins in histogram
 ~ c0                    double       I    Cl weight perc. dependent parameter
 ~ c1                    double       I    Cl weight perc. dependent parameter
 ~ 
 ~ RETURN VALUE: double -> h
 ~ ============================================================================


*******************************************************************************/

double FtSample::hValue (int ix, double L0, double c0, double c1)
{
// Source file:RBYaftcalc.def                                                //

   double r, h;

   r = (FTSTARTLENGTHDISTR + (float) ix * FTSTEPLENGTHDISTR) / L0;
   h = (double) ((log ((double) 1. - r * r * r) - c0) / c1);
   return (h);

}


double FtSample::prob (double z)
{
// Source file:RBYaftcalc.def                                                //

   double pdf, y, p;
   double z11, z12, z9, z7, z5, z3;
   double q, zabs;

   double bnp[] =
   {0., 0.398942280385, 3.805e-08, 1.00000615302, 3.98064784e-04, 1.98615381364,
    0.151679116635, 5.29330324926, 4.8385912808, 15.1508972451, 0.742380924027,
    30.789933034, 3.99019417011};
   double anp[] =
   {0., 0.5, 0.39894228044, 0.399903438504, 5.75885480458, 29.8213557808,
    2.62433121679, 48.6959930692, 5.928858724438};


   zabs = ABS (z);
   if (zabs > 12.7)
   {
      q = 0.;
      pdf = 0.;
      if (z < 0.)
      {
         p = q;
         q = 1 - p;
      }
      else
      {
         p = 1;
      }
   }
   else
   {
      y = anp[1] * z * z;
      pdf = exp (-y) * bnp[1];
      if (zabs > 1.28)
      {
         z11 = zabs + bnp[10];
         z12 = zabs + bnp[12];
         z9 = zabs + bnp[8];
         z7 = zabs - bnp[6];
         z5 = zabs + bnp[4];
         z3 = zabs - bnp[2];
         q = pdf / (z3 + bnp[3] / (z5 + bnp[5] / (z7 + bnp[7] / (z9 - bnp[9] /
                                                                 (z11 +
                                                                  bnp[11] / z12)))));
         if (z < 0)
         {
            p = q;
            q = 1 - p;
         }
         else
         {
            p = 1 - q;
         }
      }
      else
      {
         q = anp[1] - zabs * (anp[2] - anp[3] * y / (y + anp[4] - anp[5] /
                                                     (y + anp[6] + anp[7] / (y
                                                                             +
                                                                             anp[8]))));
         if (z < 0)
         {
            p = q;
            q = 1 - p;
         }
         else
         {
            p = 1 - q;
         }
      }
   }
   return (p);

}


///////////////////////////////////////////////////////////////////////////////
//                 Destroy object instance (C++ destructor).                 //
///////////////////////////////////////////////////////////////////////////////

FtSample::~FtSample (void)
{
   FtSample_c ftsample = this;

   if ((ftsample->FtGrainAges))
   {
      DaListDestroy ((ftsample->FtGrainAges), TRUE);
   }
   if ((ftsample->GRAINFTAGEDIST))
      Histogram_Delete ((ftsample->GRAINFTAGEDIST));
   if ((ftsample->PredTracklengths))
   {
      DaListDestroy ((ftsample->PredTracklengths), TRUE);
   }
   if ((ftsample->ObsTracklengths))
   {
      DaListDestroy ((ftsample->ObsTracklengths), TRUE);
   }
   if ((ftsample->AFTCALC))
      AftCalc_Remove_FtSamples ((ftsample->AFTCALC), ftsample);
   if ((ftsample->TMPBINCNT))
      PropDoubleVector_Delete ((ftsample->TMPBINCNT));
   if ((ftsample->SAMPLECODE))
      DDSfree ((ftsample->SAMPLECODE));

}
