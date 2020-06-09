#include "ftgrain.h"
#include "ftparameters.h"

namespace Calibration
{
namespace FissionTracks
{

FtGrain::FtGrain(const std::string& sampleId, int grainId, int spontTrackNo, int inducedTrackNo, double clWeightPercent)
{
   m_sampleId = sampleId;
   m_grainId = grainId;
   m_spontTrackNo = spontTrackNo;
   m_inducedTrackNo = inducedTrackNo; 
   m_clWeightPerc = clWeightPercent;  
}


std::string FtGrain::getSampleId() const
{
   return m_sampleId;
}

int     FtGrain::getGrainId() const
{
   return m_grainId;
}

int     FtGrain::getSpontTrackNo() const
{
   return m_spontTrackNo ;
}

int     FtGrain::getInducedTrackNo() const
{
   return m_inducedTrackNo;
}

double  FtGrain::getClWeightPercent() const
{
   return m_clWeightPerc;
}

double  FtGrain::getGrainAge() const
{
   return m_grainAge;
}

void   FtGrain::addTrackLength(const double length)
{
   m_trackLengths.push_back(length);
}

list<double>&  FtGrain::getTrackLengths()
{
   return m_trackLengths;
}

void FtGrain::setGrainAge(double grainAge)
{
   m_grainAge = grainAge ;
}

}

}//Calibration namespace
