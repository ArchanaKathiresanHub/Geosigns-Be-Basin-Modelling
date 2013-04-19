#ifndef SOURCENODEINPUT_H
#define SOURCENODEINPUT_H


#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include<fstream>
      #include<iomanip>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<fstream.h>
      #include<iostream.h>
      #include<iomanip.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include<iostream>
   #include <fstream>
   #include<iomanip>
   using namespace std;
#endif // sgi

#include "Constants.h"
namespace Genex5
{
//!Input for a Genex5 simulation
class SourceRockNodeInput 
{
public:
   SourceRockNodeInput(const double &in_currentTime,  const double &in_temperature, const double &in_pressure, const double 
                       &thicknessScaleFactor = 1.0);

   SourceRockNodeInput(const double &in_previousTime,
                       const double &in_currentTime,
                       const double &in_startTemperature,
                       const double &in_endTemperature,
                       const double &in_pressure, // Should this be ves?
                       const double &in_lithostaticPressure,
                       const double &in_hydrostaticPressure,
                       const double &in_startPorePressure,
                       const double &in_endPorePressure,
                       const double &in_porosity,
                       const double &in_permeability,
                       const double &in_vre,
                       const unsigned int i,
                       const unsigned int j,
                       const double &thicknessScaleFactor = 1.0);

   SourceRockNodeInput(const SourceRockNodeInput &theInput);
   SourceRockNodeInput(const double &in_currentTime, const SourceRockNodeInput &First, const SourceRockNodeInput &Second);
   ~SourceRockNodeInput();

   double GetPreviousTime() const;
   double GetCurrentTime() const;
   double GetTemperatureCelsius() const;
   double GetTemperatureKelvin() const;
   double GetPressure() const;
   double GetThicknessScaleFactor()const;

   /// Temperature from previous time-step.
   double getPreviousTemperatureCelsius () const;

   /// Temperature from previous time-step.
   double getPreviousTemperatureKelvin () const;

   /// Lithostatic pressure in Pa.
   double getLithostaticPressure () const;

   /// Hydrostatic pressure in Pa.
   double getHydrostaticPressure () const;

   /// Pore-pressure pressure in Pa.
   double getPorePressure () const;

   /// Pore-pressure pressure in Pa frmo previous time-step.
   double getPreviousPorePressure () const;

   /// Get porosity in fraction.
   double getPorosity () const;

   /// Get permeability in mD.
   double getPermeability () const;

   double getVre () const;

   void SetTemperature(const double &in_temp);

   unsigned int getI () const;

   unsigned int getJ () const;

   void OutputOnFile(ofstream &outputfile) const;

private:
   //temperature Celsius, pressure in Pa, Time in Ma
   double m_previousTime;             
   double m_currentTime;             
   double m_startTemperature;
   double m_endTemperature;
   double m_pressure;
   double m_thicknessScaleFactor;


   double m_lithostaticPressure;
   double m_hydrostaticPressure;
   double m_startPorePressure;
   double m_endPorePressure;
   double m_porosity;
   double m_permeability;

   double m_vre;

   unsigned int m_i;
   unsigned int m_j;

};


inline void SourceRockNodeInput::SetTemperature(const double &in_temp)
{
   m_endTemperature=in_temp;
}
inline double SourceRockNodeInput::GetPreviousTime() const
{
   return  m_previousTime;
}
inline double SourceRockNodeInput::GetCurrentTime() const
{
   return  m_currentTime;
}
inline double SourceRockNodeInput::GetTemperatureCelsius() const
{
   return  m_endTemperature;
}
inline double SourceRockNodeInput::GetPressure() const
{
   return  m_pressure;
}
inline double SourceRockNodeInput::GetTemperatureKelvin() const
{
   return  (m_endTemperature+Genex5::Constants::s_TCabs);
}
inline double SourceRockNodeInput::getPreviousTemperatureCelsius() const
{
   return  m_startTemperature;
}
inline double SourceRockNodeInput::getPreviousTemperatureKelvin() const
{
   return  (m_startTemperature+Genex5::Constants::s_TCabs);
}
inline double SourceRockNodeInput::GetThicknessScaleFactor()const
{
   return m_thicknessScaleFactor;
}

inline double SourceRockNodeInput::getLithostaticPressure () const {
   return m_lithostaticPressure;
}

inline double SourceRockNodeInput::getHydrostaticPressure () const {
   return m_hydrostaticPressure;
}

inline double SourceRockNodeInput::getPorePressure () const {
   return m_endPorePressure;
}

inline double SourceRockNodeInput::getPreviousPorePressure () const {
   return m_startPorePressure;
}

inline double SourceRockNodeInput::getPorosity () const {
   return m_porosity;
}

inline double SourceRockNodeInput::getPermeability () const {
   return m_permeability;
}

inline double SourceRockNodeInput::getVre () const {
   return m_vre;
}

inline unsigned int SourceRockNodeInput::getI () const {
   return m_i;
}

inline unsigned int SourceRockNodeInput::getJ () const {
   return m_j;
}


}




#endif
