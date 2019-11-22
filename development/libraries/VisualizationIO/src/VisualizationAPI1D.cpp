//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationAPI.h"
#include <assert.h>
#include <new>
#include <algorithm>
#include <cstring>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4290)
#endif

using namespace CauldronIO;
using namespace std;

/// DisplayContourIoTbl

CauldronIO::DisplayContour::DisplayContour()
{
   std::memset((void*)this, 0, sizeof(DisplayContour));

}

void CauldronIO::DisplayContour::setContourColour(const std::string& colour)
{
   std::strcpy(m_contourColour, colour.c_str());
}

void CauldronIO::DisplayContour::setPropertyName(const std::string& name)
{
   std::strcpy(m_name, name.c_str());
}

void CauldronIO::DisplayContour::setContourValue(float value)
{
   m_contourValue = value;
}

const std::string CauldronIO::DisplayContour::getContourColour() const
{
   return std::string(m_contourColour);
}

const std::string CauldronIO::DisplayContour::getPropertyName() const
{
   return std::string(m_name);
}

float CauldronIO::DisplayContour::getContourValue() const
{
   return m_contourValue;
}

/// IsoEntry
CauldronIO::IsoEntry::IsoEntry()
{
   m_age = 0.0f;
   m_contourValue = 0.0f;
   m_numberOfPoints = 0;
   m_sum = 0.0;
}

void CauldronIO::IsoEntry::setContourValue(float value)
{
   m_contourValue = value;
}

void CauldronIO::IsoEntry::setAge(float age)
{
   m_age = age;
}

void CauldronIO::IsoEntry::setSum(double sum)
{
   m_sum = sum;
}

void CauldronIO::IsoEntry::setNP(int np)
{
   m_numberOfPoints = np;
}

float CauldronIO::IsoEntry::getContourValue() const
{
   return m_contourValue;
}

float CauldronIO::IsoEntry::getAge() const
{
   return m_age;
}

double CauldronIO::IsoEntry::getSum() const
{
   return m_sum;
}

int CauldronIO::IsoEntry::getNP() const
{
   return m_numberOfPoints;
}

/// Biomarkerm
CauldronIO::Biomarkerm::Biomarkerm()
{
   m_depthIndex = 0.0f;
   m_hopaneIsomerisation = 0.0f;
   m_steraneIsomerisation = 0.0f;
   m_steraneAromatisation = 0.0f;
   m_optimization = false;
}

void CauldronIO::Biomarkerm::setOptimization(bool value)
{
   m_optimization = value;
}

void CauldronIO::Biomarkerm::setSteraneAromatisation(float value)
{
  m_steraneAromatisation = value;
}

void CauldronIO::Biomarkerm::setSteraneIsomerisation(float value)
{
   m_steraneIsomerisation = value;
}

void CauldronIO::Biomarkerm::setDepthIndex(float value)
{
   m_depthIndex = value;
}

void CauldronIO::Biomarkerm::setHopaneIsomerisation(float value)
{
   m_hopaneIsomerisation = value;
}

float CauldronIO::Biomarkerm::getDepthIndex() const
{
   return m_depthIndex;
}

float CauldronIO::Biomarkerm::getHopaneIsomerisation() const
{
   return m_hopaneIsomerisation;
}

float CauldronIO::Biomarkerm::getSteraneIsomerisation() const
{
   return  m_steraneIsomerisation;
}

float CauldronIO::Biomarkerm::getSteraneAromatisation() const
{
   return m_steraneAromatisation;
}

bool CauldronIO::Biomarkerm::getOptimization() const
{
   return m_optimization;
}

/// FtSample
CauldronIO::FtSample::FtSample()
{
   // Clear our buffers
   std::memset((void*)this, 0, sizeof(FtSample));
}

const std::string CauldronIO::FtSample::getFtSampleId() const
{
   return  std::string(m_FtSampleId);
}

void CauldronIO::FtSample::setFtSampleId(const std::string& value)
{
   std::strcpy(m_FtSampleId, value.c_str());
}


float CauldronIO::FtSample::getDepthIndex() const
{
   return m_DepthIndex;
}

void CauldronIO::FtSample::setDepthIndex(float value)
{
   m_DepthIndex = value;
}


float CauldronIO::FtSample::getFtZeta() const
{
   return m_FtZeta;
}

void CauldronIO::FtSample::setFtZeta(float value)
{
   m_FtZeta = value;
}


float CauldronIO::FtSample::getFtUstglTrackDensity() const
{
   return m_FtUstglTrackDensity;
}

void CauldronIO::FtSample::setFtUstglTrackDensity(float value)
{
   m_FtUstglTrackDensity = value;
}


float CauldronIO::FtSample::getFtPredictedAge() const
{
   return m_FtPredictedAge;
}

void CauldronIO::FtSample::setFtPredictedAge(float value)
{
   m_FtPredictedAge = value;
}


float CauldronIO::FtSample::getFtPooledAge() const
{
   return m_FtPooledAge;
}

void CauldronIO::FtSample::setFtPooledAge(float value)
{
   m_FtPooledAge = value;
}


float CauldronIO::FtSample::getFtPooledAgeErr() const
{
   return m_FtPooledAgeErr;
}

void CauldronIO::FtSample::setFtPooledAgeErr(float value)
{
   m_FtPooledAgeErr = value;
}


float CauldronIO::FtSample::getFtAgeChi2() const
{
   return m_FtAgeChi2;
}

void CauldronIO::FtSample::setFtAgeChi2(float value)
{
   m_FtAgeChi2 = value;
}


int CauldronIO::FtSample::getFtDegreeOfFreedom() const
{
   return m_FtDegreeOfFreedom;
}

void CauldronIO::FtSample::setFtDegreeOfFreedom(int value)
{
  m_FtDegreeOfFreedom = value;
}


float CauldronIO::FtSample::getFtPAgeChi2() const
{
   return m_FtPAgeChi2;
}

void CauldronIO::FtSample::setFtPAgeChi2(float value)
{
   m_FtPAgeChi2 = value;
}


float CauldronIO::FtSample::getFtCorrCoeff() const
{
   return m_FtCorrCoeff;
}

void CauldronIO::FtSample::setFtCorrCoeff(float value)
{
  m_FtCorrCoeff = value;
}


float CauldronIO::FtSample::getFtVarianceSqrtNs() const
{
   return m_FtVarianceSqrtNs;
}

void CauldronIO::FtSample::setFtVarianceSqrtNs(float value)
{
   m_FtVarianceSqrtNs = value;
}


float CauldronIO::FtSample::getFtVarianceSqrtNi() const
{
   return m_FtVarianceSqrtNi;
}

void CauldronIO::FtSample::setFtVarianceSqrtNi(float value)
{
   m_FtVarianceSqrtNi = value;
}


float CauldronIO::FtSample::getFtNsDivNi() const
{
   return m_FtNsDivNi;
}

void CauldronIO::FtSample::setFtNsDivNi(float value)
{
   m_FtNsDivNi = value;
}


float CauldronIO::FtSample::getFtNsDivNiErr() const
{
   return m_FtNsDivNiErr;
}

void CauldronIO::FtSample::setFtNsDivNiErr(float value)
{
   m_FtNsDivNiErr = value;
}


float CauldronIO::FtSample::getFtMeanRatio() const
{
   return m_FtMeanRatio;
}

void CauldronIO::FtSample::setFtMeanRatio(float value)
{
   m_FtMeanRatio = value;
}


float CauldronIO::FtSample::getFtMeanRatioErr() const
{
   return m_FtMeanRatioErr;
}

void CauldronIO::FtSample::setFtMeanRatioErr(float value)
{
   m_FtMeanRatioErr = value;
}


float CauldronIO::FtSample::getFtCentralAge() const
{
   return m_FtCentralAge;
}

void CauldronIO::FtSample::setFtCentralAge(float value)
{
  m_FtCentralAge = value;
}


float CauldronIO::FtSample::getFtCentralAgeErr() const
{
   return m_FtCentralAgeErr;
}

void CauldronIO::FtSample::setFtCentralAgeErr(float value)
{
   m_FtCentralAgeErr = value;
}

float CauldronIO::FtSample::getFtMeanAge() const
{
   return m_FtMeanAge;
}

void CauldronIO::FtSample::setFtMeanAge(float value)
{
   m_FtMeanAge = value;
}


float CauldronIO::FtSample::getFtMeanAgeErr() const
{
   return m_FtMeanAgeErr;
}

void CauldronIO::FtSample::setFtMeanAgeErr(float value)
{
   m_FtMeanAgeErr = value;
}


float CauldronIO::FtSample::getFtLengthChi2() const
{
   return m_FtLengthChi2;
}

void CauldronIO::FtSample::setFtLengthChi2(float value)
{
   m_FtLengthChi2 = value;
}


const std::string CauldronIO::FtSample::getFtApatiteYield() const
{
   return std::string(m_FtApatiteYield);
}

void CauldronIO::FtSample::setFtApatiteYield( const std::string & value )
{
   std::strcpy(m_FtApatiteYield, value.c_str());
}

///FtGrain
CauldronIO::FtGrain::FtGrain()
{
   // Clear our buffers
   std::memset((void*)this, 0, sizeof(FtGrain));
}

const std::string CauldronIO::FtGrain::getFtSampleId() const
{
   return std::string(m_FtSampleId);
}

void CauldronIO::FtGrain::setFtSampleId(const std::string & val) {
   std::strcpy(m_FtSampleId, val.c_str());
}

int CauldronIO::FtGrain::getFtGrainId() const
{
   return m_FtGrainId;
}

void CauldronIO::FtGrain::setFtGrainId(int val)
{
   m_FtGrainId = val;
}

int CauldronIO::FtGrain::getFtSpontTrackNo() const
{
   return m_FtSpontTrackNo;
}

void CauldronIO::FtGrain::setFtSpontTrackNo(int val)
{
   m_FtSpontTrackNo = val;
}

int CauldronIO::FtGrain::getFtInducedTrackNo() const
{
   return m_FtInducedTrackNo;
}

void CauldronIO::FtGrain::setFtInducedTrackNo(int val)
{
   m_FtInducedTrackNo = val;
}

double CauldronIO::FtGrain::getFtClWeightPerc() const
{
   return m_FtClWeightPerc;
}

void CauldronIO::FtGrain::setFtClWeightPerc(float val)
{
   m_FtClWeightPerc = val;
}

double CauldronIO::FtGrain::getFtGrainAge() const
{
   return m_FtGrainAge;
}

void CauldronIO::FtGrain::setFtGrainAge(float val)
{
   m_FtGrainAge = val;
}

/// FtPredLengthCountsHist
CauldronIO::FtPredLengthCountsHist::FtPredLengthCountsHist()
{
   // Clear our buffers
   std::memset((void*)this, 0, sizeof(FtPredLengthCountsHist));
}

int CauldronIO::FtPredLengthCountsHist::getFtPredLengthHistId() const
{
   return m_FtPredLengthHistId;
}

void CauldronIO::FtPredLengthCountsHist::setFtPredLengthHistId(int val)
{
  m_FtPredLengthHistId = val;
}

const std::string CauldronIO::FtPredLengthCountsHist::getFtSampleId() const
{
   return std::string(m_FtSampleId);
}

void CauldronIO::FtPredLengthCountsHist::setFtSampleId(const std::string & val)
{
   std::strcpy(m_FtSampleId, val.c_str());
}

float CauldronIO::FtPredLengthCountsHist::getFtClWeightPerc() const
{
   return m_FtClWeightPerc;
}

void CauldronIO::FtPredLengthCountsHist::setFtClWeightPerc(float val)
{
   m_FtClWeightPerc = val;
}

float CauldronIO::FtPredLengthCountsHist::getFtPredLengthBinStart() const
{
   return m_FtPredLengthBinStart;
}

void CauldronIO::FtPredLengthCountsHist::setFtPredLengthBinStart(float val)
{
   m_FtPredLengthBinStart = val;
}

float CauldronIO::FtPredLengthCountsHist::getFtPredLengthBinWidth() const
{
   return m_FtPredLengthBinWidth;
}

void CauldronIO::FtPredLengthCountsHist::setFtPredLengthBinWidth(float val)
{
   m_FtPredLengthBinWidth = val;
}

int CauldronIO::FtPredLengthCountsHist::getFtPredLengthBinNum() const
{
   return m_FtPredLengthBinNum;
}

void CauldronIO::FtPredLengthCountsHist::setFtPredLengthBinNum(int val)
{
   m_FtPredLengthBinNum = val;
}

/// FtPredLengthCountsHistData
CauldronIO::FtPredLengthCountsHistData::FtPredLengthCountsHistData()
{
   // Clear our buffers
   m_FtPredLengthHistId = 0;
   m_FtPredLengthBinIndex = 0;
   m_FtPredLengthBinCount = 0.0;
}

int CauldronIO::FtPredLengthCountsHistData::getFtPredLengthHistId() const
{
   return m_FtPredLengthHistId;
}

void CauldronIO::FtPredLengthCountsHistData::setFtPredLengthHistId(int val)
{
   m_FtPredLengthHistId = val;
}

int CauldronIO::FtPredLengthCountsHistData::getFtPredLengthBinIndex() const
{
   return m_FtPredLengthBinIndex;
}

void CauldronIO::FtPredLengthCountsHistData::setFtPredLengthBinIndex(int val)
{
   m_FtPredLengthBinIndex = val;
}

float CauldronIO::FtPredLengthCountsHistData::getFtPredLengthBinCount() const
{
   return m_FtPredLengthBinCount;
}

void CauldronIO::FtPredLengthCountsHistData::setFtPredLengthBinCount(float val)
{
   m_FtPredLengthBinCount = val;
}

/// FtClWeightPercBinsTbl
CauldronIO::FtClWeightPercBins::FtClWeightPercBins()
{
   m_FtClWeightBinStart = 0.0;
   m_FtClWeightBinWidth = 0.0;
}

double CauldronIO::FtClWeightPercBins::getFtClWeightBinStart() const
{
   return m_FtClWeightBinStart;
}

void CauldronIO::FtClWeightPercBins::setFtClWeightBinStart(double val)
{
   m_FtClWeightBinStart = val;
}

double CauldronIO::FtClWeightPercBins::getFtClWeightBinWidth() const
{
   return m_FtClWeightBinWidth;
}

void CauldronIO::FtClWeightPercBins::setFtClWeightBinWidth(double val)
{
   m_FtClWeightBinWidth = val;
}

/// SmectiteIllite
CauldronIO::SmectiteIllite::SmectiteIllite()
{
   std::memset((void*)this, 0, sizeof(SmectiteIllite));
}

float CauldronIO::SmectiteIllite::getDepthIndex() const
{
   return m_DepthIndex;
}

void CauldronIO::SmectiteIllite::setDepthIndex(float val)
{
   m_DepthIndex = val;
}

float CauldronIO::SmectiteIllite::getIlliteFraction() const
{
   return m_IlliteFraction;
}

void CauldronIO::SmectiteIllite::setIlliteFraction(float val)
{
   m_IlliteFraction = val;
}

const std::string CauldronIO::SmectiteIllite::getLabel() const
{
   return std::string(m_Label);
}

void CauldronIO::SmectiteIllite::setLabel(const std::string & val)
{
   std::strcpy(m_Label, val.c_str());
}

bool CauldronIO::SmectiteIllite::getOptimization() const
{
   return m_Optimization;
}

void CauldronIO::SmectiteIllite::setOptimization(bool val)
{
   m_Optimization = val;
}


// DepthIo
CauldronIO::DepthIo::DepthIo()
{
   std::memset((void*)this, 0, sizeof(DepthIo));
}
const std::string CauldronIO::DepthIo::getPropertyName() const
{
   return std::string(m_PropertyName);
}

void CauldronIO::DepthIo::setPropertyName(const std::string & val)
{
   std::strcpy(m_PropertyName, val.c_str());
}

float CauldronIO::DepthIo::getTime() const
{
   return m_Time;
}
void CauldronIO::DepthIo::setTime(float val)
{
   m_Time = val;
}

float CauldronIO::DepthIo::getDepth() const
{
   return m_Depth;
}
void CauldronIO::DepthIo::setDepth(float val)
{
   m_Depth = val;
}

float CauldronIO::DepthIo::getAverage() const
{
   return m_Average;
}
void CauldronIO::DepthIo::setAverage(float val)
{
   m_Average = val;
}

float CauldronIO::DepthIo::getStandardDev() const
{
   return m_StandardDev;
}
void CauldronIO::DepthIo::setStandardDev(float val)
{
   m_StandardDev = val;
}

float CauldronIO::DepthIo::getMinimum() const
{
   return m_Minimum;
}
void CauldronIO::DepthIo::setMinimum(float val)
{
   m_Minimum = val;
}

float CauldronIO::DepthIo::getMaximum() const
{
   return m_Maximum;
}
void CauldronIO::DepthIo::setMaximum(float val)
{
   m_Maximum = val;
}

float CauldronIO::DepthIo::getSum() const
{
   return m_Sum;
}
void CauldronIO::DepthIo::setSum(float val)
{
   m_Sum = val;
}

float CauldronIO::DepthIo::getSum2() const
{
   return m_Sum2;
}
void CauldronIO::DepthIo::setSum2(float val)
{
   m_Sum2 = val;
}

int CauldronIO::DepthIo::getNP() const
{
   return m_NP;
}
void CauldronIO::DepthIo::setNP(int val)
{
   m_NP = val;
}

float CauldronIO::DepthIo::getP15() const
{
   return m_P15;
}
void CauldronIO::DepthIo::setP15(float val)
{
   m_P15 = val;
}

float CauldronIO::DepthIo::getP50() const
{
   return m_P50;
}
void CauldronIO::DepthIo::setP50(float val)
{
   m_P50 = val;
}

float CauldronIO::DepthIo::getP85() const
{
   return m_P85;
}
void CauldronIO::DepthIo::setP85(float val)
{
   m_P85 = val;
}

float CauldronIO::DepthIo::getSumFirstPower() const
{
   return m_SumFirstPower;
}
void CauldronIO::DepthIo::setSumFirstPower(float val)
{
   m_SumFirstPower = val;
}

float CauldronIO::DepthIo::getSumSecondPower() const
{
   return m_SumSecondPower;
}
void CauldronIO::DepthIo::setSumSecondPower(float val)
{
   m_SumSecondPower = val;
}

float CauldronIO::DepthIo::getSumThirdPower() const
{
   return m_SumThirdPower;
}
void CauldronIO::DepthIo::setSumThirdPower(float val)
{
   m_SumThirdPower = val;
}

float CauldronIO::DepthIo::getSumFourthPower() const
{
   return m_SumFourthPower;
}
void CauldronIO::DepthIo::setSumFourthPower(float val)
{
   m_SumFourthPower = val;
}

float CauldronIO::DepthIo::getSkewness() const
{
   return m_Skewness;
}
void CauldronIO::DepthIo::setSkewness(float val)
{
   m_Skewness = val;
}

float CauldronIO::DepthIo::getKurtosis() const
{
   return m_Kurtosis;
}
void CauldronIO::DepthIo::setKurtosis(float val)
{
   m_Kurtosis = val;
}

/// 1DTimeIo
CauldronIO::TimeIo1D::TimeIo1D()
{
   std::memset((void*)this, 0, sizeof(TimeIo1D));

}

float CauldronIO::TimeIo1D::getTime() const
{
   return m_Time;
}

void CauldronIO::TimeIo1D::setTime(float val)
{
   m_Time = val;
}

const std::string CauldronIO::TimeIo1D::getPropertyName() const
{
   return std::string(m_PropertyName);
}

void CauldronIO::TimeIo1D::setPropertyName(const std::string & val)
{
   std::strcpy(m_PropertyName, val.c_str());
}

const std::string CauldronIO::TimeIo1D::getFormationName() const
{
   return std::string(m_FormationName);
}
void CauldronIO::TimeIo1D::setFormationName(const std::string & val)
{
   std::strcpy(m_FormationName, val.c_str());
}
int CauldronIO::TimeIo1D::getNodeIndex() const
{
   return m_NodeIndex;
}

void CauldronIO::TimeIo1D::setNodeIndex(int val)
{
   m_NodeIndex = val;
}

const std::string CauldronIO::TimeIo1D::getSurfaceName() const
{
   return std::string(m_SurfaceName);
}

void CauldronIO::TimeIo1D::setSurfaceName(const std::string & val)
{
   std::strcpy(m_SurfaceName, val.c_str());
}

float CauldronIO::TimeIo1D::getValue() const
{
   return m_Value;
}

void CauldronIO::TimeIo1D::setValue(float val)
{
   m_Value = val;
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

