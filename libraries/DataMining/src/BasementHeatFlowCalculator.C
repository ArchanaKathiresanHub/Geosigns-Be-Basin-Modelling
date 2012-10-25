#include "BasementHeatFlowCalculator.h"

#include <iostream>
using namespace std;

#include "Interface/RunParameters.h"
#include "Interface/Surface.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsProjectHandle.h"

#include "CompoundLithology.h"
#include "ElementFunctions.h"

DataAccess::Mining::BasementHeatFlowCalculator::BasementHeatFlowCalculator ( const DomainPropertyCollection*  collection,
                                                                             const Interface::Snapshot* snapshot,
                                                                             const Interface::Property* property ) :
   DomainFormationProperty ( collection, snapshot, property )
{

   m_depth = 0;
   m_temperature = 0;

   m_crust = 0;
   m_bottomSediment = 0;

   m_ves = 0;
   m_maxVes = 0;

   m_fromCrust = true;
   m_initialised = false;
}

bool DataAccess::Mining::BasementHeatFlowCalculator::initialise () {

   if ( not m_initialised ) {
      Interface::PropertyValueList* propVals;
      Interface::PropertyValueList::const_iterator propValIter;


      m_crust = dynamic_cast<const GeoPhysics::Formation*>(getProjectHandle ()->findFormation ( "Crust" ));

      propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "Temperature" ), getSnapshot (), 0, m_crust, 0, Interface::VOLUME );

      if ( propVals->size () != 1 ) {
         // // What to do
         // delete propVals;
         // m_initialised = false;
         // return false;
      } else {
         m_temperature = (*propVals)[ 0 ];
      }

      delete propVals;

      propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "Depth" ), getSnapshot (), 0, m_crust, 0, Interface::VOLUME );

      if ( propVals->size () != 1 ) {
         // // What to do
         // delete propVals;
         // m_initialised = false;
         // return false;
      } else {
         m_depth = (*propVals)[ 0 ];
      }

      delete propVals;


      if ( m_crust != 0 ) {
         m_bottomSediment = dynamic_cast<const GeoPhysics::Formation*>(m_crust->getTopSurface ()->getTopFormation ());
      } else {
         // How to get this formation?
         m_bottomSediment = 0;
      }

      if ( m_bottomSediment != 0 ) {

         propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "Temperature" ), getSnapshot (), 0, m_bottomSediment, 0, Interface::VOLUME );

         if ( propVals->size () != 1 ) {
            // // What to do
            // delete propVals;
            // m_initialised = false;
            // return false;
         } else {
            m_temperature = (*propVals)[ 0 ];
            m_fromCrust = false;
         }

         delete propVals;

         propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "Depth" ), getSnapshot (), 0, m_bottomSediment, 0, Interface::VOLUME );

         if ( propVals->size () != 1 ) {
            // // What to do
            // delete propVals;
            // m_initialised = false;
            // return false;
            m_fromCrust = true;
         } else {
            m_depth = (*propVals)[ 0 ];
            m_fromCrust = false;
         }

         delete propVals;

         propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "Ves" ), getSnapshot (), 0, m_bottomSediment, 0, Interface::VOLUME );

         if ( propVals->size () != 1 ) {
            // // What to do
            // delete propVals;
            // m_initialised = false;
            // return false;
            m_fromCrust = true;
         } else {
            m_ves = (*propVals)[ 0 ];
            m_fromCrust = false;
         }

         delete propVals;

         propVals = getProjectHandle ()->getPropertyValues ( Interface::FORMATION, getProjectHandle ()->findProperty ( "MaxVes" ), getSnapshot (), 0, m_bottomSediment, 0, Interface::VOLUME );

         if ( propVals->size () != 1 ) {
            // // What to do
            // delete propVals;
            // m_initialised = false;
            // return false;
            m_fromCrust = true;
         } else {
            m_maxVes = (*propVals)[ 0 ];
            m_fromCrust = false;
         }

         delete propVals;

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
   double porosity;
   double thermalConductivityN;
   double thermalConductivityH;
   unsigned int mapKSize;

   FiniteElementMethod::Matrix3x3     conductivityTensor;
   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ThreeVector   gradTemperature;

   FiniteElementMethod::ElementVector temperatureCoeffs;
   FiniteElementMethod::ElementVector depthCoeffs;
   FiniteElementMethod::ElementVector vesCoeffs;
   FiniteElementMethod::ElementVector maxVesCoeffs;

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   // const GeoPhysics::Formation* geoForm = dynamic_cast<const GeoPhysics::Formation*>( m_crust );
   const GeoPhysics::CompoundLithology* lithology = m_crust->getCompoundLithology ( position.getI (), position.getJ ());

   if ( m_fromCrust ) {
      mapKSize = m_temperature->getGridMap ()->lastK () - 1;
   } else {
      mapKSize = 0;
   }

   getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_depth->getGridMap (), depthCoeffs );
   getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_temperature->getGridMap (), temperatureCoeffs );

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

      getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_ves->getGridMap (), vesCoeffs );
      getElementCoefficients ( position.getI (), position.getJ (), mapKSize, m_maxVes->getGridMap (), maxVesCoeffs );

      fluid = dynamic_cast<const GeoPhysics::FluidType*>(m_bottomSediment->getFluidType ());

      porosity = lithology->porosity ( finiteElement.interpolate ( vesCoeffs ),
                                       finiteElement.interpolate ( maxVesCoeffs ),
                                       false, 0.0 );

   }

   temperature = finiteElement.interpolate ( temperatureCoeffs );
   lithology->calcBulkThermCondNP ( fluid, porosity, temperature, thermalConductivityN, thermalConductivityH );

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

DataAccess::Mining::DomainProperty* DataAccess::Mining::BasementHeatFlowCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                        const Interface::Snapshot* snapshot,
                                                                                                        const Interface::Property* property ) const {
   return new BasementHeatFlowCalculator ( collection, snapshot, property );
}


