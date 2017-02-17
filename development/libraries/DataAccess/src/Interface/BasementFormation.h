//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_BASEMENT_FORMATION_H_
#define _INTERFACE_BASEMENT_FORMATION_H_



#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/Formation.h"

#include "../../../CBMGenerics/src/capillarySealStrength.h"

namespace DataAccess
{
   namespace Interface
   {

      class BasementFormation : virtual public Formation
      {
      public:
         /// constructor
         BasementFormation (ProjectHandle * projectHandle, database::Record* record, const std::string& formationName, const std::string& lithologyName );
         /// destructor
         ~BasementFormation (void);

         /// Return the Surface at the top of this BasementFormation
         const string & getName (void) const;

         const GridMap * getInputThicknessMap (void) const;

         /// Return the first lithotype of this BasementFormation.
         const LithoType * getLithoType1 (void) const;

         /// Return the percentage map of the first lithotype
         const GridMap * getLithoType1PercentageMap (void) const;

         /// Return the second lithotype of this BasementFormation.
         ///
         /// The default here is to return null, since in the basement there is only a single lithotype required.
         const LithoType * getLithoType2 (void) const;

         /// Return the percentage map of the second lithotype.
         ///
         /// The default here is to return null, since in the basement there is only a single lithotype required.
         const GridMap * getLithoType2PercentageMap (void) const;

         /// Return the third lithotype of this BasementFormation.
         ///
         /// The default here is to return null, since in the basement there is only a single lithotype required.
         const LithoType * getLithoType3 (void) const;

         /// Return the percentage map of the third lithotype.
         ///
         /// The default here is to return null, since in the basement there is only a single lithotype required.
         const GridMap * getLithoType3PercentageMap (void) const;

         /// return the list of reservoirs in this formation.
         ///
         /// The default here is to return null, since in the basement there are no reservoirs.
         ReservoirList * getReservoirs (void) const;

         /// return the list of MobileLayers of this formation.
         ///
         /// The default here is to return null, since in the basement there are no mobile-layers.
         MobileLayerList * getMobileLayers (void) const;

         /// return the AllochthonousLithology for this formation.
         ///
         /// The default here is to return null, since in the basement there is not salt movement.
         const AllochthonousLithology * getAllochthonousLithology (void) const;

         /// return the list of fault collections specified for this formation.
         ///
         /// The default here is to return null, since in the basement there are no faults.
	 FaultCollectionList * getFaultCollections (void) const;

         /// tell whether this BasementFormation is a mobile layer.
         ///
         /// Returns false.
         bool isMobileLayer (void) const;

         /// tell whether this BasementFormation has an allochthonous lithology.
         ///
         /// Returns false.
         bool hasAllochthonousLithology (void) const;

         /// tell whether this BasementFormation is a source rock.
         ///
         /// Returns false.
         bool isSourceRock (void) const;

         /// Return the SourceRock of this BasementFormation.
         ///
         /// The default here is to return null, since in the basement there are no source rocks.
         const SourceRock * getSourceRock (void) const;

         /// For any formation in the basement this returns false.
         bool hasConstrainedOverpressure () const;

         /// For any formation in the basement this returns false.
         bool hasChemicalCompaction () const;

         CBMGenerics::capillarySealStrength::MixModel getMixModel (void) const;

         /// Return the mix model: always HOMOGENEOUS for basement
         const std::string& getMixModelStr (void) const;

         /// Return the layering index: -9999 for basement
         float getLayeringIndex(void) const;

         /// Return the position in the formation deposition sequence.
         ///
         /// Returns the DefaultUndefinedScalarValue.
         virtual int getDepositionSequence () const;

         /// The default here is to return null, since in the basement there are no fluids.
         const FluidType * getFluidType (void) const;

         /// \brief Get the element refinement for a basement layer, always returns 1.
         virtual unsigned int getElementRefinement () const;

         /// The default here is to return null, since in the basement there are no faults.
         GridMap * computeFaultGridMap (const Grid * localGrid, const Snapshot * snapshot) const;

         virtual PaleoFormationPropertyList * getPaleoThicknessHistory () const = 0;

         /// Load thickness map associated with this formation.
         ///
         /// Internal function, returns null since there is no map to load?
         GridMap * loadThicknessMap (void) const;

         /// Compute the thickness map in case it could not be loaded.
         ///
         /// Internal function.
         GridMap * computeThicknessMap (void) const;

      protected :

         static const std::string HomogeneousMixtureString;

         const std::string m_formationName;

         const std::string m_lithologyName;

      };
   }
}
#endif // _INTERFACE_BASEMENT_FORMATION_H_
