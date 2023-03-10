//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ConstantsGenex.h
/// @brief This file stores the constants used only by the genex applications and libraries

#ifndef CONSTANTS_GENEX_H
#define CONSTANTS_GENEX_H

#include <string>
namespace Genex6
{

   /// @ Defines genex specific constants
   namespace Constants
   {
      /// @defgroup Simulations_type
      ///    Defines which type of simulation genex6 is performing
      /// @{
      constexpr int SIMOTGC   = 0x0001; ///< Oil to gas cracking
      constexpr int SIMGENEX  = 0x0002; ///< Hydorcarbones generation
      constexpr int SIMGENEX5 = 0x0004; ///< Hydorcarbones generation
      constexpr int SIMOTGC5  = 0x0008; ///< Oil to gas cracking
      constexpr int SIMTSR    = 0x0010; ///< Termo-sulfate reduction
      /// @}

      /// @defgroup OrganicMatter_type
      ///    Defines the type of organic matter
      /// @{
      enum
      {
         TOTAL_OM,                      ///< The total organic matter
         AROMATIC_IMMOBILE_OM,          ///< The armoatic and immobile organic matter
         MOBILE_OM,                     ///< The mobile organic matter
         NUMBER_OF_ORGANIC_MATTER_TYPES ///< End of enumeration
      };
      /// @}

      /// @defgroup Numerical
      ///    Defines some numerical constants for the simulations
      /// @{
      /// @brief Fraction of time-step that is used to determine closeness to a snapshot.
      /// If the next time-step is close to a snapshot then take the snapshot time 
      /// rather than perform another time-step with a possibly very small (O(1.0e-13)) time-step size.
      const double TimeStepFraction = 0.001;
      ///@}

      /// @defgroup Diffusion_state_theory
      /// @{
      constexpr double VanKrevelenHCCorrector  = 0.5;             ///< Van Krevelen correction factor
      constexpr double s_ActUMax               = 1000000000000.0; ///< The maximum activation energy
      constexpr double CminHC                  = 0.9999999;       ///< The minimum number of carbon atoms in an hydrocarbone   (1)
      constexpr double HminHC                  = 0.0;             ///< The minimum number of hydrogen atoms in an hydrocarbone (0)
      constexpr double MaxGasDensity           = 10.0;            ///< The maximum gas density [kg.m-3]
      /// @}

      /// @defgroup formulaOfAsphaltene
      ///    Formula of asphaltene as f(preasphalt), (e.g. Durand-Souron '82)
      /// @{
      constexpr double GORUpperbound = 100000.0; ///< The maximum GOR

      //FunApi(DensOil)=APIC1/DensOil*APIC2-APIC3;
      constexpr double APIC1=141.5;
      constexpr double APIC2=1000.0; 
      constexpr double APIC3=131.5; 

      //
      constexpr double Zero=0.000001;                        ///< Zero in genex
      constexpr double FluxVolumeOilZero=1E-39;              ///< Zero oil flux in genex
      constexpr double CumulativeVolumeOilZero=9.999946E-41; ///< Zero oil volume in genex

      //formula volume, Van Krevelen, eqn. XVI,6, p.317
      //FormVol = FormVol1 + FormVol2 * Atom(IatomH, L) + FormVol3 * Atom(IatomO, L) - VolRing * Rc
      constexpr double FormVol1  = 9.9;
      constexpr double FormVol2  = 3.1;
      constexpr double FormVol3  = 3.75;

      //volume of rings, Van Krevelen and Chermin '54, VK eqn. XVI,4, p.317
      //VolRing = VolRing1 + VolRing2 * Atom(IatomH, L)
      constexpr double VolRing1  = 9.1;
      constexpr double VolRing2  = -3.65;

      // Vitrinite reflectance values, from genex4
      //    not used in genex 6 or genex 7 anymore since VRE_ini is set to 0.5
      constexpr double VRE1=0.4;
      constexpr double VRE2=0.5;
      constexpr double VRE3=0.6;
      constexpr double VRE4=0.7;
      /// @}

      /// \brief The volume of a mole of methane at surface conditions [mol.m^-3]
      constexpr double VolumeMoleMethaneAtSurfaceConditions = 42.306553;
      
      /// \brief Folder structure
#ifdef WIN32
      const std::string FolderDivider = "\\";
#else
      const std::string FolderDivider = "/";
#endif

   }

   /// @brief Defines genex Io table strings
   namespace CFG
   {
      const std::string EndOfTable                   = "[EndOfTable]";
      const std::string TableSourceRockProperties    = "Table:[SourceRockProperties]";

      //Table:SimulatorProperties
      const std::string TableSimulatorProperties     = "Table:[SimulatorProperties]";
      const std::string PreprocessSpeciesKinetics    = "PreprocessSpeciesKinetics";
      const std::string PreprocessSpeciesComposition = "PreprocessSpeciesComposition";
      const std::string UseDefaultGeneralParameters  = "UseDefaultGeneralParameters";
      const std::string NumberOfTimesteps            = "NumberOfTimesteps";
      const std::string SimulateOpenConditions       = "SimulateOpenConditions";
      const std::string MaximumTimeStepSize          = "MaximumTimeStepSize";
      const std::string MassBalancePercentTolerance  = "MassBalancePercentTolerance";

      //Table:Elements
      const std::string TableElements                = "Table:[Elements]";
      const std::string TableSpecies                 = "Table:[Species]";
      const std::string SpeciesCompositionByName     = "Table:[SpeciesCompositionByName]";
      const std::string SpeciesPropertiesByName      = "Table:[SpeciesPropertiesByName]";
      const std::string ReactionsBySpeciesName       = "Table:[ReactionsBySpeciesName]";
      const std::string ReactionRatiosBySpeciesName  = "Table:[ReactionRatiosBySpeciesName]";
      const std::string GeneralParameters            = "Table:[GeneralParameters]";
   }

}
#endif // CONSTANTS_H
