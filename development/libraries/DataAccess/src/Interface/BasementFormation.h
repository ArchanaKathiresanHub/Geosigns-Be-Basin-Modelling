//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_BASEMENT_FORMATION_H
#define INTERFACE_BASEMENT_FORMATION_H



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

         /// \brief The "fake" deposition sequence of the Crust and the Mantle
         /// \details This has no geological meaning since these "formations" are the base of the sediment deposition
         enum DepositionSequence {
            CRUST_DEPOSITION  = -1,
            MANTLE_DEPOSITION = -2
         };

         BasementFormation (ProjectHandle * projectHandle, database::Record* record, const std::string& formationName, const std::string& lithologyName );
         ~BasementFormation (void);

         /// Return the Surface at the top of this BasementFormation
         const string & getName (void) const;

         const GridMap * getInputThicknessMap (void) const;

         /// Return the first lithotype of this BasementFormation.
         const LithoType * getLithoType1 (void) const;

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

         /// The default here is to return null, since in the basement there are no faults.
         GridMap * computeFaultGridMap (const Grid * localGrid, const Snapshot * snapshot) const;

         virtual PaleoFormationPropertyList * getPaleoThicknessHistory () const = 0;

      protected : 

         static const std::string HomogeneousMixtureString;

         const std::string m_formationName;

         const std::string m_lithologyName;

      };
   }
}
#endif // INTERFACE_BASEMENT_FORMATION_H
