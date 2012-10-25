/*******************************************************************************
* Class:       AftCalc
*
* File:        aftcalc.C
*
* Copyright:   (c) 1997 Shell International Exploration and Production
*
* GeoCase rev: v1.0
*
* Description: 
 
~ *****************************************************************************
~
~   File name:    RBYaftcalc.def
~   Copyright:    KSEPL
~   Function:     Definition and implementation of the AFTA calculation
~   Dependencies: 
~   Usage:        
~   References:
~   Remarks:
~
~ *****************************************************************************
 

 ~ ===========================================================================
 ~
 ~  Class Name:       AftCalc
 ~  Subclass of:  
 ~  Description:      Appatite fission track calculator
 ~  Attributes:
 ~    Name            type             Description
 ~    -------------------------------------------------------------------------
 ~    CalcEngine      CalcEngine       Pointer to the parent CalcEngine
 ~    FtSamples       List             List of fission track samples
 ~    C0              PropDoubleVector Compositional dependent list of parameters 
 ~    C1              PropDoubleVector Compositional dependent list of parameters 
 ~    TRACKNUMDIST    int              Number of tracklengths in result distributions
 ~ ============================================================================


*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                              AFTCALC Methods                              //
///////////////////////////////////////////////////////////////////////////////
// This object is a member of the module RUBYPROJECT in project RBYPROJECT   //
///////////////////////////////////////////////////////////////////////////////
#include   <stdio.h>
#include   <string.h>
#include   "DDSall.h"
#include   "rubyutilities.h"
#define  aftcalc__file__name  1
#include   "rbyproject.h"
#include   "aftcalc.h"
#include   "aftcalcP.h"

#include "modelnodeiterator.h"

#include "ftsampleiotbl.h"
#include "ftlengthiotbl.h"
#include "ftgrainiotbl.h"
#include "histogramtbl.h"

#include "ftsample.h"
#include "calcengine.h"
#include "project.h"

#include "propdoublevector.h"

#include "polyfunction.h"	// From RubyUtilities...
#include "RBYconstants.h"


///////////////////////////////////////////////////////////////////////////////
//                  Private and Static method declarations.                  //
///////////////////////////////////////////////////////////////////////////////

void AftCalc_Add_FtSamples (AftCalc_c aftcalc, FtSample_c member)
{
   aftcalc->AftCalc::Add_FtSamples (member);
}
void AftCalc_BinFtGrainAges (AftCalc_c aftcalc, FtGrainIoTbl_c FtGrainIoTbl)
{
   aftcalc->AftCalc::BinFtGrainAges (FtGrainIoTbl);
}
void AftCalc_BinFtLengths (AftCalc_c aftcalc, FtSampleIoTbl_c FtSampleIoTbl,
                           FtLengthIoTbl_c FtLengthIoTbl, FtGrainIoTbl_c
FtGrainIoTbl)
{
   aftcalc->AftCalc::BinFtLengths (FtSampleIoTbl, FtLengthIoTbl,
   FtGrainIoTbl);
}
float CalcFtAge (double NsDivNi, double zeta, double UstglTrDens)
{
   return AftCalc::CalcFtAge (NsDivNi, zeta, UstglTrDens);
}
void CalcFtAgeChi2 (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int totNs, int
                    totNi, float *Chi2)
{
   AftCalc::CalcFtAgeChi2 (FtGrainIoTbl, sampleId, totNs, totNi, Chi2);
}
void CalcFtCentralAge (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                       grainNum, int totNs, int totNi, double UstglTrDens, double
                       zeta, float *CentralAge, float *error)
{
   AftCalc::CalcFtCentralAge (FtGrainIoTbl, sampleId, grainNum, totNs, totNi,
                              UstglTrDens, zeta, CentralAge, error);
}
void CalcFtCorrCoeff (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int totNs, int
                      totNi, int grainNum, float *CorrCoeff)
{
   AftCalc::CalcFtCorrCoeff (FtGrainIoTbl, sampleId, totNs, totNi, grainNum,
   CorrCoeff);
}
void AftCalc_CalcFtGrainAges (AftCalc_c aftcalc, FtSampleIoTbl_c
                              FtSampleIoTbl, FtGrainIoTbl_c FtGrainIoTbl)
{
   aftcalc->AftCalc::CalcFtGrainAges (FtSampleIoTbl, FtGrainIoTbl);
}
void AftCalc_CalcFtLengthChi2 (AftCalc_c aftcalc, FtSampleIoTbl_c
FtSampleIoTbl)
{
   aftcalc->AftCalc::CalcFtLengthChi2 (FtSampleIoTbl);
}
void CalcFtMeanAge (double zeta, double zetaErr, double UstglTrDens, float
                    MeanRatio, float MeanRatioErr, float *MeanAge, float
*error)
{
   AftCalc::CalcFtMeanAge (zeta, zetaErr, UstglTrDens, MeanRatio,
                           MeanRatioErr, MeanAge, error);
}
void CalcFtMeanRatio (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                      grainNum, float *MeanRatio, float *error)
{
   AftCalc::CalcFtMeanRatio (FtGrainIoTbl, sampleId, grainNum, MeanRatio,
   error);
}
void CalcFtNsDivNi (int totNs, int totNi, float *NsDivNi, float *error)
{
   AftCalc::CalcFtNsDivNi (totNs, totNi, NsDivNi, error);
}
void CalcFtP_Chi2 (int dof, float Chi2, float *P_Chi2)
{
   AftCalc::CalcFtP_Chi2 (dof, Chi2, P_Chi2);
}
void CalcFtPooledAge (double zeta, double zetaErr, double UstglTrDens, int
                      totNs, int totNi, float *PooledAge, float *error)
{
   AftCalc::CalcFtPooledAge (zeta, zetaErr, UstglTrDens, totNs, totNi,
                             PooledAge, error);
}
void AftCalc_CalcFtSampleData (AftCalc_c aftcalc, FtSampleIoTbl_c
                               FtSampleIoTbl, FtGrainIoTbl_c FtGrainIoTbl)
{
   aftcalc->AftCalc::CalcFtSampleData (FtSampleIoTbl, FtGrainIoTbl);
}
void CalcFtVarianceSqrtNiNs (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                             grainNum, float *VarianceSqrtNi, float
*VarianceSqrtNs)
{
   AftCalc::CalcFtVarianceSqrtNiNs (FtGrainIoTbl, sampleId, grainNum,
                                    VarianceSqrtNi, VarianceSqrtNs);
}
void AftCalc_CalcPredFtAge (AftCalc_c aftcalc, FtSampleIoTbl_c FtSampleIoTbl)
{
   aftcalc->AftCalc::CalcPredFtAge (FtSampleIoTbl);
}
void AftCalc_CalcPredTracklengths (AftCalc_c aftcalc, double time, double
timestep)
{
   aftcalc->AftCalc::CalcPredTracklengths (time, timestep);
}
AftCalc_c AftCalc_Create (BOOLEAN_t alloc)
{
   return AftCalc::Create (alloc);
}
void AftCalc_Delete (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::Delete ();
}
void AftCalc_DeleteModel (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::DeleteModel ();
}
void AftCalc_Finalize (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::Finalize ();
}
FtSample_c AftCalc_FindFtSample (AftCalc_c aftcalc, STRING sampleId)
{
   return aftcalc->AftCalc::FindFtSample (sampleId);
}
void AftCalc_ForAll_FtSamples (AftCalc_c aftcalc, ActionMethod Action, long
*arg)
{
   aftcalc->AftCalc::ForAll_FtSamples (Action, arg);
}
BOOLEAN_t AftCalc_FtSamples_GtFunc (FtSample_c ftsample1, FtSample_c
ftsample2)
{
   return AftCalc::FtSamples_GtFunc (ftsample1, ftsample2);
}
float Gauss (float x)
{
   return AftCalc::Gauss (x);
}
void AftCalc_GeotrackCorrPredTracklengthDists (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::GeotrackCorrPredTracklengthDists ();
}
double AftCalc_GetC0 (AftCalc_c aftcalc, int ix)
{
   return aftcalc->AftCalc::GetC0 (ix);
}
double AftCalc_GetC1 (AftCalc_c aftcalc, int ix)
{
   return aftcalc->AftCalc::GetC1 (ix);
}
BOOLEAN_t AftCalc_GetFtHistogramPlots (AftCalc_c aftcalc, STRING sampleId, int
                                       ChlorideIx, HistogramTbl_c ObsDist,
                                       HistogramTbl_c PredDist, int
                                       *MaxTrackCount, HistogramTbl_c AgeDist,
                                       int *MaxGrainCount)
{
   return aftcalc->AftCalc::GetFtHistogramPlots (sampleId, ChlorideIx,
                                                 ObsDist, PredDist,
                                                 MaxTrackCount, AgeDist,
   MaxGrainCount);
}
void GetFtSampleCounts (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                        *grainNum, int *totNs, int *totNi)
{
   AftCalc::GetFtSampleCounts (FtGrainIoTbl, sampleId, grainNum, totNs,
   totNi);
}
double AftCalc_GetL0 (AftCalc_c aftcalc, int ClIndex)
{
   return aftcalc->AftCalc::GetL0 (ClIndex);
}
void AftCalc_Init (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::Init ();
}
void AftCalc_Initialize (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::Initialize ();
}
AftCalc_c AftCalc_Make (void)
{
   return AftCalc::Make ();
}
void AftCalc_MakeFtSamples (AftCalc_c aftcalc, FtSampleIoTbl_c FtSampleIoTbl,
                            FtGrainIoTbl_c FtGrainIoTbl, FtLengthIoTbl_c
                            FtLengthIoTbl, StratIoTbl_c StratIoTbl)
{
   aftcalc->AftCalc::MakeFtSamples (FtSampleIoTbl, FtGrainIoTbl,
                                    FtLengthIoTbl, StratIoTbl);
}
void AftCalc_MergeTracklengthDists (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::MergeTracklengthDists ();
}
void AftCalc_Print (AftCalc_c aftcalc, long *arg)
{
   aftcalc->AftCalc::Print (arg);
}
void AftCalc_Remove_FtSamples (AftCalc_c aftcalc, FtSample_c member)
{
   aftcalc->AftCalc::Remove_FtSamples (member);
}
void AftCalc_ScaleObsTracklengthDists (AftCalc_c aftcalc, int tracknum)
{
   aftcalc->AftCalc::ScaleObsTracklengthDists (tracknum);
}
void AftCalc_ScalePredTracklenghtDists (AftCalc_c aftcalc, int tracknum)
{
   aftcalc->AftCalc::ScalePredTracklenghtDists (tracknum);
}
void AftCalc_ScalePredTracklengthsToObserved (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::ScalePredTracklengthsToObserved ();
}
void AftCalc_ScaleTracklengthDists (AftCalc_c aftcalc)
{
   aftcalc->AftCalc::ScaleTracklengthDists ();
}
void AftCalc_SetFtIoTbls (AftCalc_c aftcalc, FtSampleIoTbl_c FtSampleIoTbl,
                          FtGrainIoTbl_c FtGrainIoTbl, FtLengthIoTbl_c
                          FtLengthIoTbl, StratIoTbl_c StratIoTbl)
{
   aftcalc->AftCalc::SetFtIoTbls (FtSampleIoTbl, FtGrainIoTbl, FtLengthIoTbl,
   StratIoTbl);
}
///////////////////////////////////////////////////////////////////////////////
//                      Member function Implementation                       //
///////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::Add_FtSamples (FtSample_c member)
{
   AftCalc_c aftcalc = this;

   ListAppendItem ((aftcalc->FtSamples), (AnyPntr_t) member);

}




/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            BinFtGrainAges
 ~ ACCESS:          Private
 ~ FUNCTION:        Bin the calculated grain ages stored in the FtGrainIoTbl 
 ~                  in a histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name               Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c     I    Fission track grain data
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::BinFtGrainAges (FtGrainIoTbl_c FtGrainIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   STRING sampleId;
   float ClPerc;
   float grainAge;
   FtSample_c ftsample;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      sampleId = FtGrainIoTblRow_FtSampleId (gRow);

      grainAge = FtGrainIoTblRow_FtGrainAge (gRow);
      ClPerc = FtGrainIoTblRow_FtClWeightPerc (gRow);

      if (!Ibscfuncs::DoubleEqual (grainAge, PROPSYS_DOUBLE_NULL))
      {
         ftsample = aftcalc->FindFtSample (sampleId);
         if (ftsample)
         {
            ftsample->BinFtGrainAge (ClPerc, grainAge);
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            BinFtLengths
 ~ ACCESS:          Private
 ~ FUNCTION:        Bin the track lengths defined in the FtLengthIoTbl per sample
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type             I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl       FtSampleIoTbl_c   I    Fission track sample data
 ~ FtLengthIoTbl       FtLengthIoTbl_c   I    Fission track length data
 ~ FtGrainIoTbl        FtGrainIoTbl_c    I    Fission track grain data 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::BinFtLengths (FtSampleIoTbl_c FtSampleIoTbl, FtLengthIoTbl_c
                            FtLengthIoTbl, FtGrainIoTbl_c FtGrainIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it1, it2, it3;
   FtSampleIoTblRow_c sRow;
   FtLengthIoTblRow_c lRow;
   FtGrainIoTblRow_c gRow;
   int grainId;
   float tracklength, ClPerc;
   BOOLEAN_t found;
   FtSample_c ftsample;
   STRING sampleId;
   float depth;


   it1 = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (sRow = (FtSampleIoTblRow_c) Next (it1))
   {
      sampleId = FtSampleIoTblRow_FtSampleId (sRow);
      ftsample = aftcalc->FindFtSample (sampleId);

      if (!ftsample)
      {
         showerror ("AftCalc: BinFtSamples -  FtSample %s not found", sampleId);
      }
      else
      {
         // loop over all entries in FtLengthIoTbl that belong to ftsample

         it2 = FtLengthIoTbl_IteratorInit_FtLengthIoTblRows (FtLengthIoTbl);
         while (lRow = (FtLengthIoTblRow_c) Next (it2))
         {
            tracklength = FtLengthIoTblRow_FtLength (lRow);
            grainId = FtLengthIoTblRow_FtGrainId (lRow);

            if (!strcmp (sampleId, FtLengthIoTblRow_FtSampleId (lRow)))
            {
               it3 = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);

               // find the entry in the FtGrainIoTbl that belongs to (sampleId,grainId)
               found = FALSE;
               while ((!found) && (gRow = (FtGrainIoTblRow_c) Next (it3)))
               {
                  found = (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId
                           (gRow)) &&
                           grainId == FtGrainIoTblRow_FtGrainId (gRow));
               }
               if (!found)
               {
                  showerror
                             ("AftCalc: BinFtSamples - ClWeightPercentage not defined");
               }
               else
               {
                  // Get the Cl percentage and bin the tracklength in the histogram of that Cl Perc
                  ClPerc = FtGrainIoTblRow_FtClWeightPerc (gRow);
                  ftsample->BinObsTracklengths (tracklength, ClPerc);
               }
            }
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtAge
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the fission track age as function of Ns/Ni (uses the 
 ~                  standard fission track age formula)
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ NsDivNi               double       I    Number of spontaneous track divided 
 ~                                         by number of induced tracks
 ~ zeta                  double       I    Zeta calibration factor
 ~ UstglTrDens           double       I    Track density from Uranium standard glass
 ~ 
 ~ RETURN VALUE: float -> fission track age
 ~ ============================================================================


*******************************************************************************/

float AftCalc::CalcFtAge (double NsDivNi, double zeta, double UstglTrDens)
{
// Source file:RBYaftcalc.def                                                //

   float ftAge;

   /* calculate fission track age in years */
   ftAge = log (1. + (zeta * U238DECAYCONSTANT * NsDivNi * .5 * UstglTrDens)) /
         U238DECAYCONSTANT;

   /* set to Ma */
   ftAge = ftAge / 1e+6;

   return (ftAge);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtAgeChi2
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the Chi squared of the fission track age
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ totNs              int              I    total number of spontaneous tracks in the sample
 ~ totNi              int              I    total number of induced tracks in the sample
 ~ Chi2               float*           O    Chi squared of fission track age     
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtAgeChi2 (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                             totNs, int totNi, float *Chi2)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int Ns, Ni;
   float N_sj, N_ij;

   *Chi2 = .0;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
      {
         Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
         Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

         if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
             Ni != 0)           // if ( Ni != 0 && Ns != 0 )
         {
            N_sj = (float) (totNs * (Ns + Ni)) / (float) (totNs + totNi);
            N_ij = (float) (totNi * (Ns + Ni)) / (float) (totNs + totNi);

            *Chi2 += ((float) Ns - N_sj) * ((float) Ns - N_sj) / N_sj +
                  ((float) Ni - N_ij) * ((float) Ni - N_ij) / N_ij;
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtCentralAge
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the central age and error
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         This is copied from undocumented Geotrack source code 
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ grainNum           int              I    Number of grains in the sample     
 ~ totNs              int              I    total number of spontaneous tracks in the sample
 ~ totNi              int              I    total number of induced tracks in the sample
 ~ UstglTrDens        double           I    Track density from Uranium standard glass
 ~ zeta               double           I    Zeta calibration factor
 ~ CentralAge         float*           I    Central age
 ~ error              float*           I    Error in central age
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtCentralAge (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                                grainNum, int totNs, int totNi, double
                                UstglTrDens, double zeta, float *CentralAge, float *error)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int Ns, Ni;
   double sigma, theta;
   double x, t, z;
   double xSum, x2Sum, ySum, pSum, qSum, z2Sum;
   double fds, ratio;
   int nIter;

   if (grainNum > 1)
   {
      xSum = .0;
      x2Sum = .0;
      ySum = .0;

      it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
      while (gRow = (FtGrainIoTblRow_c) Next (it))
      {
         if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
         {
            Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
            Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

            if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
                Ni != 0)        // if ( Ni != 0 && Ns != 0 )
            {
               x = log (((double) Ns + .5) / ((double) Ni + .5));
               ySum += (double) Ns / (double) (Ns + Ni);
               xSum += x;
               x2Sum += x * x;
            }
         }
      }
      theta = ySum / (double) (totNs + totNi);
      sigma = .6 * sqrt ((x2Sum - xSum * xSum / (double) grainNum) / (double)
                         (grainNum - 1));

      for (nIter = 0, fds = 1.; nIter <= 20 && fds >= .001; nIter++)
      {
         pSum = .0;
         qSum = .0;

         t = theta * (1. - theta);

         it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
         while (gRow = (FtGrainIoTblRow_c) Next (it))
         {
            if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
            {
               Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
               Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

               if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
                   Ni != 0)     // if ( Ni != 0 && Ns != 0 )
               {
                  x = (double) (Ns + Ni - 1) * sigma * t * sigma * t + t;
                  pSum += (double) (Ns + Ni) / x;
                  qSum += (double) Ns / x;
               }
            }
         }


         theta = qSum / pSum;
         t = theta * (1. - theta);
         z2Sum = .0;

         it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
         while (gRow = (FtGrainIoTblRow_c) Next (it))
         {
            if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
            {
               Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
               Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

               if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
                   Ni != 0)     // if ( Ni != 0 && Ns != 0 )
               {
                  z = ((double) Ns - (double) (Ns + Ni) * theta) /
                        ((double) (Ns + Ni - 1) * sigma * t * sigma * t + t);
                  z2Sum += z * z;
               }
            }
         }

         sigma *= sqrt (z2Sum / pSum);
         fds = ABS (z2Sum / pSum - 1.);
      }
      ratio = theta / (1. - theta);

      *CentralAge = AftCalc::CalcFtAge (ratio, zeta, UstglTrDens);
      *error = *CentralAge / sqrt (pSum * t * t);
   }
   else
   {
      *CentralAge = AftCalc::CalcFtAge ((double) totNs / (double) totNi, zeta, UstglTrDens);
      *error = *CentralAge * sqrt (1. / (double) totNs + 1. / (double) totNi);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtCorrCoeff
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the correlation coefficient between the number of spontaneous
 ~                  tracks and the number of induced tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ totNs              int              I    total number of spontaneous tracks in the sample
 ~ totNi              int              I    total number of induced tracks in the sample
 ~ grainNum           int              I    Number of grains in the sample     
 ~ CorrCoeff          float*           O    Correlation coefficient    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtCorrCoeff (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                               totNs, int totNi, int grainNum, float *CorrCoeff)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   float aveNs, aveNi;
   int Ns, Ni;
   float sigmaNsNi, sigmaNs, sigmaNi;

   aveNs = (float) totNs / (float) grainNum;
   aveNi = (float) totNi / (float) grainNum;

   sigmaNsNi = .0;
   sigmaNs = .0;
   sigmaNi = .0;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
      {
         Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
         Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

         if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
             Ni != 0)           // if ( Ni != 0 && Ns != 0 )
         {
            sigmaNsNi += ((float) Ns - aveNs) * ((float) Ni - aveNi);
            sigmaNs += ((float) Ns - aveNs) * ((float) Ns - aveNs);
            sigmaNi += ((float) Ni - aveNi) * ((float) Ni - aveNi);
         }
      }
   }

   sigmaNs = sqrt (sigmaNs);
   sigmaNi = sqrt (sigmaNi);

   if (Ibscfuncs::DoubleEqual (sigmaNsNi, .0))
   {
      if (Ibscfuncs::DoubleEqual (sigmaNs, .0) && Ibscfuncs::DoubleEqual
          (sigmaNi, .0))
      {
         *CorrCoeff = 1.;
      }
      else
      {
         *CorrCoeff = 0.;
      }
   }
   else
   {
      *CorrCoeff = sigmaNsNi / (sigmaNs * sigmaNi);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtGrainAges
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate and set the ages of the individual grains in the FtGrainIoTbl
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name               Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl      FtSampleIoTbl_c    I    Fission track sample data
 ~ FtGrainIoTbl       FtGrainIoTbl_c     IO   Fission track grain data
 ~
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtGrainAges (FtSampleIoTbl_c FtSampleIoTbl, FtGrainIoTbl_c FtGrainIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it1, it2;
   FtSampleIoTblRow_c sRow;
   FtGrainIoTblRow_c gRow;
   double zeta, UstglTrDens;
   STRING sampleId;
   int Ns, Ni;
   float ftAge, ftAgeErr;

   it1 = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (sRow = (FtSampleIoTblRow_c) Next (it1))
   {
      sampleId = FtSampleIoTblRow_FtSampleId (sRow);
      zeta = FtSampleIoTblRow_FtZeta (sRow);
      UstglTrDens = FtSampleIoTblRow_FtUstglTrackDensity (sRow);

      // calculate fission track age for each entry in the FtGrainIoTbl

      it2 = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
      while (gRow = (FtGrainIoTblRow_c) Next (it2))
      {
         if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
         {
            Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
            Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

            if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
                Ni != 0 && Ns != 0)  // if ( Ni != 0 && Ns != 0 )
            {
               // calculate and set fission track age

               ftAge = AftCalc::CalcFtAge ((double) Ns / (double) Ni, zeta, UstglTrDens);
               FtGrainIoTblRow_FtGrainAge (gRow) = ftAge;

               // calculate and set error in fission track age

               ftAgeErr = ftAge * sqrt (1. / (float) Ns + 1. / (float) Ni);
               FtGrainIoTblRow_FtGrainAgeErr (gRow) = ftAgeErr;

            }
            else
            {
               FtGrainIoTblRow_FtGrainAge (gRow) = PROPSYS_DOUBLE_NULL;
               FtGrainIoTblRow_FtGrainAgeErr (gRow) = PROPSYS_DOUBLE_NULL;
            }
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtLengthChi2
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the Chi squared of the predicted and observed
 ~                  track length histograms of all samples. The Chi squared is
 ~                  used for calibration
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type             I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl       FtSampleIoTbl_c   I    Fistion track sample data
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtLengthChi2 (FtSampleIoTbl_c FtSampleIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;
   FtSampleIoTblRow_c sRow;
   STRING sampleId;
   float Chi2;

   it = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (sRow = (FtSampleIoTblRow_c) Next (it))
   {
      sampleId = FtSampleIoTblRow_FtSampleId (sRow);

      ftsample = aftcalc->FindFtSample (sampleId);
      if (ftsample)
      {
         ftsample->CalcFtLengthChi2 (&Chi2);
         FtSampleIoTblRow_FtLengthChi2 (sRow) = Chi2;
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtMeanAge
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the average fission track grain age
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ zeta                  double       I    Zeta calibration factor
 ~ zetaErr               double       I    Error in zeta calibration factor
 ~ UstglTrDens           double       I    Track density from Uranium standard glass
 ~ MeanRatio             float        I    Avarage Ns/Ni
 ~ MeanRatioErr          float        I    Error MeanRatio
 ~ MeanAge               float*       O    Average fission track grain age
 ~ error                 float*       O    Error in fission track grain age
  ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtMeanAge (double zeta, double zetaErr, double UstglTrDens, float
                             MeanRatio, float MeanRatioErr, float *MeanAge, float *error)
{
// Source file:RBYaftcalc.def                                                //

   *MeanAge = AftCalc::CalcFtAge ((double) MeanRatio, zeta, UstglTrDens);
   *error = *MeanAge *
         sqrt ((MeanRatioErr * MeanRatioErr) / (MeanRatio * MeanRatio) +
               (zetaErr * zetaErr) / (zeta * zeta));

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtMeanRatio
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the average of Ns/Ni and the error
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ grainNum           int              I    Number of grains in the sample     
 ~ MeanRatio          float*           O    average of Ns/Ni    
 ~ error              float*           O    error in MeanRatio    
  ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtMeanRatio (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                               grainNum, float *MeanRatio, float *error)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int Ns, Ni;
   float sum, sum2;
   float ratio, n;

   ratio = .0;
   sum2 = .0;
   sum = .0;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
      {
         Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
         Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

         if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
             Ni != 0)           // if ( Ni != 0 && Ns != 0 )
         {
            ratio = (float) Ns / (float) Ni;
            sum2 += ratio * ratio;
            sum += ratio;
         }
      }
   }
   n = (float) grainNum;
   *MeanRatio = sum / n;
   if (grainNum == 1)
   {
      *error = 0.;
   }
   else
   {
      *error = sqrt ((sum2 - sum * sum / n) / (n * (n - 1)));
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtNsDivNi
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the ratio between total Ns and total Ni and the error 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name               Type            I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ totNs              int              I    total number of spontaneous tracks in the sample
 ~ totNi              int              I    total number of induced tracks in the sample
 ~ NsDivNi            float*           O    total Ns/total Ni    
 ~ error              float*           O    error in total Ns/total Ni   
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtNsDivNi (int totNs, int totNi, float *NsDivNi, float *error)
{
// Source file:RBYaftcalc.def                                                //

   *NsDivNi = (float) totNs / (float) totNi;
   *error = sqrt (*NsDivNi * (*NsDivNi + 1) / (float) totNi);

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::CalcFtP_Chi2 (int dof, float Chi2, float *P_Chi2)
{
// Source file:RBYaftcalc.def                                                //

   float b0 = 0.572364942925;
   float b1 = 0.564189583548;
   float xx = Chi2;
   float a = xx / 2.;
   float y = 0.;
   float t, z, e, c;
   int ilim, i;

   if (dof % 2 == 0 || dof > 2 && xx <= 40.)
   {
      y = exp (-a);
   }
   if (dof % 2 == 0)
   {
      *P_Chi2 = y;
   }
   else
   {
      t = -sqrt (xx);
      *P_Chi2 = 2. * AftCalc::Gauss (t);
   }
   if (dof > 2)
   {
      xx = (float) (dof - 1) / 2.;
      if (dof % 2 == 0)
      {
         z = 1;
      }
      else
      {
         z = 0.5;
      }
      ilim = (int) (xx - z) + 1;
      if (xx <= 40.)
      {
         if (dof % 2 == 0)
         {
            e = 0.;
         }
         else
         {
            e = b0;
         }
         c = log (a);
         for (i = 1; i <= ilim; i++)
         {
            e = log (z) + e;
            *P_Chi2 += exp (c * z - a - e);
            z += 1.;
         }
      }
      else
      {
         if (dof % 2 == 0)
         {
            e = 1.;
         }
         else
         {
            e = b1 / sqrt (a);
         }
         c = 0.;
         for (i = 1; i <= ilim; i++)
         {
            e *= a / z;
            c += e;
            z += 1.;
         }
         *P_Chi2 += c * y;
      }
   }
   *P_Chi2 *= 100.;

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtPooledAge
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate the pooled age of the sample
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ zeta                  double       I    Zeta calibration factor
 ~ UstglTrDens           double       I    Track density from Uranium standard glass
 ~ totNs                 int          I    total number of spontaneous tracks in the sample
 ~ totNi                 int          I    total number of induced tracks in the sample
 ~ PooledAge             float*       O    Pooled age
 ~ error                 float*       O    Error in pooled age
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtPooledAge (double zeta, double zetaErr, double
                               UstglTrDens, int totNs, int totNi, float
                               *PooledAge, float *error)
{
// Source file:RBYaftcalc.def                                                //


   *PooledAge = AftCalc::CalcFtAge ((double) totNs / (double) totNi, zeta, UstglTrDens);
   *error = *PooledAge *
         sqrt (1. / (float) totNs + 1. / (float) totNi + (zetaErr * zetaErr) /
               (zeta * zeta));

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtSampleData
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculate all items from a fission track data sheet and store in FtSampleIoTbl
 ~                  Calculate the fission track ages of the grains and store in FtGrainIoTbl
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                 Type            I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl      FtSampleIoTbl_c    IO   Fission track sample data
 ~ FtGrainIoTbl       FtGrainIoTbl_c     IO   Fission track grain data
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtSampleData (FtSampleIoTbl_c FtSampleIoTbl, FtGrainIoTbl_c FtGrainIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtSampleIoTblRow_c sRow;
   double zeta, UstglTrDens;
   STRING sampleId;
   int totNs, totNi, grainNum;

   it = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (sRow = (FtSampleIoTblRow_c) Next (it))
   {
      sampleId = FtSampleIoTblRow_FtSampleId (sRow);
      zeta = FtSampleIoTblRow_FtZeta (sRow);
      UstglTrDens = FtSampleIoTblRow_FtUstglTrackDensity (sRow);

      AftCalc::GetFtSampleCounts (FtGrainIoTbl, sampleId, &grainNum, &totNs, &totNi);

      if (totNs != 0 && totNi != 0)
      {
         {
            float PooledAge, PooledAgeErr;

            AftCalc::CalcFtPooledAge (zeta, .0, UstglTrDens, totNs, totNi,
                                      &PooledAge, &PooledAgeErr);

            FtSampleIoTblRow_FtPooledAge (sRow) = PooledAge;
            FtSampleIoTblRow_FtPooledAgeErr (sRow) = PooledAgeErr;
         }
         {
            float Chi2, P_Chi2;

            AftCalc::CalcFtAgeChi2 (FtGrainIoTbl, sampleId, totNs, totNi, &Chi2);
            FtSampleIoTblRow_FtAgeChi2 (sRow) = Chi2;
            FtSampleIoTblRow_FtDegreeOfFreedom (sRow) = grainNum - 1;

            AftCalc::CalcFtP_Chi2 (grainNum - 1, Chi2, &P_Chi2);
            FtSampleIoTblRow_FtPAgeChi2 (sRow) = P_Chi2;
         }
         {
            float corrcoeff;

            AftCalc::CalcFtCorrCoeff (FtGrainIoTbl, sampleId, totNs, totNi,
                                      grainNum, &corrcoeff);
            FtSampleIoTblRow_FtCorrCoeff (sRow) = corrcoeff;
         }
         {
            float varSqrtNs, varSqrtNi;

            AftCalc::CalcFtVarianceSqrtNiNs (FtGrainIoTbl, sampleId, grainNum,
                                             &varSqrtNi, &varSqrtNs);
            FtSampleIoTblRow_FtVarianceSqrtNs (sRow) = varSqrtNs;
            FtSampleIoTblRow_FtVarianceSqrtNi (sRow) = varSqrtNi;
         }
         {
            float NsDivNi, NsDivNiErr;

            AftCalc::CalcFtNsDivNi (totNs, totNi, &NsDivNi, &NsDivNiErr);

            FtSampleIoTblRow_FtNsDivNi (sRow) = NsDivNi;
            FtSampleIoTblRow_FtNsDivNiErr (sRow) = NsDivNiErr;
         }
         {
            float MeanRatio, MeanRatioErr;
            float MeanAge, MeanAgeErr;

            AftCalc::CalcFtMeanRatio (FtGrainIoTbl, sampleId, grainNum,
                                      &MeanRatio, &MeanRatioErr);

            FtSampleIoTblRow_FtMeanRatio (sRow) = MeanRatio;
            FtSampleIoTblRow_FtMeanRatioErr (sRow) = MeanRatioErr;

            AftCalc::CalcFtMeanAge (zeta, .0, UstglTrDens, MeanRatio,
                                    MeanRatioErr, &MeanAge, &MeanAgeErr);

            FtSampleIoTblRow_FtMeanAge (sRow) = MeanAge;
            FtSampleIoTblRow_FtMeanAgeErr (sRow) = MeanAgeErr;

         }
         {
            float CentralAge, CentralAgeErr;

            AftCalc::CalcFtCentralAge (FtGrainIoTbl, sampleId, grainNum,
                                       totNs, totNi,
                                       UstglTrDens, zeta, &CentralAge, &CentralAgeErr);

            FtSampleIoTblRow_FtCentralAge (sRow) = CentralAge;
            FtSampleIoTblRow_FtCentralAgeErr (sRow) = CentralAgeErr;
         }
         {
            if (grainNum > 20)
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Excellent");
            }
            else if (grainNum == 20)
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Very Good");
            }
            else if (grainNum >= 15)
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Good");
            }
            else if (grainNum >= 10)
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Fair");
            }
            else if (grainNum >= 5)
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Poor");
            }
            else
            {
               STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), "Very Poor");
            }
         }
      }
      else
      {
         FtSampleIoTblRow_FtPooledAge (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtPooledAgeErr (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtAgeChi2 (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtDegreeOfFreedom (sRow) = PROPSYS_INTEGER_NULL;
         FtSampleIoTblRow_FtPAgeChi2 (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtCorrCoeff (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtVarianceSqrtNs (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtVarianceSqrtNi (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtNsDivNi (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtNsDivNiErr (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtMeanRatio (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtMeanRatioErr (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtMeanAge (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtMeanAgeErr (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtCentralAge (sRow) = PROPSYS_DOUBLE_NULL;
         FtSampleIoTblRow_FtCentralAgeErr (sRow) = PROPSYS_DOUBLE_NULL;
         STR_ASS (FtSampleIoTblRow_FtApatiteYield (sRow), PROPSYS_STRING_NULL);
      }

   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtVarianceSqrtNiNs
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the Variance of the square root of the induced tracks 
 ~                  and the spontaneous tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name               Type            I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ grainNum           int              I    Number of grains in the sample     
 ~ VarianceSqrtNi     float*           O    Variance of the square root of Ni    
 ~ VarianceSqrtNs     float*           O    Variance of the square root of Ns    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcFtVarianceSqrtNiNs (FtGrainIoTbl_c FtGrainIoTbl, STRING
                                      sampleId, int grainNum, float
                                      *VarianceSqrtNi, float *VarianceSqrtNs)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int Ni, Ns;
   float aveSqrtNi, sumNi;
   float aveSqrtNs, sumNs;

   aveSqrtNi = .0;
   aveSqrtNs = .0;

   // Calculate the average of the square root of Ni and Ns

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
      {
         Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);
         Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);

         if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
             Ni != 0)           // if ( Ni != 0 && Ns != 0 )
         {
            aveSqrtNi += sqrt ((float) Ni);
            aveSqrtNs += sqrt ((float) Ns);
         }
      }
   }
   aveSqrtNi = aveSqrtNi / grainNum;
   aveSqrtNs = aveSqrtNs / grainNum;

   sumNi = .0;
   sumNs = .0;

   if (grainNum > 1)
   {
      it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
      while (gRow = (FtGrainIoTblRow_c) Next (it))
      {
         if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
         {
            Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);
            Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);

            if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
                Ni != 0)        // if ( Ni != 0 && Ns != 0 )
            {
               sumNi += (sqrt  ((float) Ni) - aveSqrtNi) * (sqrt ((float) Ni) - aveSqrtNi);
               sumNs += (sqrt  ((float) Ns) - aveSqrtNs) * (sqrt ((float) Ns) - aveSqrtNs);
            }

         }
      }
      *VarianceSqrtNi = sumNi / (grainNum - 1);
      *VarianceSqrtNs = sumNs / (grainNum - 1);
   }
   else
   {
      *VarianceSqrtNi = 0.;
      *VarianceSqrtNs = 0.;
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcPredFtAge
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the predicted fission track ages of the samples 
 ~                  and stores the result in FtSampleIoTbl
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl         FtSampleIoTbl_c    IO   Fission track sample data    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcPredFtAge (FtSampleIoTbl_c FtSampleIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;
   FtSampleIoTblRow_c sRow;
   STRING sampleId;
   float PredFtAge;

   it = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (sRow = (FtSampleIoTblRow_c) Next (it))
   {
      sampleId = FtSampleIoTblRow_FtSampleId (sRow);

      ftsample = aftcalc->FindFtSample (sampleId);
      if (ftsample)
      {
         if (((ftsample)->SAMPLELOCATIONSET))
         {
            if (ftsample->CalcPredFtAge (&PredFtAge))
            {
               FtSampleIoTblRow_FtPredictedAge (sRow) = PredFtAge;
            }
            else
            {
               FtSampleIoTblRow_FtPredictedAge (sRow) = PROPSYS_DOUBLE_NULL;
            }
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcPredTracklengths
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculates the predicted track length distibution from 
 ~                  time/temperature history
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ timestep              double       I    Current step in time
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::CalcPredTracklengths (double time, double timestep)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   Project_c project = ((((aftcalc)->CALCENGINE))->PROJECT);
   FtSample_c ftsample;
   ITERATOR_t it;
   double temperature, dtime;
   int i;

   dtime = timestep * SECS_IN_MA;

   if (project->IsOneDimensional ())
   {
      it = (ListInitNextItem (((aftcalc)->FtSamples)));
      while (ftsample = (FtSample_c) Next (it))
      {
         if (((ftsample)->SAMPLELOCATIONSET))
         {
            if (ftsample->GetTemperature (&temperature))
            {
               // calculate the predicted track lenght distribution for all 
               // Cl weigth percentages
               for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
               {
                  ftsample->CalcPredTracklengths (
                                                    dtime, temperature, i);
               }
            }
         }
      }
   }

}


///////////////////////////////////////////////////////////////////////////////
//                          Create object instance.                          //
///////////////////////////////////////////////////////////////////////////////

AftCalc_c AftCalc::Create (BOOLEAN_t alloc)
{
   AftCalc_c aftcalc = (AftCalc_c) 0;

   if (!(aftcalc = new AftCalc_t))
      return (AftCalc_c) 0;

   return ((AftCalc_c) aftcalc);
}


///////////////////////////////////////////////////////////////////////////////
//                Delete object instance and any dependents.                 //
///////////////////////////////////////////////////////////////////////////////

void AftCalc::Delete (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   if (((aftcalc)->L0))
   {
      ((aftcalc)->L0)->Delete ();
   }


   delete aftcalc;

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            DeleteModel
 ~ ACCESS:          Private
 ~ FUNCTION:        Remove and delete all the ftsample objects
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

void AftCalc::DeleteModel (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ListDestroyContents (((aftcalc)->FtSamples), TRUE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Finalize
 ~ ACCESS:          Public
 ~ FUNCTION:        Finalize fission track calculations after run:
 ~                  scales the observed and predicted tracklength histograms,  
 ~		    calculates the predicted grain ages, 
 ~                  calculates the chi squared of the length histograms for the calibration
 ~		    
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

void AftCalc::Finalize (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   Project_c project = ((((aftcalc)->CALCENGINE))->PROJECT);
   FtSampleIoTbl_c ftsampletbl;

   if (project->IsOneDimensional ())
   {
      project->GetRefFtSampleIoTbl (&ftsampletbl);
      aftcalc->CalcPredFtAge (ftsampletbl);

      aftcalc->GeotrackCorrPredTracklengthDists ();
      aftcalc->ScaleTracklengthDists ();

      aftcalc->CalcFtLengthChi2 (ftsampletbl);

      aftcalc->MergeTracklengthDists ();
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            FindFtSample
 ~ ACCESS:          Private
 ~ FUNCTION:        Find and return the FtSample object on sample identifier
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ sampleId              STRING       I    Sample identifier
 ~ 
 ~ RETURN VALUE: FtSample_c -> ftsample on succes, NULL otherwise
 ~ ============================================================================


*******************************************************************************/

FtSample_c AftCalc::FindFtSample (STRING sampleId)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      if (!strcmp (((ftsample)->SAMPLECODE), sampleId))
         return (ftsample);
   }
   return (NULL);

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::ForAll_FtSamples (ActionMethod Action, long *arg)
{
   AftCalc_c aftcalc = this;

   {
      ITERATOR_t it;
      FtSample_c member;

      it = ContainerInitNextItem ((Container_c) (aftcalc->FtSamples));
      while (member = (FtSample_c) Next (it))
         Action ((AnyPntr_t) member, arg);
      ListIteratorClear (&it);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

BOOLEAN_t AftCalc::FtSamples_GtFunc (FtSample_c ftsample1, FtSample_c ftsample2)
{

   return (TRUE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcFtP_Chi2
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the P(Chi squared) of the fission track ages 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         This algorithm is supplied by Geotrack and is not documented
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ grainNum              int          I    Number of grains in the sample
 ~ Chi2                  float        I    Chi squared
 ~ P_Chi2                float*       O    P(Chi squared)
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


METHOD  CalcFtP_Chi2
  CFUNC  TRUE
  ARGUMENT grainNum
    TYPE   int
  ARGUMENT Chi2
    TYPE   float
  ARGUMENT P_Chi2
    TYPE   {float*}

DAEMON  REPLACE
CODE
{
   int dp,i;
   float np,l,m,j;
   int dof;
	   
   dof = grainNum-1;
   if ( Chi2 > 100. )
   {
      *P_Chi2 = .0;
   }   
   else
   {
      dp = 1;
      for ( i=dof ; i>=2 ; i=i-2 )
      {
         dp = dp*i;
      }
      np = pow( Chi2,(int)((dof+1)/2)) * exp(-Chi2/2. ) / dp;
      if ( (int)( (float)dof/2. ) == (float)dof/2. ) 
      {
	 j = 1;
      }
      else
      {
	 j = sqrt( 2./Chi2/3.14159);
      }
      l = m = 1.;
      while ( m>=1e-7)   
      {
	 dof += 2;
	 m = m*Chi2/dof;
         l += m;
      }
      *P_Chi2 = 100.*(1.-j*np*l);
   }
}


*******************************************************************************/

float AftCalc::Gauss (float x)
{
// Source file:RBYaftcalc.def                                                //

   float y, z, w;
   float gauss;

   if (Ibscfuncs::FloatEqual (x, 0.))
   {
      z = 0.;
   }
   else
   {
      y = ABS (x) / 2.;
      if (y >= 3.0)
      {
         z = 1.;
      }
      else if (y < 1.)
      {
         w = y * y;
         z = ((((((((0.000124818987 * w
                     - 0.001075204047) * w + 0.005198775019) * w
                   - 0.019198292004) * w + 0.059054035642) * w
                 - 0.151968751364) * w + 0.319152932694) * w
               - 0.531923007300) * w + 0.797884560593) * y * 2.;
      }
      else
      {
         y = y - 2.;
         z = (((((((((((((-0.000045255659 * y
                          + 0.000152529290) * y - 0.000019538132) * y
                        - 0.000676904986) * y + 0.001390604284) * y
                      - 0.000794620820) * y - 0.002034254874) * y
                    + 0.006549791214) * y - 0.010557625006) * y
                  + 0.011630447319) * y - 0.009279453341) * y
                + 0.005353579108) * y - 0.002141268741) * y
              + 0.000535310849) * y + 0.999936657524;
      }
   }
   if (x > 0.)
   {
      gauss = (z + 1.) / 2.;
   }
   else
   {
      gauss = (1. - z) / 2.;
   }
   return (gauss);

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::GeotrackCorrPredTracklengthDists (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      ftsample->GeotrackCorrPredTracklengthDists ();
   }
   aftcalc->ScaleTracklengthDists ();

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetC0
 ~ ACCESS:          Public
 ~ FUNCTION:        Retrieve C0 value from specified index in C0 array
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:      
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ ix                    int          I    index in C0 array
 ~ 
 ~ RETURN VALUE: double -> c0 value
 ~ ============================================================================


*******************************************************************************/

double AftCalc::GetC0 (int ix)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   double c0;

   assert (ix >= 0 && ix < FTBINNUMCLWGHTPERCDISTR);

   ((aftcalc)->C0)->GetValue (ix, &c0);
   return (c0);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetC1
 ~ ACCESS:          Public
 ~ FUNCTION:        Retrieve C1 value from specified index in C0 array
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ ix                    int          I    index in C1 array
 ~ 
 ~ RETURN VALUE: double -> c1 value
 ~ ============================================================================


*******************************************************************************/

double AftCalc::GetC1 (int ix)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   double c1;

   assert (ix >= 0 && ix < FTBINNUMCLWGHTPERCDISTR);

   ((aftcalc)->C1)->GetValue (ix, &c1);
   return (c1);

}

AftCalc * AftCalc::GetCurrent (void)
{
   assert (Current != NULL);
   return Current;
}


/*******************************************************************************
 ~ ============================================================================
 ~ NAME:            GetFtHistogramPlots
 ~ ACCESS:          Public
 ~ FUNCTION:        Retrieve the predicted track length, the observed
 ~                  track length and grain age histograms for display in the GUI 
 ~                  
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type           I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ sampleId              STRING          I    Sample identifier   
 ~ ObsDist               HistogramTbl_c  O    Tabel for observed tracklength distribution 
 ~ PredDist              HistogramTbl_c  O    Tabel for predicted tracklength distribution
 ~ AgeDist               HistogramTbl_c  O    Tabel for grain age distribution
 ~
 ~ RETURN VALUE: BOOLEAN_t -> TRUE on succes, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t AftCalc::GetFtHistogramPlots (STRING sampleId, int ChlorideIx,
                                        HistogramTbl_c ObsDist, HistogramTbl_c
                                        PredDist, int *MaxTrackCount,
                                        HistogramTbl_c AgeDist, int *MaxGrainCount)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   BOOLEAN_t bResult = TRUE;
   double MaxObsBinCnt, MaxPredBinCnt, MaxGrainCnt;
   double MaxTracknum, MaxGrainnum;

   ftsample = aftcalc->FindFtSample (sampleId);
   if (ftsample)
   {
      if (ChlorideIx == FTBINNUMCLWGHTPERCDISTR)  // get the sum of all the Cl wt% bin histograms
      {
         ftsample->GetSumObsTracklengthDist (ObsDist, &MaxObsBinCnt);
         ftsample->GetSumPredTracklengthDist (PredDist, &MaxPredBinCnt);
         ftsample->GetSumGrainAgeDist (AgeDist, &MaxGrainCnt);

         *MaxTrackCount = (int) (MAX (MaxObsBinCnt, MaxPredBinCnt) + 1.);
         *MaxGrainCount = int (MaxGrainCnt + 1.);
      }
      else
      {
         // Display histograms for one Cl wt% bin, take maximum number of 
         // bincounts over the sample

         ftsample->GetObsTracklengthDist (ChlorideIx, ObsDist);
         ftsample->GetPredTracklengthDist (ChlorideIx, PredDist);
         ftsample->GetGrainAgeDist (ChlorideIx, AgeDist);

         ftsample->GetMaxBinCount (&MaxTracknum, &MaxGrainnum);
         *MaxTrackCount = int (MaxTracknum + 1.);

         // *MaxTrackCount = (int) ( MAX(MaxObsBinCnt, MaxPredBinCnt)  + 1.);
         *MaxGrainCount = int (MaxGrainnum + 1.);
      }
   }
   else
   {
      // This is an unwanted feature in current Cauldron UI update mechanism 
      // showerror("AftCalc: GetFtHistogramPlots - Sample %s not found",sampleId);
      bResult = FALSE;
   }
   return (bResult);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetFtSampleCounts
 ~ ACCESS:          Private
 ~ FUNCTION:        Calculates the number of grains and the total number of 
 ~                  spontaneous tracks and total number of induced track of the sample
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name               Type            I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtGrainIoTbl       FtGrainIoTbl_c   I    Fission track grain data 
 ~ sampleId           STRING           I    Sample identifier
 ~ grainNum           int*             O    Number of grains in the sample
 ~ totNs              int*             O    total number of spontaneous tracks in the sample
 ~ totNi              int*             O    total number of induced tracks in the sample
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::GetFtSampleCounts (FtGrainIoTbl_c FtGrainIoTbl, STRING sampleId, int
                                 *grainNum, int *totNs, int *totNi)
{
// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtGrainIoTblRow_c gRow;
   int Ns, Ni;

   *grainNum = 0;
   *totNs = 0;
   *totNi = 0;

   it = FtGrainIoTbl_IteratorInit_FtGrainIoTblRows (FtGrainIoTbl);
   while (gRow = (FtGrainIoTblRow_c) Next (it))
   {
      if (!strcmp (sampleId, FtGrainIoTblRow_FtSampleId (gRow)))
      {
         Ns = FtGrainIoTblRow_FtSpontTrackNo (gRow);
         Ni = FtGrainIoTblRow_FtInducedTrackNo (gRow);

         if (Ns != PROPSYS_INTEGER_NULL && Ni != PROPSYS_INTEGER_NULL &&
             Ni != 0)           // if ( Ni != 0 && Ns != 0 )
         {
            *totNs += Ns;
            *totNi += Ni;

            (*grainNum)++;
         }
      }
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetL0
 ~ ACCESS:          Public
 ~ FUNCTION:        Retrieve L0 value
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:      
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
  ~ 
 ~ RETURN VALUE: double -> L0 value
 ~ ============================================================================


*******************************************************************************/

double AftCalc::GetL0 (int ClIndex)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   float ClPerc;

   ClPerc = FTSTARTCLWGHTPERCDISTR +
         FTSTEPCLWGHTPERCDISTR * (0.5 + (float) ClIndex);
   return (((aftcalc)->L0)->F (ClPerc));

}


///////////////////////////////////////////////////////////////////////////////
//                        Init Object initialization.                        //
///////////////////////////////////////////////////////////////////////////////

void AftCalc::Init (void)
{
   AftCalc_c aftcalc = this;

   AftCalc_LENGTHDISTSCALINGMODE_Set_Mode_SCALETOOBSERVED (aftcalc);
   (aftcalc->TRACKNUMDIST) = 100;

// Source file:RBYaftcalc.def                                                //

   Polyfunction_c polyf;
   int i;
   double ClWtPerc;
   double c0, c1;

   // Make polyfuncttion to interpolate L0

   polyf = ((aftcalc)->L0) = Polyfunction::Make ();

   // Add four tie-points supplied by Geotrack (Cl wt%, L0)

   polyf->AddPoint (.0, 17.0);
   polyf->AddPoint (0.4265, 17.5);
   polyf->AddPoint (0.874, 17.7);
   polyf->AddPoint (5.33, 18.0);

   ((aftcalc)->C0)->Initialize (FTBINNUMCLWGHTPERCDISTR);
   ((aftcalc)->C1)->Initialize (FTBINNUMCLWGHTPERCDISTR);

   // Create polyfunction for c0 
   polyf = Polyfunction::Make ();

   // Add four tie-points supplied by Geotrack (Cl wt%, c0)

   polyf->AddPoint (.0, -3.786);
   polyf->AddPoint (0.4265, -3.016);
   polyf->AddPoint (0.874, -2.713);
   polyf->AddPoint (5.33, -2.328);


   // retrieve interpolated c0 values and store in C0 array
   ClWtPerc = FTSTARTCLWGHTPERCDISTR + 0.5 * FTSTEPCLWGHTPERCDISTR;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      c0 = polyf->F (ClWtPerc);
      ((aftcalc)->C0)->SetValue (i, c0);

      ClWtPerc += FTSTEPCLWGHTPERCDISTR;
   }

   polyf->Delete ();


   // Create polyfunction for c1 
   polyf = Polyfunction::Make ();

   // Add four tie-points supplied by Geotrack (Cl wt%, c1)

   polyf->AddPoint (.0, 1.5756e-4);
   polyf->AddPoint (0.4265, 1.1889e-4);
   polyf->AddPoint (0.874, 1.0237e-4);
   polyf->AddPoint (5.33, 0.8037e-4);

   // retrieve interpolated c1 values and store in C1 array
   ClWtPerc = FTSTARTCLWGHTPERCDISTR + 0.5 * FTSTEPCLWGHTPERCDISTR;
   for (i = 0; i < FTBINNUMCLWGHTPERCDISTR; i++)
   {
      c1 = polyf->F (ClWtPerc);
      ((aftcalc)->C1)->SetValue (i, c1);

      ClWtPerc += FTSTEPCLWGHTPERCDISTR;
   }
   polyf->Delete ();


}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Initialize
 ~ ACCESS:          Public
 ~ FUNCTION:        Initialize the fission track calculation
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         no initialization necessary
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~                                    I    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::Initialize (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //


}


///////////////////////////////////////////////////////////////////////////////
//                       Create and initialize object.                       //
///////////////////////////////////////////////////////////////////////////////

AftCalc_c AftCalc::Make (void)
{
   AftCalc_c aftcalc = (AftCalc_c) 0;

   BOOLEAN_t alloc = FALSE;

   if (!(aftcalc = AftCalc::Create (alloc)))
      return (AftCalc_c) 0;

   return ((AftCalc_c) aftcalc);
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            MakeFtSamples
 ~ ACCESS:          Private
 ~ FUNCTION:        Create and Initialize FtSample objects defined in the FtSampleIoTbl  
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl       FtSampleIoTbl_c    I    Fission track sample data
 ~ FtGrainIoTbl        FtGrainIoTbl_c     I    Fission track grain data
 ~ StratIoTbl          StratIoTbl_c       I    StratIoTbl, used to set location (depth) of samples
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::MakeFtSamples (FtSampleIoTbl_c FtSampleIoTbl, FtGrainIoTbl_c
                             FtGrainIoTbl, FtLengthIoTbl_c FtLengthIoTbl,
                             StratIoTbl_c StratIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   ITERATOR_t it;
   FtSampleIoTblRow_c fRow;
   STRING sampleId;
   double sampleDepth;
   FtSample_c ftsample;

   it = FtSampleIoTbl_IteratorInit_FtSampleIoTblRows (FtSampleIoTbl);
   while (fRow = (FtSampleIoTblRow_c) Next (it))
   {
      // Set sampleId and Depth
      sampleId = FtSampleIoTblRow_FtSampleId (fRow);
      sampleDepth = FtSampleIoTblRow_DepthIndex (fRow);

      // Make and initialize a ftsample
      ftsample = FtSample::Make ();
      ftsample->Initialize (sampleId, sampleDepth, FtGrainIoTbl,
                            FtLengthIoTbl, StratIoTbl);
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::MergeTracklengthDists (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      ftsample->MergeTracklengthDists ();
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Print
 ~ ACCESS:          Public
 ~ FUNCTION:        Print all calculated afta data
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ dummy                 long*             Dummy argument    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::Print (long *arg)
{
   AftCalc_c aftcalc = this;

   {
      ITERATOR_t it = AftCalc_IteratorInit_FtSamples (aftcalc);
      FtSample_c member;

      while (member = (FtSample_c) Next (it))
         member->Print (arg);
   }
}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::Remove_FtSamples (FtSample_c member)
{
   AftCalc_c aftcalc = this;

   ListRemItem ((aftcalc->FtSamples), (AnyPntr_t) member, FALSE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScaleObsTracklengthDists
 ~ ACCESS:          Private
 ~ FUNCTION:        Scale the observed tracklength distributions so that each 
 ~                  contains a total of 'tracknum' tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracknum              int          I    Total number of tracks in the resulting histograms
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::ScaleObsTracklengthDists (int tracknum)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      ftsample->ScaleObsTracklengthDists (tracknum);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScalePredTracklenghtDists
 ~ ACCESS:          Private
 ~ FUNCTION:        Scale the predicted tracklength distributions so that each 
 ~                  contains a total of 'tracknum' tracks
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ tracknum              int          I    Total number of tracks in the resulting histograms
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::ScalePredTracklenghtDists (int tracknum)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      ftsample->ScalePredTracklenghtDists (tracknum);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            ScalePredTracklengthsToObserved
 ~ ACCESS:          Private
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
  ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::ScalePredTracklengthsToObserved (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   FtSample_c ftsample;
   ITERATOR_t it;

   it = (ListInitNextItem (((aftcalc)->FtSamples)));
   while (ftsample = (FtSample_c) Next (it))
   {
      ftsample->ScalePredTracklengthsToObserved ();
   }

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void AftCalc::ScaleTracklengthDists (void)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   if (((aftcalc)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_SCALETOOBSERVED))
   {
      aftcalc->ScalePredTracklengthsToObserved ();
   }
   else if (((aftcalc)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_SCALETOTRACKNUMDIST))
   {
      aftcalc->ScalePredTracklenghtDists (((aftcalc)->TRACKNUMDIST));
      aftcalc->ScaleObsTracklengthDists (((aftcalc)->TRACKNUMDIST));
   }
   else if (((aftcalc)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_NOSCALING))
   {
      // no scaling
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SetFtIoTbls
 ~ ACCESS:          Public
 ~ FUNCTION:        Create ftsample and histogram objects from FtSampleIoTbl and FtGrainIoTbl
 ~                  Bin observed track lengths from FtLengthIoTbl
 ~                  Calculate and bin fission track grain ages 
 ~                  Calculate data sheet items (Pooled age, Chi squared Corr coeff etc. ) 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                Type              I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ FtSampleIoTbl       FtSampleIoTbl_c    I    Fission track sample data
 ~ FtGrainIoTbl        FtGrainIoTbl_c     I    Fission track grain data
 ~ FtLengthIoTbl       FtLengthIoTbl_c    I    Fission track length data
 ~ StratIoTbl          StratIoTbl_c       I    StratIoTbl, used to set location (depth) of samples
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void AftCalc::SetFtIoTbls (FtSampleIoTbl_c FtSampleIoTbl, FtGrainIoTbl_c
                           FtGrainIoTbl, FtLengthIoTbl_c FtLengthIoTbl,
                           StratIoTbl_c StratIoTbl)
{
   AftCalc_c aftcalc = this;

// Source file:RBYaftcalc.def                                                //

   // Make FtSample objects
   aftcalc->MakeFtSamples (FtSampleIoTbl, FtGrainIoTbl, FtLengthIoTbl, StratIoTbl);

   // Bin track lengths
   aftcalc->BinFtLengths (FtSampleIoTbl, FtLengthIoTbl, FtGrainIoTbl);

   // Calculate fission track ages of the grains
   aftcalc->CalcFtGrainAges (FtSampleIoTbl, FtGrainIoTbl);

   // Bin the calculated fission track grain ages 
   aftcalc->BinFtGrainAges (FtGrainIoTbl);

   // Calculate all items of the data sheet
   aftcalc->CalcFtSampleData (FtSampleIoTbl, FtGrainIoTbl);


}

///////////////////////////////////////////////////////////////////////////////
//                 Destroy object instance (C++ destructor).                 //
///////////////////////////////////////////////////////////////////////////////

AftCalc::~AftCalc (void)
{
   AftCalc_c aftcalc = this;

   if ((aftcalc->C1))
      PropDoubleVector_Delete ((aftcalc->C1));
   if ((aftcalc->C0))
      PropDoubleVector_Delete ((aftcalc->C0));
   if ((aftcalc->FtSamples))
   {
      AftCalc_ForEach_FtSamples (aftcalc, (ftsample->AFTCALC) = 0);
      ListDestroy ((aftcalc->FtSamples), TRUE);
   }
   Current = NULL;
}

AftCalc * AftCalc::Current = NULL;
