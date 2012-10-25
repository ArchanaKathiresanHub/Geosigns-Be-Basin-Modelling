/*******************************************************************************
* Class:       Histogram
*
* File:        histogram.C
*
* Copyright:   (c) 1997 Shell International Exploration and Production
*
* GeoCase rev: v1.0
*
* Description: 

 ~ ===========================================================================
 ~
 ~  Class Name:       Histogram
 ~  Subclass of:  
 ~  Description:      Histogram
 ~  Attributes:
 ~    Name              type             Description
 ~    -------------------------------------------------------------------------
 ~    BINSTART          float            Start value of first bin
 ~    BINWIDTH          float            Width of bins
 ~    BINNUM            int              Number of bins 
 ~    BIN               PropDoubleVector Vector with bins, type is double to allow 
 ~                                       taking fractional parts of bin contents
 ~
 ~ ============================================================================


*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                             HISTOGRAM Methods                             //
///////////////////////////////////////////////////////////////////////////////
// This object is a member of the module RUBYPROJECT in project RBYPROJECT   //
///////////////////////////////////////////////////////////////////////////////
#include   <stdio.h>
#include   <string.h>
#include   "DDSall.h"
#define  histogram__file__name  1
#include   "rbyproject.h"
#include   "histogram.h"
#include   "histogramP.h"

#include   "propdoublevector.h"
#include   "ibscfuncs.h"

///////////////////////////////////////////////////////////////////////////////
//                  Private and Static method declarations.                  //
///////////////////////////////////////////////////////////////////////////////

void Histogram_AddBinFractions (Histogram_c histogram, Histogram_c source, float
                                factor)
{
   histogram->Histogram::AddBinFractions (source, factor);
}
void Histogram_AddBins (Histogram_c histogram, Histogram_c source)
{
   histogram->Histogram::AddBins (source);
}
double Histogram_BinCount (Histogram_c histogram, int binIx)
{
   return histogram->Histogram::BinCount (binIx);
}
float CalcChi2 (Histogram_c predHist, Histogram_c obsHist)
{
   return Histogram::CalcChi2 (predHist, obsHist);
}
void Histogram_CopyBin (Histogram_c histogram, double *newbin)
{
   histogram->Histogram::CopyBin (newbin);
}
Histogram_c Histogram_Create (BOOLEAN_t alloc)
{
   return Histogram::Create (alloc);
}
void Histogram_Delete (Histogram_c histogram)
{
   histogram->Histogram::Delete ();
}
void Histogram_FillHistogramTbl (Histogram_c histogram, HistogramTbl_c HistogramTbl)
{
   histogram->Histogram::FillHistogramTbl (HistogramTbl);
}
BOOLEAN_t Histogram_FindAndIncrementBin (Histogram_c histogram, float binVal)
{
   return histogram->Histogram::FindAndIncrementBin (binVal);
}
BOOLEAN_t Histogram_FindBin (Histogram_c histogram, float binVal, int *binIx)
{
   return histogram->Histogram::FindBin (binVal, binIx);
}
double Histogram_GetMaxBinCount (Histogram_c histogram)
{
   return histogram->Histogram::GetMaxBinCount ();
}
double Histogram_GetMean (Histogram_c histogram)
{
   return histogram->Histogram::GetMean ();
}
void Histogram_IncrementBin (Histogram_c histogram, int binIx)
{
   histogram->Histogram::IncrementBin (binIx);
}
void Histogram_Init (Histogram_c histogram)
{
   histogram->Histogram::Init ();
}
void Histogram_Initialize (Histogram_c histogram, float binstart, float
                           binwidth, int binnum)
{
   histogram->Histogram::Initialize (binstart, binwidth, binnum);
}
BOOLEAN_t JoinBins (Histogram_c master, Histogram_c slave, Histogram_c
                    masterRes, Histogram_c slaveRes)
{
   return Histogram::JoinBins (master, slave, masterRes, slaveRes);
}
Histogram_c Histogram_Make (void)
{
   return Histogram::Make ();
}
void Histogram_MergeBins (Histogram_c histogram, int numbin)
{
   histogram->Histogram::MergeBins (numbin);
}
void Histogram_MulBinCount (Histogram_c histogram, int binIx, double
                            scalefactor)
{
   histogram->Histogram::MulBinCount (binIx, scalefactor);
}
void Histogram_MulBins (Histogram_c histogram, double scalefactor)
{
   histogram->Histogram::MulBins (scalefactor);
}
void Histogram_Print (Histogram_c histogram, long *arg)
{
   histogram->Histogram::Print (arg);
}
void Histogram_PrintBinHeader (Histogram_c histogram)
{
   histogram->Histogram::PrintBinHeader ();
}
void Histogram_PrintBins (Histogram_c histogram)
{
   histogram->Histogram::PrintBins ();
}
void Histogram_Scale (Histogram_c histogram, int totBinCnt)
{
   histogram->Histogram::Scale (totBinCnt);
}
void Histogram_SetBinCount (Histogram_c histogram, int binIx, double binVal)
{
   histogram->Histogram::SetBinCount (binIx, binVal);
}
void Histogram_SumBinCount (Histogram_c histogram, int binIx, double sumValue)
{
   histogram->Histogram::SumBinCount (binIx, sumValue);
}
double Histogram_SumBins (Histogram_c histogram, int startIx, int endIx)
{
   return histogram->Histogram::SumBins (startIx, endIx);
}
///////////////////////////////////////////////////////////////////////////////
//                      Member function Implementation                       //
///////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
Undocumented.
*******************************************************************************/

void Histogram::AddBinFractions (Histogram_c source, float factor)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   double binVal;

   for (i = 0; i < (BINNUM); i++)
   {
      if (i < ((source)->BINNUM))
         binVal = source->BinCount (i);
      else
         binVal = .0;
      SumBinCount (i, factor * binVal);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            AddBins
 ~ ACCESS:          Public
 ~ FUNCTION:        Sums the bincounts of histogram and source into histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ source                Histogram_c  I    
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::AddBins (Histogram_c source)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   double binVal;

   for (i = 0; i < (BINNUM); i++)
   {
      if (i < ((source)->BINNUM))
         binVal = source->BinCount (i);
      else
         binVal = .0;
      SumBinCount (i, binVal);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            BinCount
 ~ ACCESS:          Public
 ~ FUNCTION:        Get the bin count of a specified bin
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binIx                 int          I    index of bin 
 ~ 
 ~ RETURN VALUE: double -> bincount
 ~ ============================================================================


*******************************************************************************/

double Histogram::BinCount (int binIx)
{
   Histogram_c histogram = this;

// Source file:RBYaftcalc.def                                                //

   double binVal;

   assert (binIx >= 0 && binIx < (BINNUM));

   (BIN)->GetValue (binIx, &binVal);
   return (binVal);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CalcChi2
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculate the chi squared value of a predicted and observed histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ predHist              Histogram_c  I    Predicted distribution 
 ~ obsHist               Histogram_c  I    Observed distribution 
 ~ 
 ~ RETURN VALUE: float -> chi squared
 ~ ============================================================================


*******************************************************************************/

float Histogram::CalcChi2 (Histogram_c predHist, Histogram_c obsHist)
{
// Source file:RBYaftcalc.def                                                //

   Histogram_c obsJntHist, predJntHist;
   int i;
   float chi2;
   double oVal, pVal;

   assert (FloatEqual (((predHist)->BINSTART),
                                  ((obsHist)->BINSTART)) &&
           FloatEqual (((predHist)->BINWIDTH),
                                  ((obsHist)->BINWIDTH)) &&
           ((predHist)->BINNUM) == ((obsHist)->BINNUM));


   // First reorganize the thiss so that non of the bins of
   // the master this is empty.

   obsJntHist = Histogram::Make ();
   predJntHist = Histogram::Make ();

   chi2 = .0;

   if (Histogram::JoinBins (obsHist, predHist, obsJntHist, predJntHist))
   {
      for (i = 0; i < ((obsJntHist)->BINNUM); i++)
      {
         oVal = obsJntHist->BinCount (i);
         pVal = predJntHist->BinCount (i);

         chi2 += (pVal - oVal) * (pVal - oVal) / oVal;
      }
   }

   obsJntHist->Delete ();
   predJntHist->Delete ();

   return (chi2);

}

Histogram_c Histogram::CopyYourself(void )
{
   Histogram_c src = this;
   Histogram_c dest = (Histogram_c) 0;

   if (!(dest = Histogram_Create())) return (Histogram_c)0;

   src->CopyContentsTo (dest);
   
   return dest;
}
void Histogram::CopyContentsTo (Histogram_c dest)
{
   dest->BINSTART = BINSTART;
   dest->BINWIDTH = BINWIDTH;
   dest->BINNUM = BINNUM;

   PropDoubleVector_CopyContents(BIN, dest->BIN);
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            CopyBin
 ~ ACCESS:          Public
 ~ FUNCTION:        Copies the contents of an array into the bins of the histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: number of items in the array is at least the number of bins 
 ~                  in the histogram
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ newbin                double*      I    array to copy in histogram bins
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::CopyBin (double *newbin)
{


// Source file:RBYaftcalc.def                                                //

   int i;

   for (i = 0; i < (BINNUM); i++)
   {
      SetBinCount (i, newbin[i]);
   }

}


///////////////////////////////////////////////////////////////////////////////
//                          Create object instance.                          //
///////////////////////////////////////////////////////////////////////////////

Histogram_c Histogram::Create (BOOLEAN_t alloc)
{
   Histogram_c histogram = (Histogram_c) 0;


   if (!(histogram = new Histogram_t))
      return (Histogram_c) 0;

   return ((Histogram_c) histogram);
}


///////////////////////////////////////////////////////////////////////////////
//                Delete object instance and any dependents.                 //
///////////////////////////////////////////////////////////////////////////////

void Histogram::Delete (void)
{


   delete this;
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            FillHistogramTbl
 ~ ACCESS:          Public
 ~ FUNCTION:        Fill the HistogramTbl with the contents of the Histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type           I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ HistogramTbl          HistogramTbl_c  O    resulting histogramtable 
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::FillHistogramTbl (HistogramTbl_c HistogramTbl)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   double binstart, binend;
   HistogramTblRow_c hRow;

   binstart = (BINSTART);
   binend = binstart + (BINWIDTH);

   for (i = 0; i < (BINNUM); i++)
   {
      hRow = HistogramTblRow_Make ();

      HistogramTblRow_BinStart (hRow) = binstart;
      HistogramTblRow_BinEnd (hRow) = binend;
      HistogramTblRow_BinCount (hRow) = BinCount (i);

      HistogramTbl_Add_HistogramTblRows (HistogramTbl, hRow);

      binstart = binend;
      binend += (BINWIDTH);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            FindAndIncrementBin
 ~ ACCESS:          Public
 ~ FUNCTION:        Find the bin in which binVal falls and increment the bincount
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binVal                double       I    value select bin  
 ~ 
 ~ RETURN VALUE: BOOLEAN_t -> TRUE if bin is found, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t Histogram::FindAndIncrementBin (float binVal)
{


// Source file:RBYaftcalc.def                                                //

   int binIx;

   if (FindBin (binVal, &binIx))
   {
      IncrementBin (binIx);
      return (TRUE);
   }
   return (FALSE);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            FindBin
 ~ ACCESS:          Public
 ~ FUNCTION:        Find the bin in which binVal falls
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binVal                float        I    value to select bin on
 ~ binIx                 int*         O    index of bin in which binVal falls
 ~
 ~ RETURN VALUE: BOOLEAN_t -> TRUE if bin is found, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t Histogram::FindBin (float binVal, int *binIx)
{


// Source file:RBYaftcalc.def                                                //

   *binIx = int ((binVal - (BINSTART)) / (BINWIDTH));

   return (*binIx >= 0 && *binIx < (BINNUM));

}


/*******************************************************************************
Undocumented.
*******************************************************************************/

double Histogram::GetMaxBinCount (void)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   double MaxBinCnt = 0.;

   for (i = 0; i < (BINNUM); i++)
   {
      MaxBinCnt = MAX (MaxBinCnt, BinCount (i));
   }
   return (MaxBinCnt);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            GetMean
 ~ ACCESS:          Public
 ~ FUNCTION:        Calculate the mean value of the histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~                                    I    
 ~ 
 ~ RETURN VALUE: double -> mean 
 ~ ============================================================================


*******************************************************************************/

double Histogram::GetMean (void)
{


// Source file:RBYaftcalc.def                                                //

   double binVal, binCenter;
   double mean, binValSum;
   int i;

   binCenter = (BINSTART) + 0.5 * (BINWIDTH);

   binValSum = 0;
   mean = .0;
   for (i = 0; i < (BINNUM); i++)
   {
      binVal = BinCount (i);
      if (!DoubleEqual (binVal, .0))
      {
         mean += binCenter * binVal;
         binValSum += binVal;
      }
      binCenter += (BINWIDTH);
   }
   if (!DoubleEqual (binValSum, .0))
   {
      mean /= binValSum;
   }
   return (mean);

}


///////////////////////////////////////////////////////////////////////////////
//                 Create object instance (C++ constructor).                 //
///////////////////////////////////////////////////////////////////////////////

Histogram::Histogram (BOOLEAN_t alloc)
{


   if (!((BIN) = PropDoubleVector::Create (alloc)))
   {
      Delete ();
      return;
   }
   Init ();
}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            IncrementBin
 ~ ACCESS:          Public
 ~ FUNCTION:        Increment the bin with index binIx 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binIx                 int          I    index of bin to increment
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::IncrementBin (int binIx)
{


// Source file:RBYaftcalc.def                                                //

   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal + 1.);

}


///////////////////////////////////////////////////////////////////////////////
//                        Init Object initialization.                        //
///////////////////////////////////////////////////////////////////////////////

void Histogram::Init (void)
{


   (BINSTART) = 0.000000;
   (BINWIDTH) = 1.000000;
   (BINNUM) = 50;

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Initialize
 ~ ACCESS:          Public
 ~ FUNCTION:        Initialize histogram object
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binstart              float        I    startvalue of the first bin
 ~ binwidth              float        I    width of the bins
 ~ binnum                int          I    number of bins
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::Initialize (float binstart, float binwidth, int binnum)
{


// Source file:RBYaftcalc.def                                                //

   (BINSTART) = binstart;
   (BINWIDTH) = binwidth;
   (BINNUM) = binnum;

   // (Re)alloc the binvector
   (BIN)->Initialize (binnum);

   // Set all bincounts to zero
   (BIN)->Reset (.0);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            JoinBins
 ~ ACCESS:          Private
 ~ FUNCTION:        The bins of the master histogram are joined so that no bin is empty
 ~                  the result is stored in masterRes. The same bins of the slave histogram 
 ~                  are joined as well into slaveRes, so that he binstart, binwidth and 
 ~                  binnum of masterRes and slaveRes are equal. This operation is necessary
 ~                  for the calculation of Chi squared of two histograms
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         CFUNC
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ master                Histogram_c  I    master histogram
 ~ slave                 Histogram_c  I    slave histogram
 ~ masterRes             Histogram_c  O    resulting master histogram
 ~ slaveRes              Histogram_c  O    resulting slave histogram
 ~ 
 ~ RETURN VALUE: BOOLEAN_t -> TRUE on succes, FALSE otherwise
 ~ ============================================================================


*******************************************************************************/

BOOLEAN_t Histogram::JoinBins (Histogram_c master, Histogram_c slave,
                               Histogram_c masterRes, Histogram_c slaveRes)
{
// Source file:RBYaftcalc.def                                                //

   int i, iPrev;
   int j, k, iRes, iM;
   int binNumRes;
   double mVal, sVal;

   // Determine binnum of resulting thiss 

   binNumRes = 0;
   for (i = 0; i < ((master)->BINNUM); i++)
   {
      mVal = master->BinCount (i);

      if (!DoubleEqual (mVal, .0))
      {
         binNumRes++;
      }
   }

   if (binNumRes > 0)
   {
      // Initialize resulting thiss
      // Binwidth is not the same for the bins, therefore it is not set.

      masterRes->Initialize (((master)->BINSTART),
                             PROPSYS_FLOAT_NULL, binNumRes);
      slaveRes->Initialize (((master)->BINSTART),
                            PROPSYS_FLOAT_NULL, binNumRes);


      // Do the join

      for (k = 0; k < ((master)->BINNUM) &&
           DoubleEqual (master->BinCount (k), .0); k++)
      {
         sVal = slave->BinCount (k);
         slaveRes->SumBinCount (0, sVal);
      }

      iPrev = k;
      iRes = 0;

      for (i = k; i < ((master)->BINNUM); i++)
      {
         mVal = master->BinCount (i);

         if (!DoubleEqual (mVal, .0))
         {
            masterRes->SetBinCount (iRes, mVal);

            iM = iPrev + (i - iPrev) / 2;

            for (j = iPrev; j < i; j++)
            {
               sVal = slave->BinCount (j);
               if (j > iM)
               {
                  slaveRes->SumBinCount (iRes, sVal);
               }
               else if (j < iM || (i - iPrev) % 2 == 1)
               {
                  slaveRes->SumBinCount (iRes - 1, sVal);
               }
               else             // {j==iM && (i-iPrev)%2==0}
               {
                  slaveRes->SumBinCount (iRes - 1, 0.5 * sVal);
                  slaveRes->SumBinCount (iRes, 0.5 * sVal);
               }
            }
            iRes++;
            iPrev = i;
         }
      }

      for (i = iPrev; i < ((slave)->BINNUM); i++)
      {
         sVal = slave->BinCount (i);
         slaveRes->SumBinCount (iRes - 1, sVal);
      }

      return (TRUE);
   }
   return (FALSE);


}


///////////////////////////////////////////////////////////////////////////////
//                       Create and initialize object.                       //
///////////////////////////////////////////////////////////////////////////////

Histogram_c Histogram::Make (void)
{
   Histogram_c histogram = (Histogram_c) 0;

   BOOLEAN_t alloc = FALSE;

   if (!(histogram = Histogram::Create (alloc)))
      return (Histogram_c) 0;

   return ((Histogram_c) histogram);
}


/*******************************************************************************
Undocumented.
*******************************************************************************/

void Histogram::MergeBins (int numbin)
{


// Source file:RBYaftcalc.def                                                //

   Histogram_c old;
   int i, inext, binnum;
   double sum;

   if (numbin > 1)
   {

      old = CopyYourself ();

      binnum = ((old)->BINNUM) / numbin;
      if (((old)->BINNUM) % numbin > 0)
         binnum++;

      Initialize (((old)->BINSTART),
                  numbin * ((old)->BINWIDTH),
                  binnum);

      for (i = 0; i < binnum; i++)
      {
         inext = MIN (((old)->BINNUM), (i + 1) * numbin);
         sum = old->SumBins (i * numbin, inext - 1);
         SetBinCount (i, sum);
      }
      old->Delete ();
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            MulBinCount
 ~ ACCESS:          Private
 ~ FUNCTION:        Scale the bincount of bin[binIx]
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binIx                 int          I    index of bin to scale
 ~ scalefactor           double       I    Scaling factor
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::MulBinCount (int binIx, double scalefactor)
{


// Source file:RBYaftcalc.def                                                //

   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal * scalefactor);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            MulBins
 ~ ACCESS:          Public
 ~ FUNCTION:        Scale all bins of the histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ scalefactor           double       I    Scaling factor
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::MulBins (double scalefactor)
{


// Source file:RBYaftcalc.def                                                //

   int i;

   for (i = 0; i < (BINNUM); i++)
   {
      MulBinCount (i, scalefactor);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Print
 ~ ACCESS:          Public
 ~ FUNCTION:        Print the histogram
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ arg                   long*        I    dummy
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::Print (long *arg)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   double binVal;
   float startbin, endbin;

   for (i = 0; i < (BINNUM); i++)
   {
      startbin = (BINSTART) + i * (BINWIDTH);
      binVal = BinCount (i);

      printf ("%7.2f         %20.10f\n", startbin, binVal);
   }
   endbin = (BINSTART) +
         (BINNUM) * (BINWIDTH);
   printf ("%7.2f\n\n\n", endbin);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            PrintBinHeader
 ~ ACCESS:          Public
 ~ FUNCTION:        Print the header of the histogram
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

void Histogram::PrintBinHeader (void)
{


// Source file:RBYaftcalc.def                                                //

   int i;
   float binboundary;

   binboundary = (BINSTART);
   for (i = 0; i < (BINNUM); i++)
   {
      printf ("%6.1f ", binboundary);
      binboundary += (BINWIDTH);
   }
   printf ("\n");

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            PrintBins
 ~ ACCESS:          Public
 ~ FUNCTION:        Print the bincounts of the bins
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

void Histogram::PrintBins (void)
{
// Source file:RBYaftcalc.def                                                //

   int i;
   double binVal;

   for (i = 0; i < (BINNUM); i++)
   {
      binVal = BinCount (i);

      if (DoubleEqual (.0, binVal))
         printf ("       ");
      else
         printf ("%6.0f ", binVal);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            Scale
 ~ ACCESS:          Public
 ~ FUNCTION:        Resize the bins to a total of totBinCnt 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ totBinCnt             int          I    total bincount in the resulting histogram
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::Scale (int totBinCnt)
{
// Source file:RBYaftcalc.def                                                //

   double sum;
   float factor;

   sum = SumBins (0, (BINNUM) - 1);

   if (!DoubleEqual (sum, .0))
   {
      factor = (float) totBinCnt / sum;
      MulBins (factor);
   }

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SetBinCount
 ~ ACCESS:          Public
 ~ FUNCTION:        assign binVal to bin[BinIx] 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binIx                 int          I    index of bin 
 ~ binVal                double       I    value to assign to bin[BinIx] 
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::SetBinCount (int binIx, double binVal)
{
// Source file:RBYaftcalc.def                                                //

   assert (binIx >= 0 && binIx < (BINNUM));

   (BIN)->SetValue (binIx, binVal);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SumBinCount
 ~ ACCESS:          Public
 ~ FUNCTION:        Add sumValue to the bincount with index binIx 
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ binIx                 int          I    index of bin to increment
 ~ sumValue              double       I    value to add to bin[binI]
 ~ 
 ~ RETURN VALUE: No
 ~ ============================================================================


*******************************************************************************/

void Histogram::SumBinCount (int binIx, double sumValue)
{
// Source file:RBYaftcalc.def                                                //

   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal + sumValue);

}


/*******************************************************************************


 ~ ============================================================================
 ~ NAME:            SumBins
 ~ ACCESS:          Public
 ~ FUNCTION:        Sums the bins of  the histogram between startIx and endIx
 ~ PRE-CONDITIONS:  none
 ~ POST-CONDITIONS: none
 ~ REMARKS:         none
 ~ PARAMETERS:
 ~   
 ~ Name                  Type        I/O   Description
 ~ ----------------------------------------------------------------------------
 ~ startIx               int          I    start index
 ~ endIx                 int          I    end index
 ~ 
 ~ RETURN VALUE: double -> sum of the bincounts
 ~ ============================================================================


*******************************************************************************/

double Histogram::SumBins (int startIx, int endIx)
{
// Source file:RBYaftcalc.def                                                //

   int i;
   double binVal;
   double sum = .0;

   assert (startIx >= 0 && startIx < (BINNUM) &&
           endIx >= 0 && endIx < (BINNUM));

   for (i = startIx; i <= endIx; i++)
   {
      binVal = BinCount (i);
      sum += binVal;
   }
   return (sum);

}


///////////////////////////////////////////////////////////////////////////////
//                 Destroy object instance (C++ destructor).                 //
///////////////////////////////////////////////////////////////////////////////

Histogram::~Histogram (void)
{

   if ((BIN))
      PropDoubleVector_Delete ((BIN));
}
