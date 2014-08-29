#include "histogram.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

namespace Calibration
{

namespace FissionTracks
{
	// Declare some static variable
	const double Histogram::IBSNULLVALUE = -9999;

/// add to each bin value a fraction of corresponding source
void Histogram::AddBinFractions (Histogram* source, float fraction)
{
   int i;
   double binVal;

   for (i = 0; i < (BINNUM); i++)
   {
      if (i < ((source)->BINNUM))
         binVal = source->BinCount (i);
      else
         binVal = 0.0;
      SumBinCount (i, fraction * binVal);
   }
}


/// Sums the bincounts of histogram and source into histogram
void Histogram::AddBins (Histogram* source)
{
   int i;
   double binVal;

   for (i = 0; i < BINNUM; i++)
   {
      if (i < (source->BINNUM))
         binVal = source->BinCount (i);
      else
         binVal = 0.0;
      SumBinCount (i, binVal);
   }

}


///  Get the bin count (=bin value) of a specified bin
double Histogram::BinCount (int binIx)
{
   using namespace std; 

   double binVal;

   assert ( ( binIx  >= 0 ) && ( binIx < BINNUM ) );

   binVal = BIN[binIx];
   return binVal;

}


/// Calculate the chi squared value of a observed and predicted histogram
float CalcChi2 (Histogram* predHist, Histogram* obsHist)
{
   using namespace std;

   int i;
   float chi2;
   double oVal, pVal;

   assert ( ( predHist->BINSTART  ==  obsHist->BINSTART ) &&
            ( predHist->BINWIDTH == obsHist->BINWIDTH ) &&
            ( predHist->BINNUM == obsHist->BINNUM )  );

   // First reorganize the histograms ("join") so that non of the bins of
   // the master (=obsHist) is empty.

   Histogram obsJntHist; 
   Histogram predJntHist;

   chi2 = 0.0;

   if ( JoinBins (obsHist, predHist, &obsJntHist, &predJntHist) )
   {
      for (i = 0; i < obsJntHist.BINNUM; i++)
      {
         oVal = obsJntHist.BinCount (i);
         pVal = predJntHist.BinCount (i);

         chi2 += (pVal - oVal) * (pVal - oVal) / oVal;
      }
   }

   return (chi2);
}

//copy contents of double array into histogram's bins
void Histogram::CopyBin (double *newbin)
{
   int i;

   for (i = 0; i < (BINNUM); i++)
   {
      SetBinCount (i, newbin[i]);
   }
}

//copy contents of double array into histogram's bins
void Histogram::CopyBin (const std::vector<double>& newbin)
{
   int i;
   
   assert( newbin.size() >= (unsigned int) BINNUM );

   for (i = 0; i < (BINNUM); i++)
   {
      SetBinCount (i, newbin[i]);
   }
}



// //Fill the HistogramTbl with the contents of the Histogram
// void Histogram::FillHistogramTbl (HistogramTbl_c HistogramTbl)
// {


// // Source file:RBYaftcalc.def                                                //

//    int i;
//    double binstart, binend;
//    HistogramTblRow_c hRow;

//    binstart = (BINSTART);
//    binend = binstart + (BINWIDTH);

//    for (i = 0; i < (BINNUM); i++)
//    {
//       hRow = HistogramTblRow_Make ();

//       HistogramTblRow_BinStart (hRow) = binstart;
//       HistogramTblRow_BinEnd (hRow) = binend;
//       HistogramTblRow_BinCount (hRow) = BinCount (i);

//       HistogramTbl_Add_HistogramTblRows (HistogramTbl, hRow);

//       binstart = binend;
//       binend += (BINWIDTH);
//    }

// }


// Find the bin in which binVal falls and increment the bincount
bool Histogram::FindAndIncrementBin (float binVal)
{
   int binIx;

   if (FindBin (binVal, &binIx))
   {
      IncrementBin (binIx);
      return true;
   }
   return false;
}


//Find the bin in which binVal falls
bool Histogram::FindBin (float binVal, int *binIx)
{
   *binIx = int ((binVal - (BINSTART)) / (BINWIDTH));

   return (*binIx >= 0 && *binIx < (BINNUM));
}


//public access
float Histogram::getBinStart()
{
   return BINSTART;
}


//public access
float Histogram::getBinWidth()
{
   return BINWIDTH;
}


//public access
int Histogram::getBinNum()
{
   return BINNUM;
}


//find maximum bin count in histogram
double Histogram::GetMaxBinCount (void)
{
   int i;
   double MaxBinCnt = 0.;

   for (i = 0; i < (BINNUM); i++)
   {
      MaxBinCnt = std::max (MaxBinCnt, BinCount (i));
   }

   return (MaxBinCnt);
}


// Calculate the mean value of the histogram
double Histogram::GetMean (void)
{
   double binVal, binCenter;
   double mean, binValSum;
   int i;

   binCenter = (BINSTART) + 0.5 * (BINWIDTH);

   binValSum = 0;
   mean = .0;
   for (i = 0; i < (BINNUM); i++)
   {
      binVal = BinCount (i);
      if (! (binVal == 0.0) )
      {
         mean += binCenter * binVal;
         binValSum += binVal;
      }
      binCenter += (BINWIDTH);
   }
   if (! (binValSum == 0.0) )
   {
      mean /= binValSum;
   }
   return (mean);

}

//constructor
Histogram::Histogram ()
{
   Init ();
}


//Increment the bin with index binIx 
void Histogram::IncrementBin (int binIx)
{
   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal + 1.);
}


//default initialization of object
//!!! substitute by Initialize() with default arguments
void Histogram::Init()
{
   BINSTART = 0.000000;
   BINWIDTH = 1.000000;
   BINNUM = 50;
}


// Initialize histogram object
void Histogram::Initialize (float binstart, float binwidth, int binnum)
{
   BINSTART = binstart;
   BINWIDTH = binwidth;
   BINNUM = binnum;

   BIN.clear();
   BIN.resize(binnum, 0.0);
}


// The bins of the master histogram are joined so that no bin is empty
// the result is stored in masterRes. The same bins of the slave histogram 
// are joined as well into slaveRes, so that the binstart, binwidth and 
// binnum of masterRes and slaveRes are equal. This operation is necessary
// for the calculation of Chi squared of two histograms
bool JoinBins (Histogram* master, Histogram* slave, Histogram* masterRes, Histogram* slaveRes)
{
   int i, iPrev;
   int j, k, iRes, iM;
   int binNumRes;
   double mVal, sVal;

   // Determine binnum of resulting histograms
   // (=number of non-empty master bins)
   binNumRes = 0;
   for (i = 0; i < master->BINNUM; i++)
   {
      mVal = master->BinCount (i);

      if (! (mVal == 0.0))
      {
         binNumRes++;
      }
   }

   if (binNumRes > 0)
   {
      // Initialize resulting histogram
      // Binwidth is not the same for the bins, therefore it is not set.

      masterRes-> Initialize (((master)->BINSTART),
                              Histogram::IBSNULLVALUE, binNumRes);
      slaveRes -> Initialize (((master)->BINSTART),
                              Histogram::IBSNULLVALUE, binNumRes);


      // Do the join

      //find first non-empty master bin k and sum up slave bins up to "master k" in "slave 0"
      for (k = 0; (  ( k < master->BINNUM ) &&
                     ( master->BinCount (k) == 0.0 ) ); k++)
      {
         sVal = slave->BinCount (k);
         slaveRes->SumBinCount (0, sVal);
      }

      iPrev = k;
      iRes = 0;

      for (i = k; i < master->BINNUM ; i++)
      {
         mVal = master->BinCount (i);

         if (! (mVal == 0.0) )
         {
            //set master result bin (for master bincounts > 0), and... 
            masterRes->SetBinCount (iRes, mVal);


            //...distribute values of corresponding slave bin 
            //and those slave bins related to previous zero entries of master (not counted,yet) 
            //on slave bins iRes-1 and iRes            

            iM = iPrev + (i - iPrev) / 2; //center iM of not yet counted slave bins j. j<iM to be added to iRes-1, j>iM to iRes  

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

      return true;
   }
   return false;
}


//Merge contents (=sum up) of each subsequent numbin bins into one
void Histogram::MergeBins (int numbin)
{
   Histogram old;
   int i, inext, binnum;
   double sum;

   if (numbin > 1)
   {
      old = (*this);

      //get the number of new bins (in case of fractional part add one more bin)
      binnum = old.BINNUM / numbin;
      if (old.BINNUM % numbin > 0)
         binnum++;

      this->Initialize (old.BINSTART,
                        numbin * old.BINWIDTH,
                        binnum);

      //merge contents
      for (i = 0; i < binnum; i++)
      {
         inext = std::min (old.BINNUM, (i + 1) * numbin);
         sum = old.SumBins (i * numbin, inext - 1);
         SetBinCount (i, sum);
      }
   }
}


// scale the bincount of bin[binIx]
void Histogram::MulBinCount (int binIx, double scalefactor)
{
   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal * scalefactor);
}


// scale all bins of the histogram
void Histogram::MulBins (double scalefactor)
{
   int i;

   for (i = 0; i < (BINNUM); i++)
   {
      MulBinCount (i, scalefactor);
   }
}


// Print the histogram
void Histogram::Print ()
{
   using namespace std;

   int i;
   double binVal;
   float startbin, endbin;

   cout.setf(ios::fixed,ios::floatfield);

   for (i = 0; i < BINNUM; i++)
   {
      startbin = (BINSTART) + i * (BINWIDTH);
      binVal = BinCount (i);

      cout << setw(7) << setprecision(2) << startbin << "         " 
           << setw(20) << setprecision(10) << binVal << endl;
      //printf ("%7.2f         %20.10f\n", startbin, binVal);
   }

   endbin = (BINSTART) +
         (BINNUM) * (BINWIDTH);

   cout << setw(7) << setprecision(2) << endbin << endl << endl << endl;
   //printf ("%7.2f\n\n\n", endbin);

}


//print the header of the histogram
void Histogram::PrintBinHeader (void)
{
   using namespace std;

   int i;
   float binboundary;

   cout.setf(ios::fixed,ios::floatfield);

   binboundary = (BINSTART);
   for (i = 0; i < (BINNUM); i++)
   {
      cout << setw(6) << setprecision(1) << binboundary << " ";
      //printf ("%6.1f ", binboundary);
      binboundary += (BINWIDTH);
   }
   cout << endl;
}


// Print the bincounts of the bins
void Histogram::PrintBins (void)
{
   using namespace std;

   int i;
   double binVal;

   cout.setf(ios::fixed,ios::floatfield);

   for (i = 0; i < (BINNUM); i++)
   {
      binVal = BinCount (i);

      if ( (0.0 == binVal) )
         cout << "       ";         //printf ("       ");
      else
         cout << setw(6) << setprecision(0) << binVal << " ";   //printf ("%6.0f ", binVal);
   }
}


// Resize the bins to a total of totBinCnt 
void Histogram::Scale (int totBinCnt)
{
   double sum;
   float factor;

   sum = SumBins (0, (BINNUM) - 1);

   if (! (sum == .0) )
   {
      factor = (float) totBinCnt / sum;
      MulBins (factor);
   }
}


// assign binVal to bin[BinIx] 
void Histogram::SetBinCount (int binIx, double binVal)
{
   using namespace std;
   
   assert (binIx >= 0 && binIx < BINNUM);

   BIN[binIx] = binVal;
}


// Add sumValue to the bincount with index binIx 
void Histogram::SumBinCount (int binIx, double sumValue)
{
   double binVal = BinCount (binIx);

   SetBinCount (binIx, binVal + sumValue);
}


//  Sums the bins of  the histogram between startIx and endIx
double Histogram::SumBins (int startIx, int endIx)
{
   using namespace std;
  
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


// destructor
Histogram::~Histogram (void)
{

}

}

}//namespace Calibration
