//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef EOS_PACK_C_API_H
#define EOS_PACK_C_API_H

#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

/// \file EosPackCAPI.h
/// \brief Provides backward compatibility with C like API to EosPack library
///        The new implementation of API use swig
namespace pvtFlash
{
   struct ComputeStruct
   {
   public:
      double temperature;
      double pressure;
      double compMasses     [ComponentId::NUMBER_OF_SPECIES];
      double phaseCompMasses[PhaseId::NUMBER_OF_PHASES * ComponentId::NUMBER_OF_SPECIES];
      double phaseDensity   [PhaseId::NUMBER_OF_PHASES];
      double phaseViscosity [PhaseId::NUMBER_OF_PHASES];
      bool isGormPrescribed;
      double gorm;        
   };

   extern "C" 
   {
      void SetPvtPropertiesConfigFile(const char* fileName);     
      bool EosPackComputeWithLumping(ComputeStruct* computeInfo);

      /// \brief Calculate phase masses, phase density and phase viscosity using C arrays as parameters
      /// \param temperature temperature of composition
      /// \param pressure pressure of composition
      /// \param[in] compMasses array of size 23 with mass for each component
      /// \param isGormPrescribed is gas/oil ration coeficient is given
      /// \param gorm value of gas/oil ration coeficient 
      /// \param[out] phaseCompMasses aray of size 46 with masses for each phase for each component
      /// \param phaseDensity array of size 2 with densities for liquid/vapour phases
      /// \param phaseViscosity array of size 2 with viscosities for liquid/vapour phases
      /// \return true if flashing was successfull, false otherwise
      bool EosPackComputeWithLumpingArr( double temperature, double pressure, double * compMasses, bool isGormPrescribed, double gorm,
                                         double * phaseCompMasses, double * phaseDensity, double * phaseViscosity );

      double GetMolWeight(int componentId, double gorm);     

      /// \brief Calculate gas/oil ratio for given composition
      /// \param[in] compMasses array of size 23
      /// \return calculated gas/oil ratio coefficient value
      double Gorm(double * compMasses);

      /// \brief Create PT phase diagram for the given HC composition for 11 isolines [0.0:0.1:1.0].
      /// \note The environment variable EOSPACKDIR must be defined and point to PVT_properties.cfg file folder.
      ///
      /// P/T phase diagram is calculated as set of isolines for liquid percentage volume in composition
      /// 0.0 - is dew line and 1.0 bubble line. Each isoline is defined by a set of (T,P) points.
      /// @image html PVTDiagram.png "P/T phase diagram for HC composition"
      /// The typical usage this function is following:
      /// \code{.cpp}
      /// // Define type of P/T phase diagram, 0 - mass, 1 - mole, 2 - volume
      /// int diagType = 0;
      ///
      /// // Allocate space 4 for special points (T [K], P [MPa]): critical, bubble, cricondentherm and cricondenbar points
      /// double points[8];
      ///
      /// // Allocate memory space for 11 isolines with step 0.1: [0.0:0.1:1.0], up to 400 points (T [K], P [MPa]) per isoline.
      /// // On return, it contains the real number of points per isoline calculated by the diagram calculator
      /// int szIso[11] = { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 };
      ///
      /// // Allocate memory space for isolines. For this example up to 400 (T [K], P [Pa]) points per isoline.
      /// double[] isolines = new double[11 * 400 * 2];
      ///
      /// // Pressure for composition [Pa]
      /// double compPressure = 12665600; 
      ///
      /// // Temperature for composition [K]
      /// double compTemperature = 588.15;
      ///
      /// // HC composition in trap [kg]
      /// double masses[23]  = { 4.63937e+10, 3.76229e+11, 7.53617e+10, 1.60934e+11, 7.01346e+10, 1.93474e+11, 
      ///                        6.20802e+09, 6.26855e+09, 6.3693e+09, 7.4708e+09, 7.78099e+09, 0, 2.65935e+09,
      ///                        5.4181e+08, 8.16853e+09, 2.68247e+10, 4.82603e+09, 5.65998e+07, 2.17633e+07,
      ///                        0, 0, 0, 0 };
      ///
      /// // Call PVT diagram calculator
      /// bool ret = EosPackAPI::BuildPTDiagram( diagType, compTemperature, compPressure, masses, points, szIso, isolines );
      ///
      /// \endcode
      /// \param diagType - type of diagram, 0 - mass, 1 - mole, 2 - volume
      /// \param T trap temperature [K]
      /// \param P trap pressure [MPa]
      /// \param[in] comp array of size 23 for HC composition mass fractions
      /// \param[out] points array, size of 8 which will contain on return CriticalT, CriticalP, BubbleT, BubbleP, CricondenthermT, CricondenthermP, CricondenbarT, CricondenbarP
      /// \param[in,out] szIso array with size of 11 (number of isolines). For each isoline it contains on input maximum number of isoline points allocated in isoline array.
      ///                      On output, it contains the number of calculated points for each isoline
      /// \param[out] isolines 1D array which keeps T,P values for each isoline, number of points for each isoline keeps szIso array 
      /// \return true on success, false otherwise
      bool BuildPTDiagram( int diagType, double T, double P, double * comp, double * points, int * szIso, double * isolines );
      
      /// \brief Search critical point on PT phase diagram
      /// \param diagType - type of diagram, 0 - mass, 1 - mole, 2 - volume
      /// \param[in] comp array of size 23 for composition mass fractions
      /// \param[out] critPt critical point array, size of 2 which will contain on return CriticalT, CriticalP
      /// \return true on success, false otherwise
      bool FindCriticalPoint( int diagType, double * comp, double * critPt );
   }
}

#endif // EOS_PACK_C_API_H
