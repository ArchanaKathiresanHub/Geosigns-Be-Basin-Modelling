//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PermeabilityCalculator.h"

#include "ProjectHandle.h"
#include "RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithology.h"
#include "CompoundProperty.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::M2ToMilliDarcy;


DataAccess::Mining::PermeabilityCalculator::PermeabilityCalculator ( const DomainPropertyCollection*            collection,
                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                     const Interface::Snapshot*                 snapshot,
                                                                     const Interface::Property*                 property,
                                                                     const bool                                 normalPermeability ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property ),
   m_normalPermeability ( normalPermeability )
{
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_chemicalCompactionRequired = getPropertyCollection ()->getProjectHandle ()->getRunParameters ()->getChemicalCompaction ();
   m_initialised = false;
}

bool DataAccess::Mining::PermeabilityCalculator::initialise () {

   if ( not m_initialised ) {
      m_ves = getPropertyCollection ()->getDomainProperty ( "Ves", getPropertyManager ());
      m_maxVes = getPropertyCollection ()->getDomainProperty ( "MaxVes", getPropertyManager ());

      if ( m_chemicalCompactionRequired ) {
         m_chemicalCompaction = getPropertyCollection ()->getDomainProperty ( "ChemicalCompaction", getPropertyManager ());
      } else {
         m_chemicalCompaction = 0;
      }

      if ( m_ves != 0 and m_maxVes != 0 and (( m_chemicalCompactionRequired and m_chemicalCompaction != 0 ) or not m_chemicalCompactionRequired )) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::PermeabilityCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or not position.isValidElement ()) {
      return Interface::DefaultUndefinedMapValue;
   }

   GeoPhysics::CompoundProperty porosity;
   double ves;
   double maxVes;
   double chemicalCompaction;
   double permeabilityN;
   double permeabilityP;

   const Interface::Formation* formation = position.getFormation ();
   const GeoPhysics::GeoPhysicsFormation* geoForm = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
   bool layerRequiresChemicalCompaction = m_chemicalCompactionRequired and ( geoForm != 0 and geoForm->hasChemicalCompaction ());

   ves = m_ves->compute ( position );
   maxVes = m_maxVes->compute ( position );

   if ( layerRequiresChemicalCompaction ) {
      chemicalCompaction = m_chemicalCompaction->compute ( position );
   } else {
      chemicalCompaction = Interface::DefaultUndefinedMapValue;
   }

   if ( ves != Interface::DefaultUndefinedMapValue and maxVes != Interface::DefaultUndefinedMapValue and 
        (( layerRequiresChemicalCompaction and chemicalCompaction != Interface::DefaultUndefinedMapValue ) or not layerRequiresChemicalCompaction )) {

      const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

      lithology->getPorosity ( ves, maxVes, layerRequiresChemicalCompaction, chemicalCompaction, porosity );
      lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permeabilityN, permeabilityP );

      permeabilityN *= M2ToMilliDarcy;
      permeabilityP *= M2ToMilliDarcy;

#if 0
      permeabilityN = std::log10 ( permeabilityN );
      permeabilityH = std::log10 ( permeabilityH );
#endif

   } else {
      permeabilityN = Interface::DefaultUndefinedMapValue;
      permeabilityP = Interface::DefaultUndefinedMapValue;
   }

   if ( m_normalPermeability ) {
      return permeabilityN;
   } else {
      return permeabilityP;
   }

}

DataAccess::Mining::PermeabilityCalculatorAllocator::PermeabilityCalculatorAllocator ( const bool normalPermeability ) :
   m_normalPermeability ( normalPermeability )
{
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::PermeabilityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                    const Interface::Snapshot*                 snapshot,
                                                                                                    const Interface::Property*                 property ) const {
   return new PermeabilityCalculator ( collection, propertyManager, snapshot, property, m_normalPermeability );
}
