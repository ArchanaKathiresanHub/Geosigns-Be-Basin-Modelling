#ifndef CALIBRATION_FTSAMPLEOBSERVATION_H
#define CALIBRATION_FTSAMPLEOBSERVATION_H

#include <vector>
#include <list>
#include <string>

namespace Calibration
{
   namespace FissionTracks
   {
      class Histogram;
   }
}

namespace Calibration
{

namespace FissionTracks
{

class FtGrain;

//class for observed samples. 


////////////////////////////////////////////////////////////////////////////////////
///Interface of FtSampleObservation
////////////////////////////////////////////////////////////////////////////////////
class FtSampleObservation
{
 public:
  
  //public functions
  FtSampleObservation(const std::string sampleCode, 
                      double zeta, 
                      double UStdGlassTrackDensity);

  ~FtSampleObservation();

  void addLengthsClIndexIfNotYetExists(size_t indexCl);

  void addAgesClIndexIfNotYetExists(size_t indexCl);

  void addGrain(FtGrain* grain);

  FtGrain* getGrain(int grainId);

  void processMeasuredData();


  //interface for sample data
  const std::string& getSampleCode() const;       

   double getZeta() const;
   double getUStdGlassTrackDensity() const;

   double getPooledAge() const;
   double getPooledAgeErr() const;

   double getAgeChi2() const;
   double getPAgeChi2() const;

   int    getDegreesOfFreedom() const;
   double getCorrCoeff() const;
   double getVarSqrtNs() const;
   double getVarSqrtNi() const;
   double getNsDivNi() const;
   double getNsDivNiErr() const;

   double getMeanRatioNsNi() const;
   double getMeanRatioNsNiErr() const;

   double getCentralAge() const;
   double getCentralAgeErr() const;

   double getMeanAge() const;
   double getMeanAgeErr() const;

  const std::string& getApatiteYield() const;

  Histogram* getTrackLengths(int indexCl);


 private:

  //private functions
  void binTrackLengths();

  void binGrainAges();

  void calcGrainAgesAndErrors();

  void calcPooledAgeAndError();

  void calcDegreesOfFreedom();

  void calcAgeChi2();

  void calcPAgeChi2();

  void calcCorrCoeff();

  void calcVarSqrtNiNs();

  void calcRatioNsNiAndErr();

  void calcMeanRatioNsNiAndError();

  void calcMeanAgeAndError ();

  void calcCentralAgeAndError();

  void determineApatiteYield();


  //private data
  std::string               m_sampleCode;

  double                    m_zeta;

  double                    m_UStdGlassTrackDensity;

  std::vector<Histogram*>   m_trackLengths;

  std::vector<Histogram*>   m_grainAges;

  std::list<FtGrain*>       m_grains;

  int m_sumSpontTrackNo;
  int m_sumInducedTrackNo;

  double m_pooledAge;
  double m_pooledAgeErr;


  double m_ageChi2;
  double m_pAgeChi2;

  int    m_degreesOfFreedom;
  double m_corrCoeff;
  double m_varSqrtNs;
  double m_varSqrtNi;
  double m_NsDivNi;
  double m_NsDivNiErr;

  double m_meanRatioNsNi;
  double m_meanRatioNsNiErr;

  double m_centralAge;
  double m_centralAgeErr;

  double m_meanAge;
  double m_meanAgeErr;

  std::string m_apatiteYield;

  int    m_numGrainsWithCounts;
};

 double calcAge(double ratioNsNi, double zeta, double UStdGlassTrackDensity );
 double calcGauss (double x);
 double calcPChi2 (int dof, double Chi2);

}

}//Namespace Calibration

#endif
