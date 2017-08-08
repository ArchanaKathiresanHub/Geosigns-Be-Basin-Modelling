#ifndef CALIBRATION_FTGRAIN_H
#define CALIBRATION_FTGRAIN_H

#include <string>
#include <list>

namespace Calibration
{

namespace FissionTracks
{

class FtGrain
{

 public:
  
  FtGrain(const std::string& sampleId, int grainId, int spontTrackNo, int inducedTrackNo, double clWeightPercent);

  std::string   getSampleId() const;
  int           getGrainId() const;
  int           getSpontTrackNo() const;
  int           getInducedTrackNo() const;
  double        getClWeightPercent() const;
  double        getGrainAge() const;
  void          setGrainAge(double grainAge);
  void          addTrackLength(const double length);
  std::list<double>& getTrackLengths(); 

 private:

  std::string  m_sampleId;
  int          m_grainId;
  int          m_spontTrackNo;
  int          m_inducedTrackNo;
  double       m_clWeightPerc;
  double       m_grainAge;
  std::list<double> m_trackLengths;

};

//!!! use prefix for ALL member variables in Ft... classes!

}

}

#endif
