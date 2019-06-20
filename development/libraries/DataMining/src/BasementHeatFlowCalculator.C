//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BasementHeatFlowCalculator.h"

#include <iostream>
using namespace std;

#include "RunParameters.h"
#include "Surface.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsProjectHandle.h"

#include "CompoundLithology.h"
#include "ElementFunctions.h"

using namespace AbstractDerivedProperties;

DataAccess::Mining::BasementHeatFlowCalculator::BasementHeatFlowCalculator ( const DomainPropertyCollection*            collection,
                                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                             const Interface::Snapshot*                 snapshot,
                                                                             const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property ),
   m_fromCrust ( true ),
   m_initialised ( false )
{
}

bool DataAccess::Mining::BasementHeatFlowCalculator::initialise () {

   if ( not m_initialised ) {
      FormationPropertyPtr propVals;

      m_crust = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(getProjectHandle ()->findFormation ( "Crust" ));
      m_temperature = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Temperature" ),
                                                                   getSnapshot (),
                                                                   m_crust );

      if ( m_temperature == 0 ) {
         // What to do
         // return false;
      }

      m_depth = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Depth" ),
                                                             getSnapshot (),
                                                             m_crust );

      if ( m_depth == 0 ) {
         // What to do
         // return false;
      }


      if ( m_crust != 0 ) {
         m_bottomSediment = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(m_crust->getTopSurface ()->getTopFormation ());
      } else {
         // How to get this formation?
         m_bottomSediment = 0;
      }

      if ( m_bottomSediment != 0 ) {

         propVals = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Temperature" ),
                                                                 getSnapshot (),
                                                                 m_bottomSediment );

         if ( propVals == 0 ) {
            // What to do
            // return false;
            m_fromCrust = true;
         } else {
            m_temperature = propVals;
            m_fromCrust = false;
         }

         m_depth = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Depth" ),
                                                                getSnapshot (),
                                                                m_bottomSediment );

         if ( propVals == 0 ) {
            // What to do
            // return false;
            m_fromCrust = true;
         } else {
            m_depth = propVals;
            m_fromCrust = false;
         }

         m_ves = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Ves" ),
                                                              getSnapshot (),
                                                              m_bottomSediment );

         m_fromCrust = m_ves == 0;

         if ( m_ves == 0 ) {
            // What to do
            // return false;
         }

         m_maxVes = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "MaxVes" ),
                                                                 getSnapshot (),
                                                                 m_bottomSediment );

         m_fromCrust = m_maxVes == 0;

         if ( m_maxVes == 0 ) {
            // What to do
            // return false;
         }

         m_pressure = getPropertyManager ().getFormationProperty ( getProjectHandle ()->findProperty ( "Pressure" ),
                                                                   getSnapshot (),
                                                                   m_bottomSediment );

         m_fromCrust = m_pressure == 0;

         if ( m_pressure == 0 ) {
            // What to do
            // return false;
         }

      }

      if ( m_crust != 0 ) {
      // if ( m_crust != 0 or m_bottomSediment != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   // m_fromCrust = false;
   return m_initialised;
}

double DataAccess::Mining::BasementHeatFlowCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or position.getFormation () == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const GeoPhysics::FluidType* fluid;
   double basementHeatFlow;
   double temperature;
   double pressure;
   double porosity;
   double thermalConductivityN;
   double thermalConductivityH;
   unsigned int mapKSize;

   FiniteElementMethod::Matrix3x3     conductivityTensor;
   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ThreeVector   gradTemperature;

   FiniteElementMethod::ElementVector temperatureCoeffs;
   FiniteElementMethod::ElementVector pressureCoeffs;
   FiniteElementMethod::ElementVector depthCoeffs;
   FiniteElementMethod::ElementVector vesCoeffs;
   FiniteElementMethod::ElementVector maxVesCoeffs;

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   // const GeoPhysics::GeoPhysicsFormation* geoForm = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( m_crust );
   const GeoPhysics::CompoundLithology* lithology = m_crust->getCompoundLithology ( position.getI (), position.getJ ());

   if ( m_fromCrust ) {
      mapKSize = m_temperature->lastK () - 1;
   } else {
      mapKSize = 0;
   }

   getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_depth, depthCoeffs );
   getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_temperature, temperatureCoeffs );
   getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_pressure, pressureCoeffs );

   getGeometryMatrix ( position,
                       dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ())->getCauldronGridDescription (),
                       depthCoeffs, geometryMatrix );

   finiteElement.setGeometry ( geometryMatrix );

   if ( m_fromCrust ) {
      // Want only the heat-flow across the top of crust.
      finiteElement.setQuadraturePoint ( position.getReferencePoint ()( 0 ), position.getReferencePoint ()( 1 ), -1.0 );
      fluid = 0;
      porosity = 1.0;
   } else {
      finiteElement.setQuadraturePoint ( position.getReferencePoint ()( 0 ), position.getReferencePoint ()( 1 ), 1.0 );

      getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_ves, vesCoeffs );
      getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_maxVes, maxVesCoeffs );

      fluid = dynamic_cast<const GeoPhysics::FluidType*>(m_bottomSediment->getFluidType ());

      porosity = lithology->porosity ( finiteElement.interpolate ( vesCoeffs ),
                                       finiteElement.interpolate ( maxVesCoeffs ),
                                       false, 0.0 );

   }

   temperature = finiteElement.interpolate ( temperatureCoeffs );
   pressure    = finiteElement.interpolate ( pressureCoeffs );
   lithology->calcBulkThermCondNP ( fluid, porosity, temperature, pressure, thermalConductivityN, thermalConductivityH );

   finiteElement.setTensor ( thermalConductivityN, thermalConductivityH, conductivityTensor );

   gradTemperature = finiteElement.interpolateGrad ( temperatureCoeffs );

   basementHeatFlow = conductivityTensor ( 3, 1 ) * gradTemperature ( 1 ) + conductivityTensor ( 3, 2 ) * gradTemperature ( 2 ) + conductivityTensor ( 3, 3 ) * gradTemperature ( 3 );

   // Fouriers law states q = -k \grad T.
   // basementHeatFlow *= -1.0;
   // But Cauldron uses a +ve depth, so we need a -q(3)

   // Convert to mW/m^2.
   basementHeatFlow *= 1000.0;


   return basementHeatFlow;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::BasementHeatFlowCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                        DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                        const Interface::Snapshot*                 snapshot,
                                                                                                        const Interface::Property*                 property ) const {
   return new BasementHeatFlowCalculator ( collection, propertyManager, snapshot, property );
}


