//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "Input.h"
#include "ComponentManager.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

namespace Genex6
{
Input::Input(const double in_currentTime, const double in_temperature,
             const double in_pressure, const double vre, const double in_PorePressure, const double thicknessScaleFactor):
   m_previousTime(in_currentTime),
   m_currentTime(in_currentTime),
   m_startTemperature(in_temperature),
   m_endTemperature(in_temperature),
   m_pressure(in_pressure),
   m_thicknessScaleFactor(thicknessScaleFactor),
   m_lithostaticPressure (CauldronNoDataValue),
   m_hydrostaticPressure (CauldronNoDataValue),
   m_startPorePressure   (in_PorePressure),
   m_endPorePressure     (in_PorePressure),
   m_porosity            (CauldronNoDataValue),
   m_permeability        (CauldronNoDataValue),
   m_vre                 (vre)
{
}

Input::Input ( const double in_previousTime,
               const double in_currentTime,
               const double in_startTemperature,
               const double in_endTemperature,
               const double in_pressure,
               const double in_lithostaticPressure,
               const double in_hydrostaticPressure,
               const double in_startPorePressure,
               const double in_endPorePressure,
               const double in_porosity,
               const double in_permeability,
               const double in_vre,
               const unsigned int i,
               const unsigned int j,
               const double thicknessScaleFactor ) :
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

Input::Input(const Input &theInput):
   m_currentTime(theInput.m_currentTime),
   m_endTemperature(theInput.m_endTemperature),
   m_pressure(theInput.m_pressure),
   m_thicknessScaleFactor(theInput.m_thicknessScaleFactor) 
{
  
}
Input::Input(const double in_currentTime, const Input &First, const Input &Second):
   m_currentTime(in_currentTime) 
{

   double DTimeTotal        = Second.GetTime() - First.GetTime();
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
      
   double Dt         = in_currentTime - First.GetTime();
   m_startTemperature     = First.getPreviousTemperatureCelsius() + Dt * gradientPreviousTemp;
   m_endTemperature     = First.GetTemperatureCelsius() + Dt * gradientTemp;
   m_pressure        = First.GetPressure() + Dt * gradientPressure;
   m_thicknessScaleFactor = First.m_thicknessScaleFactor + Dt * gradientthicknessScaleFactor;

   if ( First.m_lithostaticPressure == CauldronNoDataValue or Second.m_lithostaticPressure == CauldronNoDataValue ) {
      m_lithostaticPressure = CauldronNoDataValue;
   } else {
      m_lithostaticPressure = First.m_lithostaticPressure + Dt * gradientLithostaticPressure;
   }

   if ( First.m_hydrostaticPressure == CauldronNoDataValue or Second.m_hydrostaticPressure == CauldronNoDataValue ) {
      m_hydrostaticPressure = CauldronNoDataValue;
   } else {
      m_hydrostaticPressure = First.m_hydrostaticPressure + Dt * gradientHydrostaticPressure;
   }

   if ( First.m_endPorePressure == CauldronNoDataValue or Second.m_endPorePressure == CauldronNoDataValue ) {
      m_endPorePressure = CauldronNoDataValue;
   } else {
      m_endPorePressure = First.m_endPorePressure + Dt * gradientEndPorePressure;
   }

   if ( First.m_startPorePressure == CauldronNoDataValue or Second.m_startPorePressure == CauldronNoDataValue ) {
      m_startPorePressure = CauldronNoDataValue;
   } else {
      m_startPorePressure = First.m_startPorePressure + Dt * gradientStartPorePressure;
   }

   if ( First.m_porosity == CauldronNoDataValue or Second.m_porosity == CauldronNoDataValue ) {
      m_porosity = CauldronNoDataValue;
   } else {
      m_porosity = First.m_porosity + Dt * gradientPorosity;
   }

   if ( First.m_permeability == CauldronNoDataValue or Second.m_permeability == CauldronNoDataValue ) {
      m_permeability = CauldronNoDataValue;
   } else {
      m_permeability = First.m_permeability + Dt * gradientPermeability;
   }

   if ( First.m_vre == CauldronNoDataValue or Second.m_vre == CauldronNoDataValue ) {
      m_vre = CauldronNoDataValue;
   } else {
      m_vre = First.m_vre + Dt * gradientVre;
   }

#if 0
   double DTimeTotal        = Second.GetTime() - First.GetTime();
   double gradientTemp      = (Second.GetTemperatureCelsius() - First.GetTemperatureCelsius()) / DTimeTotal;
   double gradientPressure  = (Second.GetPressure() - First.GetPressure() ) / DTimeTotal;
	double gradientthicknessScaleFactor  = (Second.m_thicknessScaleFactor - First.m_thicknessScaleFactor ) / DTimeTotal;
   
   double Dt     = in_currentTime - First.GetTime();
   m_endTemperature = First.GetTemperatureCelsius() + Dt * gradientTemp;
   m_pressure    = First.GetPressure() + Dt * gradientPressure;
	m_thicknessScaleFactor = First.m_thicknessScaleFactor + Dt * gradientthicknessScaleFactor;
#endif
}
void Input::OutputOnFile(ofstream &outputfile) const 
{
   outputfile << m_currentTime << "," << m_endTemperature << "," << m_pressure << endl;
}
void Input::OutputOnFile(FILE * fp) const 
{
   fprintf(fp, "%.15lf, %.15lf, %.15lf,\n", m_currentTime,  m_endTemperature, m_pressure);
}
 
}
