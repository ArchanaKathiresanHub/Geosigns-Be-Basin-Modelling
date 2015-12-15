#ifndef CALIBRATION_FTSAMPLEPREDICTION_H
#define CALIBRATION_FTSAMPLEPREDICTION_H

#include <vector>
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

//class for predicted samples


////////////////////////////////////////////////////////////////////////////////////
///Interface of FtSamplePrediction
////////////////////////////////////////////////////////////////////////////////////
class FtSamplePrediction
{
 public:
  
  //public functions
  FtSamplePrediction (const std::string sampleCode);

  ~FtSamplePrediction();

  const std::string& getSampleCode() const;       

  Histogram* getTrackLengths(int indexCl);

 
  void advanceState(double timestep, double temperature);

  bool addClIndexIfNotYetExists(size_t indexCl);

  void calcPredAge();

  void geotrackCorrPredTracklengthDists();

  double getPredAge();

 private:

  //private functions
  void   calcTrackLengths (double timestep, double temperature, int ClIndex);

  void   hBinPrevious (int ix, int binnum, double L0, double c0, double c1,
		       double temperature, double timestep, double *hPrev, 
		       int *ixPrev, double *tPrev);     //!!! input L0 based on geocase-polyfunction has float precision  //!!! static?

  double equivalentTime (double temperature, double h);  //!!! static?

  int hBinIndex (double h, int binnum, double L0, double c0, double c1); //!!! static?

  double hValue (int ix, double L0, double c0, double c1);

  double densityFromR (double r);

  double prob (double z);


  //private data
  std::string               m_sampleCode;
  std::vector<Histogram*>   m_trackLengths;
  double                    m_predAge;
};

}

}//namespace Calibration

#endif
