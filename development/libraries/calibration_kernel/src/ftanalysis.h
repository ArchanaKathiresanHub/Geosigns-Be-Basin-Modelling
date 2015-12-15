#ifndef CALIBRATION_FTANALYSIS_H
#define CALIBRATION_FTANALYSIS_H

#include <list>
#include <map>
#include <string>


namespace Calibration
{
namespace FissionTracks
{

class FtSamplePrediction;
class FtSampleObservation;
class FtGrain;


class FtAnalysis
{
 public:
  
  FtAnalysis();

  ~FtAnalysis();

  FtSamplePrediction* addSample(const std::string& sampleId, double zeta, double UStdGlassTrackDensity );

  void addGrain(const std::string& sampleId, int grainId, int spontTracko, int inducedTrackNo, double clWeightPercent);

  void addTrackLength(const std::string& sampleId, const int grainId, const double length);
  
  FtSamplePrediction*  getPredSample(const std::string& sampleId);
  FtSampleObservation* getObsSample(const std::string& sampleId);
  
  void initialize();

  void resetPredictedFissionTrackLenghts();

  void finalize();

  std::list<FtGrain*>& getGrains();

  double getLengthChi2(const std::string & sampleId);

  void printTrackLengths();

  //!!! add: void getPredSamples(map< const std::string, FtSamplePrediction* > & ftPredSamples);

 private:

  //private methods
  void scalePredTracklengthsToObserved();

  void calcLengthChi2();
  
  void mergeBinsForOutput();

  //private data
  std::map< const std::string, FtSamplePrediction* > m_predSamples; 
  std::map< const std::string, FtSampleObservation*> m_obsSamples;
  std::map< const std::string, double > m_lengthChi2s;

  std::list<FtGrain*> m_grains;
};

}//Fission tracks

}//Calibration

#endif
