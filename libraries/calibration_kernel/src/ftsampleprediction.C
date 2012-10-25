#include "ftsampleprediction.h"

#include "ftparameters.h"

#include "histogram.h"

#include <iostream>
//Yiannis
#include <iomanip>

#include <math.h>

using std::cout; 
using std::setw;
using std::setprecision;

////////////////////////////////////////////////////////////////////////////////////
///Implementation of FtSamplePrediction
////////////////////////////////////////////////////////////////////////////////////

namespace Calibration
{

namespace FissionTracks
{

FtSamplePrediction::FtSamplePrediction(const std::string sampleCode)
{
   m_sampleCode = sampleCode;

   FtParameters& params = FtParameters::getInstance();

   m_trackLengths.clear();
   m_trackLengths.resize( params.getNumBinClWeightPercent(), 0 );
}

FtSamplePrediction::~FtSamplePrediction()
{
   //for (size_t i; i < m_trackLengths.size(); ++i)
   
   for (size_t i = 0; i < m_trackLengths.size(); ++i)//Yiannis
   {
      if ( m_trackLengths[i] )
      {
         delete m_trackLengths[i];
         m_trackLengths[i] = 0;
      }
   }
}


void FtSamplePrediction::addClIndexIfNotYetExists(size_t indexCl)
{
   if ( ( ! m_trackLengths[indexCl] ) && ( indexCl < m_trackLengths.size() ) )
   {
      m_trackLengths[indexCl] = new Histogram;

      FtParameters& params = FtParameters::getInstance();
      m_trackLengths[indexCl] -> Initialize( params.getStartBinTrackLengths(), 
                                           params.getStepBinTrackLengths(), 
                                           params.getNumBinTrackLengths() );
   }
}

void FtSamplePrediction::advanceState (double timestep, double temperature)
{
   FtParameters& params = FtParameters::getInstance();
   
   for ( int indexCl = 0 ; indexCl < params.getNumBinClWeightPercent(); ++indexCl )
   { 
      if ( getTrackLengths(indexCl) )
      {
         calcTrackLengths (timestep, temperature, indexCl);
      }         
   }
}


// Calculate the fission track length distribution of the sample 
// through time as a function of the temperature of a specific Cl
// weight percentage, defined by the index in the histogram array
void FtSamplePrediction::calcTrackLengths (double timestep, double temperature, int ClIndex)
{
   double hBeg, hEnd;           // The original h value of the start and the end of a bin
   int iBeg, iEnd;              // Index of the bin for hBeg and hEnd
   double tBeg, tEnd;           // The original equivalent time of the start and end of a bin
   double factor;
   double c0, c1, L0;
   int binnum, i;

   FtParameters& params = FtParameters::getInstance();

   Histogram* histogram = m_trackLengths[ClIndex];

   if (histogram)
   {
      // Cl weight percentage is present in the sample
      
      vector<double> newCount( params.getNumBinTrackLengths(), 0.0 ); //!!! is allocation a performance issue? Otherwise make it a static / member!

      // Get the Cl weight percentage dependent parameters
      c0 = params.getC0( ClIndex );
      c1 = params.getC1( ClIndex );
      L0 = params.getL0( ClIndex );
      //binnum = ( histogram ->BINNUM);
      histogram->FindBin (L0, &binnum);

      // Get the h, index and equivalent time of the start of the first bin at time t-timestep
      hBinPrevious (0, binnum, L0, c0, c1, temperature, timestep,
                              &hBeg, &iBeg, &tBeg);

      // loop over all bins in the histogram
      for (i = 0; i < binnum; i++)
      {

         // Get the h, index and equivalent time of the end  of the current bin at 
         // time t-timestep
         hBinPrevious (i + 1, binnum, L0, c0, c1, temperature, timestep,
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
                     factor = ( hEnd - hBeg ) /
                              ( this->hValue (iBeg + 1, L0, c0, c1) -
                                this->hValue (iBeg, L0, c0, c1));
                  }
                  newCount[i] += factor * histogram->BinCount (iBeg);
               }
               else             // hBeg and hEnd fall in different bins 
               {
                  // take fractional part of first bin 
                  factor = ( this->hValue (iBeg + 1, L0, c0, c1) - hBeg ) /
                           ( this->hValue (iBeg + 1, L0, c0, c1) -
                             this->hValue (iBeg, L0, c0, c1));
                  newCount[i] += factor * histogram->BinCount (iBeg);

                  // last bin 
                  if (iEnd == binnum - 1)  // take the whole bin
                  {
                     factor = 1.;
                  }
                  else          // take fractional part of the bin
                  {
                     factor = ( hEnd - this->hValue (iEnd, L0, c0, c1)) /
                              ( this->hValue (iEnd + 1, L0, c0, c1) -
                                this->hValue (iEnd, L0, c0, c1));
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
                  factor = ( this->hValue (iBeg + 1, L0, c0, c1) - hBeg) /
                           ( this->hValue (iBeg + 1, L0, c0, c1) -
                             this->hValue (iBeg, L0, c0, c1));
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
               newCount[i] += tEnd / params.getSecsInMa();
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
            newCount[i] += (tEnd - tBeg) / params.getSecsInMa();
         }

         // set start of bin to start of the next bin
         hBeg = hEnd;
         tBeg = tEnd;
         iBeg = iEnd;

         cout.setf(ios::scientific,ios::floatfield);
         cout << setw(25) << setprecision(18);

         //cout << "end of predLength-loop " << i << ": hEnd " << hEnd << " tEnd " << tEnd << " iEnd " << iEnd <<endl;

      }
      // Copy the new counts into the track length distribution
      histogram->CopyBin (newCount);
   }

}


// Calculate the equivalent time as function of temperatur and h
double FtSamplePrediction::equivalentTime (double temperature, double h)
{
   FtParameters& params =FtParameters::getInstance();

   double a = (double) 1. / temperature - params.getT0Inv();
   double teq = params.getT0() * exp (a * h);

   return (teq);

}


// Find the index of the bin in which h falls
int FtSamplePrediction::hBinIndex (double h, int binnum, double L0, double c0, double c1)
{
   int i;

   for (i = 1; i < binnum; i++)
   {
      if (h > this->hValue (i, L0, c0, c1))
      {
         return (i - 1);
      }
   }
   return (binnum - 1);

}

// Calculate h from r (where is calculated from i) of a specific
//                  composition (defined by c0 and c1)
double FtSamplePrediction::hValue (int ix, double L0, double c0, double c1)
{
   FtParameters& params = FtParameters::getInstance();

   double r = ( params.getStartBinTrackLengths() + (float) ix * params.getStepBinTrackLengths() ) / L0;
   double h = (double) ( ( log ( (double) 1. - r * r * r ) - c0 ) / c1 );

   return h;
}


//  This routine calculates the h value, the equivalent time of this h
//  and the index of the bin in which this h falls at time t-dt. 
//  From the binindex ix the h value at time t is calculated. From this 
//  h and the current temperature the equivalent time is calculated. 
//  From this the hPrev, which is the h value at time t-dt 
//  and the equivalent time of hPrev and current temperature is calculated
//  Finally the bin index of hPrev is determined.

//  Name                  Type        I/O   Description
//  ----------------------------------------------------------------------------
//  ix                    int          I    index of bin
//  binnum                int          I    number of bins in histogram
//  c0                    double       I    Cl weight perc. dependent parameter
//  c1                    double       I    Cl weight perc. dependent parameter
//  temperature           double       I    temperature
//  hPrev                 double*      O    h value at time t-dt with temperture 
//  ixPrev                int*         O    index of bin in which hPrev lies 
//  tPrev                 double*      O    equivalent time of hPrev at temperature 

void FtSamplePrediction::hBinPrevious (int ix, int binnum, double L0, double c0, 
                             double c1, double temperature, double timestep,
                             double* hPrev, int* ixPrev, double* tPrev)
{
   FtParameters& params = FtParameters::getInstance();

   double teq;

   *ixPrev = -1;
   if ( ix == binnum )
   {
      // equivalent time is zero ( l==L0 => r==1 => h== -inf)
      *tPrev = timestep;
   }
   else
   {
      teq = this->equivalentTime (temperature, this->hValue (ix, L0, c0, c1) );
      *tPrev = timestep - teq;

      if ( *tPrev < .0 )
      {
         // previous equivalent time is in time range
         *hPrev = log ( ( teq - timestep ) / (double) params.getT0() ) / ((double) 1. / temperature - params.getT0Inv() );
         *ixPrev = (int) this->hBinIndex ( *hPrev, binnum, L0, c0, c1 );
      }
   }
}

void FtSamplePrediction::calcPredAge()
{
   Histogram* histogram;
   double L, L0, binCount;
   double cumDensity = 0.0;
   int histnum = 0;
   int i, j;

   FtParameters& params = FtParameters::getInstance();

   for (i = 0; i < params.getNumBinClWeightPercent(); i++)
   {
      histogram = m_trackLengths[i];
      if (histogram)
      {
         L0 = 16.0;             // this is what geotrack does
         L = params.getStartBinTrackLengths() + 0.5 * params.getStepBinTrackLengths();

         for (j = 0; j < histogram->getBinNum(); j++)
         {
            binCount = histogram->BinCount (j);
            cumDensity += densityFromR (L / L0) * binCount;

            L += params.getStepBinTrackLengths();
         }
         histnum++;
      }
   }
   if (histnum > 0)
   {
      // calculate the predicted age

      // The total number of generated tracks == histnum*Depositional Age of the sample
      // Depositional age is therefore divided out of the equation.

      m_predAge = params.getRenormalizationFactor() * cumDensity / (double) histnum;
   }
   else
   {
      m_predAge = params.getUndefinedDouble();
   }
}


double FtSamplePrediction::densityFromR (double r)
{
   double density;

   if (r > .65)
      density = r;
   else if (r > 0.45 && r <= 0.65)
      density = 3.25 * (r - 0.45);
   else                         /* r <= .45 */
      density = .0;

   return (density);
}


void FtSamplePrediction::geotrackCorrPredTracklengthDists()
{
   int i, j, ClIndex, binnum;
   Histogram *histogram, *partdist, *sum;
   double L0, tracklength, tl, sigma;
   double p, nextp, a, b, epx, bjval;
   vector<double> bj;

   FtParameters& params = FtParameters::getInstance();

   partdist = new Histogram;
   sum = new Histogram;

   for (ClIndex = 0; ClIndex < params.getNumBinClWeightPercent(); ClIndex++)
   {
      histogram = m_trackLengths[ClIndex];
      if (histogram)
      {
         binnum = histogram->getBinNum();
         partdist->Initialize (params.getStartBinTrackLengths(), params.getStepBinTrackLengths(), binnum);
         sum->Initialize (params.getStartBinTrackLengths(), params.getStepBinTrackLengths(), binnum);
         
         L0 = params.getL0 (ClIndex);
         
         bj.resize(binnum);
         
         for (j = 0; j < binnum; j++)
         {
            a = -4.5;
            b = 0.65;
            if (j == binnum - 1)
               tracklength = (params.getStartBinTrackLengths() + (double) j *
                              params.getStepBinTrackLengths() + L0) / 2.0;
            else
               tracklength = params.getStartBinTrackLengths() + ((double) j + .5) * params.getStepBinTrackLengths();

            epx = exp (a + (b * tracklength));
            bj[j] = tracklength * epx / (1.0 + epx);
         }

         for (i = 0; i < binnum; i++)
         {
            if (i == binnum - 1)
               tracklength = (params.getStartBinTrackLengths() + (double) i *
                              params.getStepBinTrackLengths() + L0) / 2.0;
            else
               tracklength = params.getStartBinTrackLengths() + ((double) i + 0.5) * params.getStepBinTrackLengths();

            if (tracklength > 7.2)
            {
               if (tracklength > 11.)
                  sigma = tracklength * (-0.0325) + 1.34;
               else
                  sigma = tracklength * (-0.54) + 6.9;

               p = prob (-tracklength / sigma);
               tl = params.getStartBinTrackLengths();
               for (j = 0; j < binnum; j++)
               {
                  tl += params.getStepBinTrackLengths();
                  nextp = prob ((tl - tracklength) / sigma);
                  partdist->SetBinCount (j, p - nextp);
                  p = nextp;
               }

               // normalise partial distributions
               partdist->Scale (1);

               for (j = 0; j < binnum; j++)
               {
                  bjval = bj[j];
                  partdist->MulBinCount (j, 1.0 / bjval);
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
            bjval = bj[j];

            sum->MulBinCount (j, bjval);
         }
         
         (*histogram) = (*sum);
      }
   
   }
   delete partdist;
   delete  sum;

}


double FtSamplePrediction::prob (double z)
{
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


   zabs = fabs (z);
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


Histogram* FtSamplePrediction::getTrackLengths(int indexCl)
{
   return m_trackLengths[indexCl];
}


const std::string& FtSamplePrediction::getSampleCode() const 
{
   return m_sampleCode;
}

double FtSamplePrediction::getPredAge()
{
   return m_predAge;
}


}

}//namespace Calibration
