//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_CRUST_FORMATION_H
#define INTERFACE_CRUST_FORMATION_H

#include "DAObject.h"
#include "Interface.h"
#include "BasementFormation.h"


namespace DataAccess
{
   namespace Interface
   {

      /// A specialised formation providing the extra functionality required by the crust formation.
      class CrustFormation : virtual public BasementFormation
      {
      public:

         CrustFormation (ProjectHandle& projectHandle, database::Record* record);

         ~CrustFormation () = default;

         /// @defgroups MapAccess
         /// @{
         /// \return The closest to present day (youngest), user-supplied Crust thickness GridMap
         /// \details If the thermal model is Advanced Lithosphere Calculator or Basic Crust Thinning, then returns the youngest map
         ///          If the thermal model is Heat Flow History, then returns the fixed crustal thickness (no variation through time)
         const GridMap * getInputThicknessMap () const override;

         /// \return The closest to basin age (oldest), user-supplied Crust thickness GridMap
         /// \details This function is only used if the thermal model is Advanced Lithosphere Calculator
         const GridMap * getInitialThicknessMap() const;

         /// \return The user-supplied heat production map for the crust
         /// \details This function is only used if the thermal model is Advanced Lithosphere Calculator or Basic Crust Thinning
         const GridMap * getCrustHeatProductionMap () const;
         /// @}

         /// \return The user-supplied crustal thickness at melt onset map
         /// \details This function is only used if the thermal model is Advanced Lithosphere Calculator
         const GridMap * getCrustThicknessMeltOnsetMap() const;

         /// \return The user-supplied basalt thickness map
         /// \details This function is only used if the thermal model is Advanced Lithosphere Calculator
         const GridMap * getBasaltThicknessMap() const;

         /// \return The crust paleo-thickness history stored by the project handle
         /// \warning This function allocates a new PaleoFormationPropertyList object which needs to be deleted
         PaleoFormationPropertyList * getPaleoThicknessHistory () const override;

		 /// \return The Oceanic crust paleo-thickness history stored by the project handle
		 /// \warning This function allocates a new PaleoFormationPropertyList object which needs to be deleted
		 PaleoFormationPropertyList* getOceaPaleoThicknessHistory() const ;

         /// \return The "fake" deposition sequence of the crust which is defined to be -1
         /// \details This has no geological meaning and is used to comply with legacy code
         int getDepositionSequence () const override;

         /// \return The heat production decay constant for the crust
         /// \details The heat production has an exponential decay w.r.t. depth
         virtual double getHeatProductionDecayConstant () const;

         /// \return The crust informations as a string
         /// \brief Informations returned are name, decay constant, top surface name and bottom surface name
         /// \details This is a debug utility
         void asString (std::string & str) const override;

         void setInitialCrustalThickness( const double aInitCrustalThickness );
         double getInitialCrustalThickness() const;

      protected:

         mutable const GridMap* m_inputThicknessMap;   ///< The youngest crustal thickness map [m]
         mutable const GridMap* m_initialThicknessMap; ///< The oldest crustal thickness map   [m]

         mutable const GridMap* m_basaltThickness;           ///< The basalt thickness map (ALC only)                  [m]
         mutable const GridMap* m_crustalThicknessMeltOnset; ///< The crustl thickness at melt onset map (ALC only)    [m]
         double m_initialCrustalThickness;                   ///< The crustal thickness at the age of basin (ALC only) [m]

      private:

         /// @defgroups MapLoading
         /// @{
         /// \brief Load the Crust Heat Production user defined map from the [BasementIoTbl]
         /// \details If the Crust Heat Production is defined as a scalar, creates a constant map from it
         GridMap * loadCrustHeatProductionMap() const;
         /// @}

         /// @defgroups MapLoadingLegacyAlc
         /// @{
         /// \brief Load the Crust Thickness At Melt Onset user defined map from the [BasementIoTbl]
         /// \details If the Crust Thickness At Melt Onset is defined as a scalar, creates a constant map from it
         ///    Only use by prograde application to upgrade leagcy ALC projects
         GridMap * loadCrustThicknessMeltOnsetMap() const;

         /// \brief Load the Basalt Thickness user defined map from the [BasementIoTbl]
         /// \details If the Basalt Thickness is defined as a scalar, creates a constant map from it
         ///    Only use by prograde application to upgrade leagcy ALC projects
         GridMap * loadBasaltThicknessMap() const;
         /// @}

      };

      inline void CrustFormation::setInitialCrustalThickness (const double aInitCrustalThickness ) {
         m_initialCrustalThickness = aInitCrustalThickness;
      }

      inline double CrustFormation::getInitialCrustalThickness () const {
         return m_initialCrustalThickness;
      }
   }
}
#endif // INTERFACE_CRUST_FORMATION_H
