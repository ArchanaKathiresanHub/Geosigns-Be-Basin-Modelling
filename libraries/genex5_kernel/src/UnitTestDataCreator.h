#ifndef UNITTESTDATACREATOR_H
#define UNITTESTDATACREATOR_H


#include <vector>
namespace Genex5
{

class UnitTestDataCreator
{
public:
  UnitTestDataCreator(const int &in_numberOfTimesteps,
                                                                 const double &in_TemperatureAtStart,
                                                                 const double &in_TemperatureAtEnd,
                                                                 const double &in_heatingRate,
                                                                 const double &in_temperatureGradient,
                                                                 const double &in_surfaceTemperature,
                                                                 const double &in_overBurderDensity,
                                                                 const double &in_maximumPeff,
                                                                 const double &in_Pfract,
                                                                 const bool &in_OpenConditions
                                                                 );

   ~UnitTestDataCreator();
   int GetNumberOfTimesteps() const;
   double GetTemperatureAtStart() const;
   double GetTemperatureGradient() const;
   double GetSurfaceTemperature() const;
   double GetOverBurderDensity() const;
   double GetMaximumPeff() const;
   double GetPfract() const;
   bool SimulateOpenSourceRock()const;

   double GetTimeStepSize() const;
   double GetTimeAtStart() const;
   double GetTimeAtEnd() const;

   //Used in the SourceNodeInputHistoryCreation
   double ComputePressure(const double &in_time) const;//FunPeff(DensOB, PeffMax, Pfract, Zsource)
   double ComputeTemperature(const double &in_time)const ;//FunPeff(DensOB, PeffMax, Pfract, Zsource)

private:
   int m_numberOfTimesteps;
   double m_TemperatureAtStart;
   double m_TemperatureAtEnd;
   double m_heatingRate;
   double m_temperatureGradient;  //temporary variable for POC, VBA,  residing in .RUN file    TCgrad
   double m_surfaceTemperature; //temporary variable for POC, VBA,  residing in .RUN file  TCsurf
   double m_overBurderDensity; //temporary variable for POC, VBA,  residing in .RUN file  DensOB
   double m_maximumPeff;
   double m_Pfract;
   bool   m_openConditions;

   //derived properties
   double m_timeStart;
   double m_timeEnd;
   double ComputeTimeAtEnd();
   double m_depthAtStart;                       //derived
   double m_depthAtEnd;                        //derived
   double ComputeDepth(const double & in_time)const;//start or end,  FunDepth(TCleg(0), TCgrad, TCsurf)
   double m_timestepSize;
   double ComputeTimestepSize();
   double m_burialRate;                           //derived
   double ComputeBurialRate();


};
inline int UnitTestDataCreator::GetNumberOfTimesteps() const
{
   return m_numberOfTimesteps;
}
inline double UnitTestDataCreator::GetTimeAtStart() const
{
   return m_timeStart;
}
inline double UnitTestDataCreator::GetTimeAtEnd() const
{
   return m_timeEnd;
}
inline double UnitTestDataCreator::GetTimeStepSize() const
{
   return m_timestepSize;
}
inline double UnitTestDataCreator::GetTemperatureAtStart() const
{
   return m_TemperatureAtStart;
}
inline double UnitTestDataCreator::GetTemperatureGradient() const
{
   return m_temperatureGradient;
}
inline double UnitTestDataCreator::GetSurfaceTemperature() const
{
   return m_surfaceTemperature;
}
inline double UnitTestDataCreator::GetOverBurderDensity() const
{
   return m_overBurderDensity;
}
inline double UnitTestDataCreator::GetMaximumPeff() const
{
   return m_maximumPeff;
}
inline double UnitTestDataCreator::GetPfract() const
{
   return m_Pfract;
}
inline bool UnitTestDataCreator::SimulateOpenSourceRock()const
{
   return m_openConditions;
}

}
#endif
