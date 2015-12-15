#ifndef _GENEX6__INPUT_H_
#define _GENEX6__INPUT_H_

#include <iostream>
#include <fstream>
#include <iomanip>
#include "Constants.h"
using namespace std;

#include "Constants.h"
namespace Genex6
{
class Input 
{
public:
   Input(const double in_currentTime, const double in_temperature, 
         const double in_pressure, const double thicknessScaleFactor = 1.0);

   Input ( const double in_previousTime,
           const double in_currentTime,
           const double in_startTemperature,
           const double in_endTemperature,
           const double in_pressure, // Should this be ves?
           const double in_lithostaticPressure,
           const double in_hydrostaticPressure,
           const double in_startPorePressure,
           const double in_endPorePressure,
           const double in_porosity,
           const double in_permeability,
           const double in_vre,
           const unsigned int i,
           const unsigned int j,
           const double thicknessScaleFactor = 1.0);

   Input(const Input &theInput);
   Input(const double in_currentTime, const Input &First, const Input &Second);

   virtual ~Input(){}

   double GetTime() const;
   double GetPreviousTime() const;
   double GetTemperatureCelsius() const;
   double GetTemperatureKelvin() const;
   double GetPressure() const;
   double GetThicknessScaleFactor()const;

   void SetTemperature(const double &in_temp);

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

   /// \brief Return the VRe of the current input.
   double getVre () const;

   /// \brief Return the i-position of the surrnt input.
   unsigned int getI () const;

   /// \brief Return the j-position of the surrnt input.
   unsigned int getJ () const;


   void OutputOnFile(ofstream &outputfile) const;
   void OutputOnFile(FILE * fp) const;

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


inline void Input::SetTemperature(const double &in_temp)
{
   m_endTemperature = in_temp;
}
inline double Input::GetTime() const
{
   return m_currentTime;
}
inline double Input::GetPreviousTime() const
{
   return m_previousTime;
}
inline double Input::GetTemperatureCelsius() const
{
   return m_endTemperature;
}
inline double Input::GetPressure() const
{
   return  m_pressure;
}
inline double Input::GetTemperatureKelvin() const
{
   return (m_endTemperature + Genex6::Constants::s_TCabs);
}
inline double Input::GetThicknessScaleFactor()const
{
   return m_thicknessScaleFactor;
}
inline double Input::getPreviousTemperatureCelsius() const
{
   return  m_startTemperature;
}
inline double Input::getPreviousTemperatureKelvin() const
{
   return  (m_startTemperature+Genex6::Constants::s_TCabs);
}
inline double Input::getLithostaticPressure () const {
   return m_lithostaticPressure;
}

inline double Input::getHydrostaticPressure () const {
   return m_hydrostaticPressure;
}

inline double Input::getPorePressure () const {
   return m_endPorePressure;
}

inline double Input::getPreviousPorePressure () const {
   return m_startPorePressure;
}

inline double Input::getPorosity () const {
   return m_porosity;
}

inline double Input::getPermeability () const {
   return m_permeability;
}

inline double Input::getVre () const {
   return m_vre;
}

inline unsigned int Input::getI () const {
   return m_i;
}

inline unsigned int Input::getJ () const {
   return m_j;
}

}
#endif // _GENEX6__INPUT_H_
