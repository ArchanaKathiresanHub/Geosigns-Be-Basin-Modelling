#ifndef CALIBRATION_FTPARAMETERS_H
#define CALIBRATION_FTPARAMETERS_H

#include <vector>

//#include "polyfunction.h"
#include "Polyfunction.h"//from CBMGenerics

using CBMGenerics::Polyfunction;

namespace Calibration
{
namespace FissionTracks
{

class FtParameters
{
 public:
  //public interface
  static FtParameters& getInstance();   //get singleton

  double getC0( int indexCl ) const;
  double getC1( int indexCl ) const;
  double getL0( double weightContentCl ) const;    //!!! float? double? of base type might be significant for regression testing
  double getL0( int indexCl ) const;   //!!! float? double? of base type might be significant for regression testing
  
  int    getNumBinClWeightPercent() const;
  double getStartBinClWeightPercent() const;
  double getStepBinClWeightPercent() const;

  int    getNumBinTrackLengths() const;
  double getStartBinTrackLengths() const;
  double getStepBinTrackLengths() const;

  int    getNumBinGrainAge() const;
  double getStepBinGrainAge() const;
  double getStartBinGrainAge() const;

  double getT0() const; 
  double getT0Inv() const;
  double getRenormalizationFactor() const;  // geotrack value

  int    getBinMergingFactor() const;  // merge this number of histogram bins after calculation for output

  double getSecsInMa() const;

  double getKelvinAt0Celsius() const;

  double getU238DecayConstant();

  int indexFromClWeightPercent (double ClPerc); //!!! originally float

  double getUndefinedDouble() const;
  int    getUndefinedInt() const;


 private:
  FtParameters();


  //Polyfunction<float> m_l0;                //!!! float? double? of base type might be significant for regression testing
  Polyfunction m_l0; //fixed to double from CBMGenerics::Polyfunction

  std::vector<double> m_c0;

  std::vector<double> m_c1;

  int    m_numBinClWeightPercent;
  double m_stepBinClWeightPercent;  //!!! float? double? might be significant for regression testing
  double m_startBinClWeightPercent; //!!! float? double? might be significant for regression testing

  int    m_numBinTrackLengths;
  double m_stepBinTrackLengths;   //!!! float? double? might be significant for regression testing
  double m_startBinTrackLengths;  //!!! float? double? might be significant for regression testing

  int     m_numBinGrainAge;
  double  m_stepBinGrainAge;
  double  m_startBinGrainAge;

  double m_t0; 
  double m_t0Inv;
  double m_renormalizationFactor;   //geotrack value

  int    m_binMergingFactor;

  double m_secsInMa;

  double m_kelvinAt0Celsius;

  double m_U238DecayConstant;

  double m_undefinedDouble;
  int    m_undefinedInt;

/* #define FTFACTOR                    2              /\* display binnum =  */
/*                                                       FTBINNUMLENGTHDISTR/FTFACTOR *\/ */

};


inline double FtParameters::getC0( int indexCl ) const
{
  return m_c0[ indexCl ];
}

inline double FtParameters::getC1( int indexCl ) const
{
  return m_c1[ indexCl ];
}

inline double FtParameters::getL0( double weightContentCl ) const
{
  return m_l0.F( weightContentCl );
}

inline int FtParameters::getNumBinClWeightPercent() const
{
  return m_numBinClWeightPercent;
}

inline double FtParameters::getStepBinClWeightPercent() const
{
  return m_stepBinClWeightPercent;
}

inline double FtParameters::getStartBinClWeightPercent() const
{
  return m_startBinClWeightPercent;
}

inline int FtParameters::getNumBinTrackLengths() const
{
  return m_numBinTrackLengths;
}

inline double FtParameters::getStartBinTrackLengths() const
{
  return m_startBinTrackLengths;
}

inline double FtParameters::getStepBinTrackLengths() const
{
  return m_stepBinTrackLengths;
}

inline int FtParameters::getNumBinGrainAge() const
{
  return m_numBinGrainAge;
}

inline double FtParameters::getStepBinGrainAge() const
{
  return m_stepBinGrainAge;
}

inline double FtParameters::getStartBinGrainAge() const
{
  return m_startBinGrainAge;
}

inline double FtParameters::getT0() const
{
  return m_t0;
}

inline double FtParameters::getT0Inv() const
{
  return m_t0Inv;
}

inline double FtParameters::getRenormalizationFactor() const  // geotrack value
{
  return m_renormalizationFactor;
}

inline int FtParameters::getBinMergingFactor() const  // merge this number of histogram bins after calculation for output
{
  return m_binMergingFactor;
}

inline double FtParameters::getSecsInMa() const
{
  return m_secsInMa;
}

inline double FtParameters::getKelvinAt0Celsius() const
{
  return m_kelvinAt0Celsius;
}

}

}

#endif
