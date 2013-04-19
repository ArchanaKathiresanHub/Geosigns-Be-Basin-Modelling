#include "SourceRockNodeInput.h"

#include "Constants.h"

namespace Genex5
{

SourceRockNodeInput::SourceRockNodeInput(const double &in_currentTime, 
                                         const double &in_temperature, 
                                         const double &in_pressure, 
                                         const double &thicknessScaleFactor):
   m_previousTime(in_currentTime),
   m_currentTime(in_currentTime),
   m_startTemperature(in_temperature),
   m_endTemperature(in_temperature),
   m_pressure(in_pressure),
   m_thicknessScaleFactor(thicknessScaleFactor),
   m_lithostaticPressure (Constants::UNDEFINEDVALUE),
   m_hydrostaticPressure (Constants::UNDEFINEDVALUE),
   m_startPorePressure (Constants::UNDEFINEDVALUE),
   m_endPorePressure (Constants::UNDEFINEDVALUE),
   m_porosity (Constants::UNDEFINEDVALUE),
   m_permeability (Constants::UNDEFINEDVALUE),
   m_vre (Constants::UNDEFINEDVALUE)
{
}

SourceRockNodeInput::SourceRockNodeInput ( const double &in_previousTime,
                                           const double &in_currentTime,
                                           const double &in_startTemperature,
                                           const double &in_endTemperature,
                                           const double &in_pressure,
                                           const double &in_lithostaticPressure,
                                           const double &in_hydrostaticPressure,
                                           const double &in_startPorePressure,
                                           const double &in_endPorePressure,
                                           const double &in_porosity,
                                           const double &in_permeability,
                                           const double &in_vre,
                                           const unsigned int i,
                                           const unsigned int j,
                                           const double &thicknessScaleFactor ):
   m_previousTime(in_previousTime),
   m_currentTime(in_currentTime),
   m_startTemperature(in_startTemperature),
   m_endTemperature(in_endTemperature),
   m_pressure(in_pressure),
   m_thicknessScaleFactor(thicknessScaleFactor),
   m_lithostaticPressure (in_lithostaticPressure),
   m_hydrostaticPressure (in_hydrostaticPressure),
   m_startPorePressure (in_startPorePressure),
   m_endPorePressure (in_endPorePressure),
   m_porosity (in_porosity),
   m_permeability ( in_permeability ),
   m_vre ( in_vre ),
   m_i ( i ),
   m_j ( j )
{
}

SourceRockNodeInput::~SourceRockNodeInput()
{

}
void SourceRockNodeInput::OutputOnFile(ofstream &outputfile) const
{
     outputfile<<m_currentTime<<","<<m_endTemperature<<","<<m_pressure<<endl;
}

SourceRockNodeInput::SourceRockNodeInput(const SourceRockNodeInput &theInput):
   m_currentTime(theInput.m_currentTime),
   m_startTemperature(theInput.m_startTemperature),
   m_endTemperature(theInput.m_endTemperature),
   m_pressure(theInput.m_pressure),
   m_thicknessScaleFactor(theInput.m_thicknessScaleFactor),
   m_lithostaticPressure (theInput.m_lithostaticPressure),
   m_hydrostaticPressure (theInput.m_hydrostaticPressure),
   m_startPorePressure (theInput.m_startPorePressure),
   m_endPorePressure (theInput.m_endPorePressure),
   m_porosity (theInput.m_porosity),
   m_permeability (theInput.m_permeability),
   m_vre (theInput.m_vre)

{  

}

SourceRockNodeInput::SourceRockNodeInput(const double &in_currentTime, const SourceRockNodeInput &First, const SourceRockNodeInput &Second):
   m_currentTime(in_currentTime)
{
   double DTimeTotal        = Second.GetCurrentTime() - First.GetCurrentTime();
   double gradientPreviousTemp      = (Second.getPreviousTemperatureCelsius() - First.getPreviousTemperatureCelsius()) / DTimeTotal;
   double gradientTemp      = (Second.GetTemperatureCelsius() - First.GetTemperatureCelsius()) / DTimeTotal;
   double gradientPressure  = (Second.GetPressure() - First.GetPressure() ) / DTimeTotal;
   double gradientLithostaticPressure = ( Second.getLithostaticPressure () - First.getLithostaticPressure ()) / DTimeTotal;

   double gradientHydrostaticPressure = ( Second.getHydrostaticPressure () - First.getHydrostaticPressure ()) / DTimeTotal;
   double gradientStartPorePressure = ( Second.getPreviousPorePressure () - First.getPreviousPorePressure ()) / DTimeTotal;
   double gradientEndPorePressure = ( Second.getPorePressure () - First.getPorePressure ()) / DTimeTotal;
   double gradientPorosity = ( Second.getPorosity () - First.getPorosity ()) / DTimeTotal;
   double gradientPermeability = ( Second.getPermeability () - First.getPermeability ()) / DTimeTotal;
   double gradientVre = ( Second.getVre () - First.getVre ()) / DTimeTotal;

   double gradientthicknessScaleFactor  = (Second.m_thicknessScaleFactor - First.m_thicknessScaleFactor ) / DTimeTotal;
      
   double Dt         = in_currentTime - First.GetCurrentTime();
   m_startTemperature     = First.getPreviousTemperatureCelsius() + Dt * gradientPreviousTemp;
   m_endTemperature     = First.GetTemperatureCelsius() + Dt * gradientTemp;
   m_pressure        = First.GetPressure() + Dt * gradientPressure;
   m_thicknessScaleFactor = First.m_thicknessScaleFactor + Dt * gradientthicknessScaleFactor;

   if ( First.m_lithostaticPressure == Constants::UNDEFINEDVALUE or Second.m_lithostaticPressure == Constants::UNDEFINEDVALUE ) {
      m_lithostaticPressure = Constants::UNDEFINEDVALUE;
   } else {
      m_lithostaticPressure = First.m_lithostaticPressure + Dt * gradientLithostaticPressure;
   }

   if ( First.m_hydrostaticPressure == Constants::UNDEFINEDVALUE or Second.m_hydrostaticPressure == Constants::UNDEFINEDVALUE ) {
      m_hydrostaticPressure = Constants::UNDEFINEDVALUE;
   } else {
      m_hydrostaticPressure = First.m_hydrostaticPressure + Dt * gradientHydrostaticPressure;
   }

   if ( First.m_endPorePressure == Constants::UNDEFINEDVALUE or Second.m_endPorePressure == Constants::UNDEFINEDVALUE ) {
      m_endPorePressure = Constants::UNDEFINEDVALUE;
   } else {
      m_endPorePressure = First.m_endPorePressure + Dt * gradientEndPorePressure;
   }

   if ( First.m_startPorePressure == Constants::UNDEFINEDVALUE or Second.m_startPorePressure == Constants::UNDEFINEDVALUE ) {
      m_startPorePressure = Constants::UNDEFINEDVALUE;
   } else {
      m_startPorePressure = First.m_startPorePressure + Dt * gradientStartPorePressure;
   }

   if ( First.m_porosity == Constants::UNDEFINEDVALUE or Second.m_porosity == Constants::UNDEFINEDVALUE ) {
      m_porosity = Constants::UNDEFINEDVALUE;
   } else {
      m_porosity = First.m_porosity + Dt * gradientPorosity;
   }

   if ( First.m_permeability == Constants::UNDEFINEDVALUE or Second.m_permeability == Constants::UNDEFINEDVALUE ) {
      m_permeability = Constants::UNDEFINEDVALUE;
   } else {
      m_permeability = First.m_permeability + Dt * gradientPermeability;
   }

   if ( First.m_vre == Constants::UNDEFINEDVALUE or Second.m_vre == Constants::UNDEFINEDVALUE ) {
      m_vre = Constants::UNDEFINEDVALUE;
   } else {
      m_vre = First.m_vre + Dt * gradientVre;
   }


}

}
