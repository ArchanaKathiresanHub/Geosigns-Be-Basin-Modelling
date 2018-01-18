#include "ftanalysis.h"
#include "ftparameters.h"
#include "ftgrain.h"
#include "ftsampleprediction.h"
#include "ftsampleobservation.h"
#include "histogram.h"

#include <iostream>
//Yiannis
using std::cout; 
using std::endl;

namespace Calibration
{
namespace FissionTracks
{

// constructor
FtAnalysis::FtAnalysis ()
{

}

FtAnalysis::~FtAnalysis ()
{
   map<const std::string, FtSamplePrediction*>::const_iterator predSampIt;
   
   for ( predSampIt = m_predSamples.begin(); predSampIt != m_predSamples.end(); ++predSampIt )
      delete (*predSampIt).second;

   map<const std::string, FtSampleObservation*>::const_iterator obsSampIt;
   
   for ( obsSampIt = m_obsSamples.begin(); obsSampIt != m_obsSamples.end(); ++obsSampIt )
      delete (*obsSampIt).second;

   list<FtGrain*>::const_iterator grainIt;
   
   for ( grainIt = m_grains.begin(); grainIt != m_grains.end(); ++grainIt )
      delete (*grainIt);

}


FtSamplePrediction* FtAnalysis::addSample( const std::string& sampleCode, 
					   double zeta, 
					   double UStdGlassTrackDensity )
{
   FtSamplePrediction* predSample = new FtSamplePrediction(sampleCode); 
   m_predSamples[ sampleCode ] = predSample;

   m_obsSamples[ sampleCode ] = new FtSampleObservation(sampleCode, zeta, UStdGlassTrackDensity); 

   return predSample;
}

void FtAnalysis::addGrain( const std::string& sampleId, int grainId, int spontTrackNo, int inducedTrackNo, double clWeightPercent )
{
   //add grain
   FtGrain* grain = new FtGrain ( sampleId, grainId, spontTrackNo, inducedTrackNo, clWeightPercent );
   
   //add Cl-index of grain to prediction-samples
   FtParameters& params = FtParameters::getInstance();
   int indexCl = params.indexFromClWeightPercent( grain->getClWeightPercent() );

   if( !m_predSamples[ grain -> getSampleId() ] -> addClIndexIfNotYetExists( indexCl )) {
      std::cout << "Basin_Error  Invalid Fission track chlorine weight percentage: " << clWeightPercent << std::endl;
      return;
   };

   //add grain to total grain list and to corresponding observed sample, and add age histogram if 
   //numbers of spontaneous/induced lenghts are available
   
   m_grains.push_back(grain);

   FtSampleObservation* obsSample = m_obsSamples[ grain -> getSampleId() ];
   obsSample -> addGrain(grain);

   //always add length index, even if there is no length recorded(might be total annealing)
   obsSample -> addLengthsClIndexIfNotYetExists( indexCl );

   if ( grain -> getSpontTrackNo() || grain -> getSpontTrackNo() )
      obsSample -> addAgesClIndexIfNotYetExists( indexCl );

  
   
}

void FtAnalysis::addTrackLength(const std::string& sampleId, const int grainId, const double length)
{
   FtGrain* grain = m_obsSamples[sampleId ] -> getGrain(grainId);
   if( grain != 0 ) {
      grain->addTrackLength(length);
   }
}

FtSamplePrediction* FtAnalysis::getPredSample(const std::string& sampleId)
{
   return m_predSamples[ sampleId ];
}

FtSampleObservation* FtAnalysis::getObsSample(const std::string& sampleId)
{
   return m_obsSamples[ sampleId ];
}

void FtAnalysis::initialize()
{
   std::map<const std::string, FtSampleObservation*>::const_iterator iObsSample;
   
   for ( iObsSample = m_obsSamples.begin(); iObsSample != m_obsSamples.end(); ++iObsSample )
   {
      iObsSample->second -> processMeasuredData();
   }
 
}

void FtAnalysis::resetPredictedFissionTrackLenghts()
{
   // set all predicted tracklength distributions to zero

   FtParameters& params = FtParameters::getInstance();

   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;

   for ( iPredSample = m_predSamples.begin(); iPredSample != m_predSamples.end(); ++iPredSample )
   {
      Histogram *predhist;

      for (int i = 0; i < params.getNumBinClWeightPercent(); i++)
      {
         predhist = iPredSample -> second -> getTrackLengths(i);
         if ( predhist )
         {
            predhist->Initialize (  ((predhist)->getBinStart()),
                                    ((predhist)->getBinWidth()),
                                    ((predhist)->getBinNum()));
         }
      }
   }
}


void FtAnalysis::finalize()
{
   //calculatation of predicted ages and comparison/matching of measured and predicted data


   // calculatation of predicted ages from predicted tracklength distributions
   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;
   
   for ( iPredSample = m_predSamples.begin(); iPredSample != m_predSamples.end(); ++iPredSample )
   {
      iPredSample->second -> calcPredAge();
   }

   //correction of predicted lengths
   for ( iPredSample = m_predSamples.begin(); iPredSample != m_predSamples.end(); ++iPredSample )
   {
      iPredSample->second -> geotrackCorrPredTracklengthDists();
   }

   //scale predicted to observed lengths
   scalePredTracklengthsToObserved();

   //cout << "StartOutputResults Histogram 2" << endl;
   //printTrackLengths();
   //cout << "EndOutputResults Histogram 2" << endl;

   //calculate measure for accordance of predicted and measured track lengths
   calcLengthChi2();

   //merge bins for output
   mergeBinsForOutput();
   
}

list<FtGrain*>& FtAnalysis::getGrains()
{
   return m_grains;
}

double FtAnalysis::getLengthChi2(const std::string & sampleId)
{
   return m_lengthChi2s[sampleId];
}


void FtAnalysis::scalePredTracklengthsToObserved()
{

   FtParameters& params = FtParameters::getInstance();

   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;
   std::map<const std::string, FtSampleObservation*>::const_iterator iObsSample;

   for ( iPredSample = m_predSamples.begin(), iObsSample = m_obsSamples.begin(); 
         ( (iPredSample != m_predSamples.end() ) && ( iObsSample != m_obsSamples.end() ) ); 
         ++iPredSample, ++iObsSample )
   {

      Histogram *predhist, *obshist;
      double tracknum;
      int i;

      for (i = 0; i < params.getNumBinClWeightPercent(); i++)
      {
         obshist  = iObsSample  -> second -> getTrackLengths(i);
         predhist = iPredSample -> second -> getTrackLengths(i);
         if (predhist && obshist)
         {
            tracknum = obshist->SumBins (0, obshist->getBinNum() - 1);
            if (tracknum > 0)
            {
               predhist->Scale ( (int) tracknum);
            }
            else
            {
               predhist->Initialize (((predhist)->getBinStart()),
                                     ((predhist)->getBinWidth()),
                                     ((predhist)->getBinNum()));
            }
         }
      }
   }
}

void FtAnalysis::calcLengthChi2 ()
{
   FtParameters& params = FtParameters::getInstance();

   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;
   std::map<const std::string, FtSampleObservation*>::const_iterator iObsSample;

   for ( iPredSample = m_predSamples.begin(), iObsSample = m_obsSamples.begin(); 
         ( (iPredSample != m_predSamples.end() ) && ( iObsSample != m_obsSamples.end() ) ); 
         ++iPredSample, ++iObsSample )
   {

      Histogram *predhist, *obshist;
      int i;

      double chi2 = 0.0;

      for (i = 0; i < params.getNumBinClWeightPercent(); i++)
      {
         obshist  = iObsSample  -> second -> getTrackLengths(i);
         predhist = iPredSample -> second -> getTrackLengths(i);
         if (predhist && obshist)
         {
            chi2 += CalcChi2 (predhist, obshist);
         }
      }
      const string& sampleId = iObsSample -> first;
      m_lengthChi2s[ sampleId ] = chi2;
   }      
}

void FtAnalysis::mergeBinsForOutput()
{
   FtParameters& params = FtParameters::getInstance();

   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;
   std::map<const std::string, FtSampleObservation*>::const_iterator iObsSample;

   for ( iPredSample = m_predSamples.begin(), iObsSample = m_obsSamples.begin(); 
         ( (iPredSample != m_predSamples.end() ) && ( iObsSample != m_obsSamples.end() ) ); 
         ++iPredSample, ++iObsSample )
   {

      Histogram *predhist, *obshist;

      for (int i = 0; i < params.getNumBinClWeightPercent(); i++)
      {
         obshist  = iObsSample  -> second -> getTrackLengths(i);
         predhist = iPredSample -> second -> getTrackLengths(i);
         if (predhist)
         {
            predhist -> MergeBins(params.getBinMergingFactor());
         }
         if (obshist)
         {
            obshist  -> MergeBins(params.getBinMergingFactor());
         }
      }
   }      
}


void FtAnalysis::printTrackLengths()
{

   FtParameters& params = FtParameters::getInstance();

   std::map<const std::string, FtSamplePrediction*>::const_iterator iPredSample;
   std::map<const std::string, FtSampleObservation*>::const_iterator iObsSample;

   for ( iPredSample = m_predSamples.begin(), iObsSample = m_obsSamples.begin(); 
         ( (iPredSample != m_predSamples.end() ) && ( iObsSample != m_obsSamples.end() ) ); 
         ++iPredSample, ++iObsSample )
   {

      Histogram *predhist, *obshist;
      int i;

      for (i = 0; i < params.getNumBinClWeightPercent(); i++)
      {
         obshist  = iObsSample  -> second -> getTrackLengths(i);
         predhist = iPredSample -> second -> getTrackLengths(i);
         if (predhist && obshist)
         {
            cout << "predicted track lengths of sample " << iPredSample  -> second -> getSampleCode() << ", indexCl " << i << endl;
            predhist->Print();
            cout << endl;
            cout << "observed track lengths of sample " << iObsSample  -> second -> getSampleCode() << ", indexCl " << i << endl;
            obshist->Print();
         }
      }
   }
}



}//FissionTracks

}//Calibration 
