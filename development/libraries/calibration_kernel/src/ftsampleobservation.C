#include "ftsampleobservation.h"

#include "ftgrain.h"

#include "ftparameters.h"

#include "histogram.h"

#include <iostream>

#include <math.h>



////////////////////////////////////////////////////////////////////////////////////
///Implementation of FtSampleObservation
////////////////////////////////////////////////////////////////////////////////////
namespace Calibration
{

namespace FissionTracks
{

FtSampleObservation::FtSampleObservation(const std::string sampleCode, double zeta, double UStdGlassTrackDensity)
{
   m_sampleCode = sampleCode;
   m_zeta = zeta;
   m_UStdGlassTrackDensity = UStdGlassTrackDensity;

   FtParameters& params = FtParameters::getInstance();

   m_trackLengths.clear();
   m_trackLengths.resize( params.getNumBinClWeightPercent(), 0 );

   m_grainAges.clear();
   m_grainAges.resize( params.getNumBinClWeightPercent(), 0 );

   m_sumSpontTrackNo   = 0;
   m_sumInducedTrackNo = 0;

   m_numGrainsWithCounts = 0;
}

FtSampleObservation::~FtSampleObservation()
{
   //for (size_t i; i < m_trackLengths.size(); ++i)
   for (size_t i = 0; i < m_trackLengths.size(); ++i)//Yiannis: the default ctor will set it to 0 but let's be sure:-)
   {
      if ( m_trackLengths[i] )
      {
         delete m_trackLengths[i];
         m_trackLengths[i] = 0;
      }
   }

   //for (size_t i; i < m_grainAges.size(); ++i)
   for (size_t i = 0; i < m_trackLengths.size(); ++i)
   {
      if ( m_grainAges[i] )
      {
         delete m_grainAges[i];
         m_grainAges[i] = 0;
      }
   }
}


void FtSampleObservation::addLengthsClIndexIfNotYetExists(size_t indexCl)
{
   if (( indexCl < m_trackLengths.size() ) && ( ! m_trackLengths[indexCl] ))
   {
      m_trackLengths[indexCl] = new Histogram;

      FtParameters& params = FtParameters::getInstance();
      m_trackLengths[indexCl] -> Initialize( params.getStartBinTrackLengths(), 
                                             params.getStepBinTrackLengths(), 
                                             params.getNumBinTrackLengths() );
   }
}

void FtSampleObservation::addAgesClIndexIfNotYetExists(size_t indexCl)
{
   if (( indexCl < m_grainAges.size() ) && ( ! m_grainAges[indexCl] ))
   {
      m_grainAges[indexCl] = new Histogram;

      FtParameters& params = FtParameters::getInstance();
      m_grainAges[indexCl] -> Initialize( params.getStartBinGrainAge(), 
                                             params.getStepBinGrainAge(), 
                                             params.getNumBinGrainAge() );
   }
}

void FtSampleObservation::addGrain(FtGrain* grain)
{
   FtParameters& params = FtParameters::getInstance();

   int Ns = grain->getSpontTrackNo();
   int Ni = grain->getInducedTrackNo();

   if ( ( Ns !=  params.getUndefinedInt() ) && 
        ( Ni !=  params.getUndefinedInt() ) &&
        ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
   {
      m_sumSpontTrackNo   += Ns;
      m_sumInducedTrackNo += Ni;
      ++m_numGrainsWithCounts;
   }
   m_grains.push_back(grain);
}

FtGrain* FtSampleObservation::getGrain(int grainId)
{
   FtGrain* grain = 0;
   list<FtGrain*>::const_iterator iter;
   for (iter = m_grains.begin() ; iter != m_grains.end(); ++iter )
   {
      if ( (*iter)->getGrainId() == grainId )
      {
         grain = *iter;
         break;
      }
   }
   
   return grain;
}

void FtSampleObservation::binTrackLengths()
{
   FtParameters& params = FtParameters::getInstance();
   
   list<FtGrain*>::const_iterator iGrain;

   for (iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain)
   {
      size_t indexCl =  params.indexFromClWeightPercent( (*iGrain)->getClWeightPercent() );
      list<double>& lengths = (*iGrain)->getTrackLengths();
      list<double>::const_iterator iLength;
      for (iLength = lengths.begin(); iLength != lengths.end(); ++iLength)
      {
         if (( indexCl < m_trackLengths.size() ) && m_trackLengths[ indexCl ] )
         {
            m_trackLengths[ indexCl ] -> FindAndIncrementBin (*iLength); 
            //return value could be checked: false->throw exception("invalid track length"); 
         }
         //else throw exception("invalid Cl-Index");
      }
   }
}

void FtSampleObservation::binGrainAges()
{
   list<FtGrain*>::const_iterator iGrain;
   
   FtParameters& params = FtParameters::getInstance();
   
   for (iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain)
   {
      size_t indexCl =  params.indexFromClWeightPercent( (*iGrain)->getClWeightPercent() );
      double age = (*iGrain)->getGrainAge();
      if (( indexCl < m_grainAges.size() ) && m_grainAges[ indexCl ] )
      {
         m_grainAges[ indexCl ] -> FindAndIncrementBin(age); 
         //return value could be checked: false->throw exception("invalid track length"); 
      } 
      //else throw exception("invalid Cl-Index");
   }
}

void FtSampleObservation::processMeasuredData()
{
   //more re-engineering necessary: all calc functions should not be member functions, 
   //but get all necessary arguments and return all results 
   //(e.g. ages of all grains, which have to be set in a 
   //subsequent step in order to see all calculation dependencies)

   //bin measured track lengths into histograms
   binTrackLengths();

   //compute grain ages
   calcGrainAgesAndErrors();

   binGrainAges();

   if ( (m_sumSpontTrackNo != 0) && (m_sumInducedTrackNo != 0) )
   {
      calcPooledAgeAndError();

      calcDegreesOfFreedom();
 
      calcAgeChi2();

      calcPAgeChi2();

      calcCorrCoeff();

      calcVarSqrtNiNs();
      
      calcRatioNsNiAndErr();

      calcMeanRatioNsNiAndError();

      calcMeanAgeAndError();

      calcCentralAgeAndError();
      
      determineApatiteYield();
   }
   else
   {
      FtParameters& params = FtParameters::getInstance();

      m_pooledAge        = params.getUndefinedDouble();
      m_pooledAgeErr     = params.getUndefinedDouble();
      m_ageChi2          = params.getUndefinedDouble();
      m_pAgeChi2         = params.getUndefinedDouble();
      m_degreesOfFreedom = params.getUndefinedInt()   ;
      m_corrCoeff        = params.getUndefinedDouble();
      m_varSqrtNs        = params.getUndefinedDouble();
      m_varSqrtNi        = params.getUndefinedDouble();
      m_NsDivNi          = params.getUndefinedDouble();
      m_NsDivNiErr       = params.getUndefinedDouble();
      m_meanRatioNsNi    = params.getUndefinedDouble();
      m_meanRatioNsNiErr = params.getUndefinedDouble();
      m_meanAge          = params.getUndefinedDouble();
      m_meanAgeErr       = params.getUndefinedDouble();
      m_centralAge       = params.getUndefinedDouble();
      m_centralAgeErr    = params.getUndefinedDouble();
      m_apatiteYield     = "";
   }

}


void FtSampleObservation::calcPooledAgeAndError()
{


   m_pooledAge    = calcAge ( (double) m_sumSpontTrackNo / (double) m_sumInducedTrackNo, m_zeta, m_UStdGlassTrackDensity);
   m_pooledAgeErr = m_pooledAge * sqrt (1.0 / (double) m_sumSpontTrackNo + 1.0 / (double) m_sumInducedTrackNo 
                                           //+ (zetaErr * zetaErr) / (zeta * zeta)  //zetaErr set to 0.0 in old cauldron
                                           );

}


void  FtSampleObservation::calcDegreesOfFreedom()
{
   m_degreesOfFreedom = m_numGrainsWithCounts - 1;
}

void FtSampleObservation::calcAgeChi2()
{
   int Ns, Ni;
   double N_sj, N_ij;

   m_ageChi2 = 0.0;

   FtParameters& params = FtParameters::getInstance();

   list<FtGrain*>::const_iterator iGrain;
   
   for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
   {
      Ns = (*iGrain)->getSpontTrackNo();
      Ni = (*iGrain)->getInducedTrackNo();

      if ( ( Ns !=  params.getUndefinedInt() ) && 
           ( Ni !=  params.getUndefinedInt() ) &&
           ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
      {
         N_sj = (double) (m_sumSpontTrackNo   * (Ns + Ni)) / (double) (m_sumSpontTrackNo + m_sumInducedTrackNo);
         N_ij = (double) (m_sumInducedTrackNo * (Ns + Ni)) / (double) (m_sumSpontTrackNo + m_sumInducedTrackNo);
         
         m_ageChi2 += ((double) Ns - N_sj) * ((double) Ns - N_sj) / N_sj +
                      ((double) Ni - N_ij) * ((double) Ni - N_ij) / N_ij;
      }
   }
}

void FtSampleObservation::calcPAgeChi2()
{
   m_pAgeChi2 = calcPChi2(m_degreesOfFreedom, m_ageChi2);
}

void FtSampleObservation::calcCorrCoeff()
{
   float aveNs, aveNi;
   int Ns, Ni;
   float sigmaNsNi, sigmaNs, sigmaNi;

   int numGrains = (int)m_numGrainsWithCounts;

   aveNs = (double) m_sumSpontTrackNo / (double) numGrains;
   aveNi = (double) m_sumInducedTrackNo / (double) numGrains;

   sigmaNsNi = 0.0;
   sigmaNs   = 0.0;
   sigmaNi   = 0.0;

   FtParameters& params = FtParameters::getInstance();

   list<FtGrain*>::const_iterator iGrain;
   
   for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
   {
      Ns = (*iGrain)->getSpontTrackNo();
      Ni = (*iGrain)->getInducedTrackNo();

      if ( ( Ns !=  params.getUndefinedInt() ) && 
           ( Ni !=  params.getUndefinedInt() ) &&
           ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
      {
         sigmaNsNi += ((float) Ns - aveNs) * ((float) Ni - aveNi);
         sigmaNs += ((float) Ns - aveNs) * ((float) Ns - aveNs);
         sigmaNi += ((float) Ni - aveNi) * ((float) Ni - aveNi);
      }
      
   }

   sigmaNs = sqrt (sigmaNs);
   sigmaNi = sqrt (sigmaNi);

   if ( sigmaNsNi == 0.0 )
   {
      if ( ( sigmaNs == 0.0 ) && ( sigmaNi == 0.0 ) )
      {
         m_corrCoeff = 1.0;
      }
      else
      {
         m_corrCoeff = 0.0;
      }
   }
   else
   {
      m_corrCoeff = sigmaNsNi / (sigmaNs * sigmaNi);
   }
}

void FtSampleObservation::calcVarSqrtNiNs ()
{
   int Ni, Ns;
   double aveSqrtNi, sumNi;
   double aveSqrtNs, sumNs;

   FtParameters& params = FtParameters::getInstance();

   aveSqrtNi = 0.0;
   aveSqrtNs = 0.0;

   // Calculate the average of the square root of Ni and Ns
   list<FtGrain*>::const_iterator iGrain;
   
   for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
   {
      Ns = (*iGrain)->getSpontTrackNo();
      Ni = (*iGrain)->getInducedTrackNo();

      if ( ( Ns !=  params.getUndefinedInt() ) && 
           ( Ni !=  params.getUndefinedInt() ) &&
           ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
      {
         aveSqrtNi += sqrt ((double) Ni);
         aveSqrtNs += sqrt ((double) Ns);
      }
   }

   int numGrains = m_numGrainsWithCounts;

   aveSqrtNi = aveSqrtNi / numGrains;
   aveSqrtNs = aveSqrtNs / numGrains;

   sumNi = 0.0;
   sumNs = 0.0;

   if (numGrains > 1)
   {
      for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
      {

         Ns = (*iGrain)->getSpontTrackNo();
         Ni = (*iGrain)->getInducedTrackNo();

         if ( ( Ns !=  params.getUndefinedInt() ) && 
              ( Ni !=  params.getUndefinedInt() ) &&
              ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
         {
            sumNi += (sqrt  ((float) Ni) - aveSqrtNi) * (sqrt ((float) Ni) - aveSqrtNi);
            sumNs += (sqrt  ((float) Ns) - aveSqrtNs) * (sqrt ((float) Ns) - aveSqrtNs);
         }
      }
      m_varSqrtNi = sumNi / (numGrains - 1);
      m_varSqrtNs = sumNs / (numGrains - 1);
   }
   else
   {
      m_varSqrtNi = 0.;
      m_varSqrtNs = 0.;
   }
}

void FtSampleObservation::calcRatioNsNiAndErr()
{
   m_NsDivNi    = (double) m_sumSpontTrackNo / (double) m_sumInducedTrackNo;
   m_NsDivNiErr = sqrt (m_NsDivNi * (m_NsDivNi + 1) / (double) m_sumInducedTrackNo);
}

void FtSampleObservation::calcMeanAgeAndError ()
{
   m_meanAge    = calcAge (m_meanRatioNsNi, m_zeta, m_UStdGlassTrackDensity);

   m_meanAgeErr = m_meanAge * sqrt ( (m_meanRatioNsNiErr * m_meanRatioNsNiErr) / (m_meanRatioNsNi * m_meanRatioNsNi ) 
                                     //+ (zetaErr * zetaErr) / (zeta * zeta) // we don't have zetaErr, the old cauldron zetaErr=0.0 is assumed
                                     );

}

void  FtSampleObservation::calcMeanRatioNsNiAndError()
{
   int Ns, Ni;
   double sum, sum2;
   double ratio; 

   ratio = .0;
   sum2 = .0;
   sum = .0;



   list<FtGrain*>::const_iterator iGrain;
   
   for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
   {
      FtParameters& params = FtParameters::getInstance();

      Ns = (*iGrain)->getSpontTrackNo();
      Ni = (*iGrain)->getInducedTrackNo();

      if ( ( Ns !=  params.getUndefinedInt() ) && 
           ( Ni !=  params.getUndefinedInt() ) &&
           ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
      {
         ratio = (double) Ns / (double) Ni;
         sum2 += ratio * ratio;
         sum += ratio;
      }
   }
  
   int numGrains = m_numGrainsWithCounts;

   m_meanRatioNsNi = sum / numGrains;

   if (numGrains == 1)
   {
      m_meanRatioNsNiErr = 0.;
   }
   else
   {
      double n = (double) numGrains;
      m_meanRatioNsNiErr = sqrt ( (sum2 - sum * sum / n) / ( n * (n - 1) ) );
   }

}

void FtSampleObservation::calcCentralAgeAndError()
{
   int Ns, Ni;
   double sigma, theta;
   double x, t, z;
   double xSum, x2Sum, ySum, pSum, qSum, z2Sum;
   double fds, ratio;
   int nIter;

   int numGrains = m_numGrainsWithCounts;

   if (numGrains > 1)
   {
      xSum  = 0.0;
      x2Sum = 0.0;
      ySum  = 0.0;

      FtParameters& params = FtParameters::getInstance();

      list<FtGrain*>::const_iterator iGrain;

      for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
      {
         Ns = (*iGrain)->getSpontTrackNo();
         Ni = (*iGrain)->getInducedTrackNo();

         if ( ( Ns !=  params.getUndefinedInt() ) && 
              ( Ni !=  params.getUndefinedInt() ) &&
              ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
         {
            x = log (((double) Ns + .5) / ((double) Ni + .5));
            ySum += (double) Ns / (double) (Ns + Ni);
            xSum += x;
            x2Sum += x * x;
         }
      }
      theta = ySum / (double) (m_sumSpontTrackNo + m_sumInducedTrackNo);
      sigma = .6 * sqrt ((x2Sum - xSum * xSum / (double) numGrains) / (double)
                         (numGrains - 1));

      for (nIter = 0, fds = 1.; nIter <= 20 && fds >= .001; nIter++)
      {
         pSum = .0;
         qSum = .0;

         t = theta * (1. - theta);

         for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
         {
            Ns = (*iGrain)->getSpontTrackNo();
            Ni = (*iGrain)->getInducedTrackNo();

            if ( ( Ns !=  params.getUndefinedInt() ) && 
                 ( Ni !=  params.getUndefinedInt() ) &&
                 ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
            {
               x = (double) (Ns + Ni - 1) * sigma * t * sigma * t + t;
               pSum += (double) (Ns + Ni) / x;
               qSum += (double) Ns / x;
            }
         }


         theta = qSum / pSum;
         t = theta * (1. - theta);
         z2Sum = .0;

         for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
         {

            Ns = (*iGrain)->getSpontTrackNo();
            Ni = (*iGrain)->getInducedTrackNo();

            if ( ( Ns !=  params.getUndefinedInt() ) && 
                 ( Ni !=  params.getUndefinedInt() ) &&
                 ( Ni != 0 ) )        // if ( Ni != 0 && Ns != 0 )
            {
               z = ((double) Ns - (double) (Ns + Ni) * theta) /
                  ((double) (Ns + Ni - 1) * sigma * t * sigma * t + t);
               z2Sum += z * z;
            }
         }

         sigma *= sqrt (z2Sum / pSum);
         fds = fabs (z2Sum / pSum - 1.);
      }
      ratio = theta / (1. - theta);

      m_centralAge = calcAge (ratio, m_zeta, m_UStdGlassTrackDensity);
      m_centralAgeErr = m_centralAge / sqrt (pSum * t * t);
   }
   else
   {
      m_centralAge    = calcAge ((double) m_sumSpontTrackNo / (double) m_sumInducedTrackNo, m_zeta, m_UStdGlassTrackDensity);
      m_centralAgeErr = m_centralAge * sqrt (1. / (double) m_sumSpontTrackNo + 1. / (double) m_sumInducedTrackNo);
   }
}


void FtSampleObservation::determineApatiteYield()
{
   int numGrains = m_numGrainsWithCounts;

   if (numGrains > 20)
   {
      m_apatiteYield = "Excellent";
   }
   else if (numGrains == 20)
   {
      m_apatiteYield = "Very Good";
   }
   else if (numGrains >= 15)
   {
      m_apatiteYield = "Good";
   }
   else if (numGrains >= 10)
   {
      m_apatiteYield = "Fair";
   }
   else if (numGrains >= 5)
   {
      m_apatiteYield = "Poor";
   }
   else
   {
      m_apatiteYield = "Very Poor";
   }
}

void FtSampleObservation::calcGrainAgesAndErrors()
{
   std::list<FtGrain*>::const_iterator iGrain;
   
   for ( iGrain = m_grains.begin(); iGrain != m_grains.end(); ++iGrain )
   {

      FtParameters& params = FtParameters::getInstance();

      int Ns = (*iGrain)->getSpontTrackNo();
      int Ni = (*iGrain)->getInducedTrackNo();
      double ratioNsNi = (double) Ns / (double) Ni;

      if ( ( Ns != params.getUndefinedInt() ) &&
           ( Ni != params.getUndefinedInt() ) &&
           ( Ns != 0) &&
           ( Ni != 0) )
      {
         // calculate and set fission track age
         // calculate and set error in fission track age

         double grainAge    = FissionTracks::calcAge(ratioNsNi, m_zeta, m_UStdGlassTrackDensity);
         (*iGrain) -> setGrainAge( grainAge );
 
         double grainAgeErr =  grainAge * sqrt (1.0 / (double) Ns + 1.0 / (double) Ni); //originally cast to float
         (*iGrain) -> setGrainAgeErr( grainAgeErr );
      }
      else
      {
          (*iGrain) -> setGrainAge   ( params.getUndefinedDouble() );
          (*iGrain) -> setGrainAgeErr( params.getUndefinedDouble() ); 
      }
   }
}

const std::string& FtSampleObservation::getSampleCode() const 
{
   return m_sampleCode;
}

double FtSampleObservation::getZeta() const
{
   return m_zeta;
}

double FtSampleObservation::getUStdGlassTrackDensity() const
{
   return m_UStdGlassTrackDensity;
}

double FtSampleObservation::getPooledAge() const
{
   return m_pooledAge ;
}

double FtSampleObservation::getPooledAgeErr() const
{
   return m_pooledAgeErr;
}

double FtSampleObservation::getAgeChi2() const
{
   return m_ageChi2;
}

double FtSampleObservation::getPAgeChi2() const
{
   return m_pAgeChi2;
}

int FtSampleObservation::getDegreesOfFreedom() const
{
   return m_degreesOfFreedom;
}

double FtSampleObservation::getCorrCoeff() const
{
   return m_corrCoeff;
}

double FtSampleObservation::getVarSqrtNs() const
{
   return m_varSqrtNs;
}

double FtSampleObservation::getVarSqrtNi() const
{
   return m_varSqrtNi;
}

double FtSampleObservation::getNsDivNi() const
{
   return m_NsDivNi;
}

double FtSampleObservation::getNsDivNiErr() const
{
   return m_NsDivNiErr;
}

double FtSampleObservation::getMeanRatioNsNi() const
{
   return m_meanRatioNsNi;
}

double FtSampleObservation::getMeanRatioNsNiErr() const
{
   return m_meanRatioNsNiErr;
}

double FtSampleObservation::getCentralAge() const
{
   return m_centralAge;
}

double FtSampleObservation::getCentralAgeErr() const
{
   return m_centralAgeErr;
}

double FtSampleObservation::getMeanAge() const
{
   return m_meanAge;
}

double FtSampleObservation::getMeanAgeErr() const
{
   return m_meanAgeErr;
}

const std::string& FtSampleObservation::getApatiteYield() const
{
   return m_apatiteYield;
}

Histogram* FtSampleObservation::getTrackLengths(int indexCl)
{
   if ( indexCl < m_trackLengths.size() ) {
      return m_trackLengths[indexCl];
   } else {
      return 0;
   }
}

double calcAge(double ratioNsNi, double zeta, double UStdGlassTrackDensity )
{
   FtParameters& params = FtParameters::getInstance();

   double U238DecayConst = params.getU238DecayConstant();
   
   double ageInYears = log (1.0 + (zeta * U238DecayConst * ratioNsNi * 0.5 * UStdGlassTrackDensity ) ) / U238DecayConst;
   double ageInMa = ageInYears * 1.0e-6;
   return ageInMa;
}

double calcGauss (double x)
{
   double y, z, w;
   double gauss;

   if ( x == 0.0 )
   {
      z = 0.0;
   }
   else
   {
      y = fabs (x) / 2.;
      if (y >= 3.0)
      {
         z = 1.0;
      }
      else if (y < 1.)
      {
         w = y * y;
         z = ((((((((0.000124818987 * w
                     - 0.001075204047) * w + 0.005198775019) * w
                   - 0.019198292004) * w + 0.059054035642) * w
                 - 0.151968751364) * w + 0.319152932694) * w
               - 0.531923007300) * w + 0.797884560593) * y * 2.0;
      }
      else
      {
         y = y - 2.0;
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
   if (x > 0.0)
   {
      gauss = (z + 1.0) / 2.0;
   }
   else
   {
      gauss = (1.0 - z) / 2.0;
   }
   return (gauss);
}

double calcPChi2 (int dof, double Chi2)
{
   double b0 = 0.572364942925;
   double b1 = 0.564189583548;
   double xx = Chi2;
   double a = xx / 2.;
   double y = 0.;
   double t, z, e, c;
   int ilim, i;

   double pChi2;

   if (dof % 2 == 0 || dof > 2 && xx <= 40.)
   {
      y = exp (-a);
   }
   if (dof % 2 == 0)
   {
      pChi2 = y;
   }
   else
   {
      t = -sqrt (xx);
      pChi2 = 2. * calcGauss (t);
   }
   if (dof > 2)
   {
      xx = (double) (dof - 1) / 2.;
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
            pChi2 += exp (c * z - a - e);
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
         pChi2 += c * y;
      }
   }
   pChi2 *= 100.;

   return pChi2;
}

}

}
